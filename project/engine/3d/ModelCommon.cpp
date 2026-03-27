#include "ModelCommon.h"
#include "TextureManager.h"

void ModelCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvMnager, TextureManager* textureManager)
{
	dxCommon_ = dxCommon;
	srvMnager_ = srvMnager;
	textureManager_ = textureManager;

}
