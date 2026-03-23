#include "Framework.h"
#include <CameraManager.h>
#include "ParticleMnager.h"

void Framework::Initialize()
{
	// 初期化
	// Windows API初期化
	endRequst_ = false;

	winApp = std::make_unique<WinApp>();
	winApp->Initialize();
	// DirectX初期化

	dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp.get());
	// SRVマネージャの初期化
	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());
	// オフスクリーンレンダーマネージャの初期化
	ofscreenRenderManager = std::make_unique<OfscreenRenderManager>();
	ofscreenRenderManager->Initialize(dxCommon.get(), srvManager.get());

	//テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());
	//Input初期化
	Input::GetInstance()->Initialize(winApp.get());
	//Audio初期化
	Audio::GetInstance()->Initialize();
	//パーティクル
	ParticleMnager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());
	//camera初期化
	CameraManager::GetInstance()->Initialize();

	//スプライト共通部分の初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon.get());

	//3Dモデルマネージャの初期化
	ModelManager::GetInstans()->Initialize(dxCommon.get(), srvManager.get());

	//3Dオブジェクト共通部の初期化
	Object3DCommon::GetInstance()->Initialize(dxCommon.get(),srvManager.get());

	// Line初期化
	LineCommon::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

	SkyBoxCommon::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

#ifdef _DEBUG
	// ImGuiマネージャの初期化
	imGuiMnager = std::make_unique<ImGuiManager>();
	imGuiMnager->Initialize(dxCommon.get(), winApp.get());
#endif // _DEBUG
}

void Framework::Finalize()
{
#ifdef _DEBUG
	imGuiMnager->Finalize();
#endif // DEBUG

	// Audio解放
	Audio::GetInstance()->Finalize();
	//WindowsAPI終了処理
	winApp->Finalize();
	//WindowsAPI解放
	TextureManager::GetInstance()->Finalize();
	//DirectXCommon解放
	ModelManager::GetInstans()->Finalize();
	//カメラの解放
	CameraManager::GetInstance()->Finalize();
	//パーティクルの解放
	ParticleMnager::GetInstance()->Finalize();

	SkyBoxCommon::GetInstance()->Finalize();

	// ユニークポインタは自動的に解放されるため、deleteは不要
#ifdef _DEBUG
	imGuiMnager.reset();
#endif // _DEBUG

	Input::GetInstance()->Finalize();
	SpriteCommon::GetInstance()->Finalize();
	Object3DCommon::GetInstance()->Finalize();
	SceneManager::GetInstance()->Finalize();
	LineCommon::GetInstance()->Finalize();
}

void Framework::Update()
{
	//Windowsのメッセージ処理
	if (winApp->ProcessMessage()) {
		//ゲームループを抜ける
		endRequst_ = true;
	}

	Input::GetInstance()->Update();
	ParticleMnager::GetInstance()->Update();
	SceneManager::GetInstance()->Update();
	LineCommon::GetInstance()->Update();
}

void Framework::Run()
{
	Initialize();
	while (true)
	{
		Update();

		if (IsEndRequest()) {
			break;
		}
		//描画
		Draw();
	}
	Finalize();
}
