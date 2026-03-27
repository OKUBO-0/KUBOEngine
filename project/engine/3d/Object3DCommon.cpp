#include "Object3DCommon.h"
#include "Logger.h"
#include "CameraManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

void Object3DCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvmanage, CameraManager* cameraManager, ModelManager* modelManager, TextureManager* textureManager)
{

	dxCommon_ = dxCommon;
	srvManager_ = srvmanage;
	cameraManager_ = cameraManager;
	modelManager_ = modelManager;
	textureManager_ = textureManager;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->Create();
	
	skinningGraphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	skinningGraphicsPipeline_->Initialize(dxCommon_);
	skinningGraphicsPipeline_->CreateSkinning();

	

}

void Object3DCommon::Finalize()
{
	graphicsPipeline_.reset();
	skinningGraphicsPipeline_.reset();
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
	cameraManager_ = nullptr;
	modelManager_ = nullptr;
	textureManager_ = nullptr;
}

void Object3DCommon::CommonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignature());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void Object3DCommon::SkinNingCommonDraw()
{
	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(skinningGraphicsPipeline_->GetRootSignatureSkinning());
	dxCommon_->GetCommandList()->SetPipelineState(skinningGraphicsPipeline_->GetGraphicsPipelineStateSkinning());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


}


