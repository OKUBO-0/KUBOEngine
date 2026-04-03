#pragma once
#include"DirectXCommon.h"
#include"SrvManager.h"
#include "GraphicsPipeline.h"

// OffscreenRenderManager.h の上部 or 外部ファイルで定義
//enum class PostEffectType {
//
//	Fullscreen,
//	Grayscale,
//	Vignette,
//	BoxFilter,
//	LuminanceOutline
//};

enum class PostEffectType {
	Fullscreen,
	Grayscale,
	Vignette,
	BoxFilter,
	LuminanceOutline,
	RadialBlur,

};

class OffscreenRenderManager
{
public:
	//初期化
	void Initialize(DirectXCommon* dxcommon, SrvManager*srvmanager);
	//描画前処理
	void Begin();
	//描画後処理
	void End();
	
	void Draw();
	
	//RenderTargetTextureの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& ClearColor);

	void SetPostEffectType(PostEffectType type) {
		currentEffectType_ = type;
	}

	void DrawImGui();
private:
	struct PostEffectParameters {
		float radialBlurCenterX = 0.5f;
		float radialBlurCenterY = 0.5f;
		float radialBlurWidth = 0.01f;
		float radialBlurSamples = 10.0f;
		float vignettePower = 0.8f;
		float vignetteMultiplier = 16.0f;
		float boxFilterStepScale = 1.0f;
		float boxFilterBlend = 1.0f;
		float outlineStrength = 6.0f;
		float outlineThreshold = 0.0f;
	};

	//DirectXCommonのポインタ
	DirectXCommon* dxCommon_ = nullptr;
	//SRVManagerのポインタ
	SrvManager* srvManager_ = nullptr;
	//レンダーテクスチャ
	DirectX::TexMetadata renderTargetMetadata_;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTextureResource;//レンダーテクスチャ
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetTextureHandle;//レンダーテクスチャのハンドル
	const Vector4 clearColor = { 0.1f,0.25f,0.5f,1.0f };//とりあえず赤
	uint32_t srvIndex = 0;

	PostEffectType currentEffectType_ = PostEffectType::Fullscreen; // ←追加
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;
	Microsoft::WRL::ComPtr<ID3D12Resource> postEffectParameterResource_;
	PostEffectParameters* postEffectParameters_ = nullptr;

	D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;


};

