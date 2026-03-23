#pragma once
#include "DirectXCommon.h"
#include "Camera.h"
#include "GraphicsPipeline.h"
#include "SrvManager.h"

class Object3DCommon
{
public:

	static Object3DCommon* GetInstance();



	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager*srvmanager);

	//終了
	void Finalize();

	//共通描画設定
	void CommonDraw();
	void SkinNingCommonDraw();

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//SrvManager
	SrvManager* GetSrvManager()const { return srvManager_; }

	

private:

	Object3DCommon() = default;
	~Object3DCommon() = default;
	Object3DCommon(const Object3DCommon&) = delete;
	Object3DCommon& operator=(const Object3DCommon&) = delete;

private:

	//インスタンス
	static Object3DCommon* instance_;

	DirectXCommon* dxCommon_;
	SrvManager* srvManager_ = nullptr;


	Camera* defaultCamera = nullptr;

	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;
	std::unique_ptr<GraphicsPipeline> skinningGraphicsPipeline_;
};

