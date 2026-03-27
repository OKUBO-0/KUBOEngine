#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include "GraphicsPipeline.h"
#include "SrvManager.h"
class CameraManager;
class ModelManager;
class TextureManager;

class Object3DCommon
{
public:
	Object3DCommon() = default;
	~Object3DCommon() = default;
	Object3DCommon(const Object3DCommon&) = delete;
	Object3DCommon& operator=(const Object3DCommon&) = delete;



	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager*srvmanager, CameraManager* cameraManager, ModelManager* modelManager, TextureManager* textureManager);

	//終了
	void Finalize();

	//共通描画設定
	void CommonDraw();
	void SkinNingCommonDraw();

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//SrvManager
	SrvManager* GetSrvManager()const { return srvManager_; }
	CameraManager* GetCameraManager() const { return cameraManager_; }
	ModelManager* GetModelManager() const { return modelManager_; }
	TextureManager* GetTextureManager() const { return textureManager_; }

	

private:
	DirectXCommon* dxCommon_;
	SrvManager* srvManager_ = nullptr;
	CameraManager* cameraManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;
	std::unique_ptr<GraphicsPipeline> skinningGraphicsPipeline_;
};

