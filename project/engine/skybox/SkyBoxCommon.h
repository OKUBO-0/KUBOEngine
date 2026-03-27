#pragma once
#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "Camera.h"
#include "GraphicsPipeline.h"
class CameraManager;
class TextureManager;
class SkyBoxCommon
{
public:	
	SkyBoxCommon() = default;
	~SkyBoxCommon() = default;
	SkyBoxCommon(const SkyBoxCommon&) = delete;
	SkyBoxCommon& operator=(const SkyBoxCommon&) = delete;
	


	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager*srvmanager, CameraManager* cameraManager, TextureManager* textureManager);

	//終了
	void Finalize();

	void commonDraw();
	

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//srvManager
	SrvManager* GetSrvManager()const { return srvManager_; }
	CameraManager* GetCameraManager() const { return cameraManager_; }
	TextureManager* GetTextureManager() const { return textureManager_; }

private:
	// DirectX共通
	DirectXCommon* dxCommon_ = nullptr;
	// シェーダーリソースマネージャー
	SrvManager* srvManager_ = nullptr;
	CameraManager* cameraManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	// パイプライン
	GraphicsPipeline* graphicsPipeline_ = nullptr;
	

};

