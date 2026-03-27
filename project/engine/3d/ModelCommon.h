#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
class TextureManager;
class ModelCommon
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvMnager, TextureManager* textureManager);

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	SrvManager* GetSRVManager() { return srvMnager_; }
	TextureManager* GetTextureManager() const { return textureManager_; }

private:
	DirectXCommon* dxCommon_;
	SrvManager* srvMnager_ = nullptr;
	TextureManager* textureManager_ = nullptr;


};

