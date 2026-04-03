#include "OffscreenRenderManager.h"
#include <imgui.h>
#include "Logger.h"

void OffscreenRenderManager::Initialize(DirectXCommon* dxcommon, SrvManager* srvmanager)
{
	dxCommon_ = dxcommon;
	srvManager_ = srvmanager;

	//RTVの作成

	renderTargetTextureResource = CreateRenderTargetTextureResource(
		WinApp::kClientWindth,
		WinApp::kClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		clearColor
	);
	renderTargetMetadata_.width = WinApp::kClientWindth;
	renderTargetMetadata_.height = WinApp::kClientHeight;
	renderTargetMetadata_.depth = 1;
	renderTargetMetadata_.arraySize = 1;
	renderTargetMetadata_.mipLevels = 1;
	renderTargetMetadata_.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTargetMetadata_.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
	renderTargetTextureHandle = dxCommon_->GetRTVCPUDescriputorHandole(2);

	dxCommon_->GetDevice()->CreateRenderTargetView(renderTargetTextureResource.Get(),
		&dxCommon_->GetRTVDesc(), renderTargetTextureHandle);


	//SRVの作成
	srvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforTexture2D(srvIndex, renderTargetTextureResource.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, renderTargetMetadata_);

	//graphicsPipelineの初期化
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);

	graphicsPipeline_->RootSignatureCopyImageCreate();
	graphicsPipeline_->CreateAllPostEffects(); // ←これだけ！

	postEffectParameterResource_ = dxCommon_->CreateBufferResource(sizeof(PostEffectParameters));
	postEffectParameterResource_->Map(0, nullptr, reinterpret_cast<void**>(&postEffectParameters_));
}

void OffscreenRenderManager::Begin()
{
	if (currentState_ != D3D12_RESOURCE_STATE_RENDER_TARGET) {

		// 前回使ったSRV状態から描画に戻す
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTargetTextureResource.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);

		currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	//描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	dxCommon_->GetCommandList()->OMSetRenderTargets(1, &renderTargetTextureHandle, false, &dsvHandle);
	//指定した色で画面全体をクリアする
	float kclearColor[] = { clearColor.x,clearColor.y,clearColor.z,clearColor.w };//青っぽい色。RGBAの順
	dxCommon_->GetCommandList()->ClearRenderTargetView(renderTargetTextureHandle, kclearColor, 0, nullptr);
	dxCommon_->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);



	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCliseすること

	D3D12_VIEWPORT viewport = dxCommon_->GetViewport();
	D3D12_RECT scissorRect = dxCommon_->GetScissorRect();

	dxCommon_->GetCommandList()->RSSetViewports(1, &viewport);
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &scissorRect);


}

void OffscreenRenderManager::End()
{
	if (currentState_ != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = renderTargetTextureResource.Get();
		barrier.Transition.StateBefore = currentState_;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);

		currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}


}

void OffscreenRenderManager::Draw()
{
	ID3D12PipelineState* pipelineState = graphicsPipeline_->GetGraphicsPipelineStateCopyImage(currentEffectType_);
	if (pipelineState == nullptr) {
		Logger::Log("OffscreenRenderManager::Draw skipped. Post effect pipeline was not created.\n");
		return;
	}

	dxCommon_->GetCommandList()->SetPipelineState(pipelineState);
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureCopyImage());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//heapの設定
	srvManager_->SetGraficsRootDescriptorTable(0, srvIndex);
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, postEffectParameterResource_->GetGPUVirtualAddress());

	//描画	
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);




}



Microsoft::WRL::ComPtr<ID3D12Resource> OffscreenRenderManager::CreateRenderTargetTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& ClearColor)
{
	D3D12_RESOURCE_DESC resouceDesc{ };
	resouceDesc.Width = width;//Textureの幅
	resouceDesc.Height = height;//Textureの高さ
	resouceDesc.MipLevels = 1;//mipmapの数
	resouceDesc.DepthOrArraySize = 1;//奥行きor配Texturaの配列数
	resouceDesc.Format = format;//Textureのフォーマット
	resouceDesc.SampleDesc.Count = 1;//サンプリクト。１固定。
	resouceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;//RenderTargetとして使う
	resouceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//Textureの次元数。普段使っているのは２次元

	//利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//細かい設定を行う


	// クリア値の設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	// リソース作成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊設定。特になし
		&resouceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_RENDER_TARGET,//RenderTargetとして使う状態にしておく
		&clearValue,//Clear最適値
		IID_PPV_ARGS(&resource)//作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return resource;
}

void OffscreenRenderManager::DrawImGui()
{
	ImGui::Begin("OffscreenRenderManager");
	const char* items[] = {
	   "Fullscreen",
	   "Grayscale",
	   "Vignette",
	   "BoxFilter",
	   "LuminanceOutline",
	   "RadialBlur"
	};

	// 現在の enum を int に変換
	int current = static_cast<int>(currentEffectType_);

	if (ImGui::Combo("Post Effect", &current, items, IM_ARRAYSIZE(items))) {
		// 変更があった場合は enum にキャストして設定
		SetPostEffectType(static_cast<PostEffectType>(current));
	}

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Radial Blur", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat2("Center", &postEffectParameters_->radialBlurCenterX, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("Width", &postEffectParameters_->radialBlurWidth, 0.0005f, 0.0f, 0.1f);
		ImGui::DragFloat("Samples", &postEffectParameters_->radialBlurSamples, 0.1f, 1.0f, 32.0f);
	}

	if (ImGui::CollapsingHeader("Vignette", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat("Power", &postEffectParameters_->vignettePower, 0.01f, 0.1f, 4.0f);
		ImGui::DragFloat("Multiplier", &postEffectParameters_->vignetteMultiplier, 0.1f, 0.0f, 32.0f);
	}

	if (ImGui::CollapsingHeader("Box Filter", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat("Step Scale", &postEffectParameters_->boxFilterStepScale, 0.01f, 0.1f, 4.0f);
		ImGui::DragFloat("Blend", &postEffectParameters_->boxFilterBlend, 0.01f, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Outline", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat("Strength", &postEffectParameters_->outlineStrength, 0.1f, 0.0f, 20.0f);
		ImGui::DragFloat("Threshold", &postEffectParameters_->outlineThreshold, 0.001f, 0.0f, 1.0f);
	}
	ImGui::End();
}
