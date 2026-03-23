#pragma once
#include <string>
#include <vector>
#include "RenderingData.h"
#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "SkyBoxCommon.h"

class SkyBox
{
public:
	SkyBox() = default;
	~SkyBox() ;

	void Initialize(std::string textureFilePath);
	void Update();
	void Draw();
	
	void imguidebug();
	
	std::string GetTextureFilePath() const { return textureFilePath_; }

private:
	SrvManager* srvManager_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//データを書き込む
	TransformationMatrix* transformaitionMatrixData = nullptr;

	//indexバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	uint16_t* indexData = nullptr;
	std::vector<uint16_t> indices;
	


	std::vector<VertexData>vertices;
	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	//VBV
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//マテリアルにデータを書き込む	
	Material* materialData = nullptr;
	std::string textureFilePath_;
	int textureIndex_ = 0;

	//SRT
	EulerTransform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

};

