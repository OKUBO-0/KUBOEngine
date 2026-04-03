#include "TextureManager.h"
#include "StringUtility.h"
#include "Logger.h"



using namespace StringUtility;
using namespace Logger;

namespace {
DirectX::TexMetadata gEmptyMetadata{};
}

void TextureManager::Finalize()
{
	textureDatas.clear();
	dxCommon_ = nullptr;
	srvmanager = nullptr;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvmanager)
{
	dxCommon_ = dxCommon;
	this->srvmanager = srvmanager;
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);

}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filepath)
{
	if (!textureDatas.contains(filepath)) {
		Log("TextureManager::GetMetaData failed. Texture not loaded: " + filepath + "\n");
		return gEmptyMetadata;
	}
	return textureDatas.at(filepath).metadata;
}

//Imgui で０番を使用するため１番から使用
uint32_t TextureManager::kSRVIndexTop = 1;
void TextureManager::LoadTexture(const std::string& filePath)
{
	if (textureDatas.contains(filePath)) {
		return;//酔いこみ済みなら早期return
	}
	if (!dxCommon_ || !srvmanager) {
		Log("TextureManager::LoadTexture failed. Manager is not initialized.\n");
		return;
	}
	if (!srvmanager->CheckTexturesNumber()) {
		Log("TextureManager::LoadTexture failed. No available SRV slot: " + filePath + "\n");
		return;
	}


	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr;
	if (filePathW.ends_with(L".dds")) {
		//DDSファイルの場合はLoadFromDDSFileを使用
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	if (FAILED(hr)) {
		Log("TextureManager::LoadTexture failed to read file: " + filePath + "\n");
		return;
	}


	//ミニマップの作成
	DirectX::ScratchImage mipImages{};
	if(DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		//非圧縮テクスチャの場合はGenerateMipMapsを使用
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
	}
	if (FAILED(hr)) {
		Log("TextureManager::LoadTexture failed to generate mipmaps: " + filePath + "\n");
		return;
	}

	////テクスチャデータを追加
	//textureDatas.resize(textureDatas.size() + 1);
	//追加したデータの参照を取得する
	TexturData& textureData = textureDatas[filePath];

	//textureData.filePath= ConvertString(filePathW);
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

	Microsoft::WRL::ComPtr<ID3D12Resource>  intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
	dxCommon_->CommandKick();

	////テクスチャデータの要素番号をSRVのインデックスとする
	//uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size()-1)+kSRVIndexTop;

	textureData.srvIndex = srvmanager->Allocate();
	textureData.srvHandleCPU = srvmanager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvmanager->GetGPUDescriptorHandle(textureData.srvIndex);

	srvmanager->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, (UINT)textureData.metadata.mipLevels, textureDatas[filePath].metadata);

}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filepath)
{
	if (textureDatas.contains(filepath)) {
		return textureDatas[filepath].srvIndex;
	}
	Log("TextureManager::GetTextureIndexByFilePath failed. Texture not loaded: " + filepath + "\n");
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filepath)
{
	if (!textureDatas.contains(filepath)) {
		Log("TextureManager::GetSrvHandleGPU failed. Texture not loaded: " + filepath + "\n");
		return {};
	}
	return textureDatas.at(filepath).srvHandleGPU;
}



