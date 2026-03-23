#include "SkyBoxCommon.h"


SkyBoxCommon* SkyBoxCommon::instance_ = nullptr;
SkyBoxCommon* SkyBoxCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new SkyBoxCommon();
	}
	return instance_;
}

void SkyBoxCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvmanager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvmanager;

	graphicsPipeline_ = new GraphicsPipeline();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateSkybox();

}

void SkyBoxCommon::Finalize()
{

	delete graphicsPipeline_;
	if (instance_ != nullptr)
	{
		delete instance_;
		instance_ = nullptr;
	}

}

void SkyBoxCommon::commonDraw()
{

	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureSkybox());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateSkybox());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
