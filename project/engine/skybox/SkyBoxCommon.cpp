#include "SkyBoxCommon.h"
#include "CameraManager.h"
#include "TextureManager.h"

void SkyBoxCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvmanager, CameraManager* cameraManager, TextureManager* textureManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvmanager;
	cameraManager_ = cameraManager;
	textureManager_ = textureManager;

	graphicsPipeline_ = new GraphicsPipeline();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateSkybox();

}

void SkyBoxCommon::Finalize()
{

	delete graphicsPipeline_;
	graphicsPipeline_ = nullptr;
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
	cameraManager_ = nullptr;
	textureManager_ = nullptr;

}

void SkyBoxCommon::commonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureSkybox());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateSkybox());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
