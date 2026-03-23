#pragma once
#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "Camera.h"
#include "GraphicsPipeline.h"
class SkyBoxCommon
{
public:	
	//インスタンス取得
	static SkyBoxCommon* GetInstance();
	


	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager*srvmanager);

	//終了
	void Finalize();

	void commonDraw();
	

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//srvManager
	SrvManager* GetSrvManager()const { return srvManager_; }

private:
	

	static SkyBoxCommon* instance_;
	// DirectX共通
	DirectXCommon* dxCommon_ = nullptr;
	// シェーダーリソースマネージャー
	SrvManager* srvManager_ = nullptr;
	// パイプライン
	GraphicsPipeline* graphicsPipeline_ = nullptr;
	

};

