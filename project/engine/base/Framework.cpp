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
	engineContext_.winApp = winApp.get();
	// DirectX初期化

	dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp.get());
	engineContext_.dxCommon = dxCommon.get();
	// SRVマネージャの初期化
	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());
	engineContext_.srvManager = srvManager.get();
	// オフスクリーンレンダーマネージャの初期化
	ofscreenRenderManager = std::make_unique<OfscreenRenderManager>();
	ofscreenRenderManager->Initialize(dxCommon.get(), srvManager.get());
	engineContext_.ofscreenRenderManager = ofscreenRenderManager.get();

	//テクスチャマネージャの初期化
	textureManager = std::make_unique<TextureManager>();
	textureManager->Initialize(dxCommon.get(), srvManager.get());
	engineContext_.textureManager = textureManager.get();
	//Input初期化
	input = std::make_unique<Input>();
	input->Initialize(winApp.get());
	engineContext_.input = input.get();
	//Audio初期化
	audio = std::make_unique<Audio>();
	audio->Initialize();
	engineContext_.audio = audio.get();
	cameraManager = std::make_unique<CameraManager>();
	cameraManager->Initialize();
	engineContext_.cameraManager = cameraManager.get();
	modelManager = std::make_unique<ModelManager>();
	modelManager->Initialize(dxCommon.get(), srvManager.get(), engineContext_.textureManager);
	engineContext_.modelManager = modelManager.get();
	particleManager = std::make_unique<ParticleMnager>();
	particleManager->Initialize(dxCommon.get(), srvManager.get(), engineContext_.cameraManager, engineContext_.modelManager, engineContext_.textureManager);
	engineContext_.particleManager = particleManager.get();

	//スプライト共通部分の初期化
	spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize(dxCommon.get());
	spriteCommon->SetCameraManager(engineContext_.cameraManager);
	spriteCommon->SetTextureManager(engineContext_.textureManager);
	engineContext_.spriteCommon = spriteCommon.get();

	//3Dオブジェクト共通部の初期化
	object3DCommon = std::make_unique<Object3DCommon>();
	object3DCommon->Initialize(dxCommon.get(),srvManager.get(), engineContext_.cameraManager, engineContext_.modelManager, engineContext_.textureManager);
	engineContext_.object3DCommon = object3DCommon.get();

	// Line初期化
	lineCommon = std::make_unique<LineCommon>();
	lineCommon->Initialize(dxCommon.get(), srvManager.get(), engineContext_.cameraManager);
	engineContext_.lineCommon = lineCommon.get();

	skyBoxCommon = std::make_unique<SkyBoxCommon>();
	skyBoxCommon->Initialize(dxCommon.get(), srvManager.get(), engineContext_.cameraManager, engineContext_.textureManager);
	engineContext_.skyBoxCommon = skyBoxCommon.get();
	sceneManager = std::make_unique<SceneManager>();
	engineContext_.sceneManager = sceneManager.get();

#ifdef _DEBUG
	// ImGuiマネージャの初期化
	imGuiMnager = std::make_unique<ImGuiManager>();
	imGuiMnager->Initialize(dxCommon.get(), winApp.get());
	engineContext_.imGuiManager = imGuiMnager.get();
#endif // _DEBUG
}

void Framework::Finalize()
{
#ifdef _DEBUG
	if (engineContext_.imGuiManager) {
		engineContext_.imGuiManager->Finalize();
	}
#endif // DEBUG

	// Audio解放
	engineContext_.audio->Finalize();
	//WindowsAPI終了処理
	engineContext_.winApp->Finalize();
	//WindowsAPI解放
	engineContext_.textureManager->Finalize();
	//DirectXCommon解放
	engineContext_.modelManager->Finalize();
	//カメラの解放
	engineContext_.cameraManager->Finalize();
	//パーティクルの解放
	engineContext_.particleManager->Finalize();

	// ユニークポインタは自動的に解放されるため、deleteは不要
#ifdef _DEBUG
	imGuiMnager.reset();
	engineContext_.imGuiManager = nullptr;
#endif // _DEBUG

	engineContext_.input->Finalize();
	engineContext_.spriteCommon->Finalize();
	engineContext_.object3DCommon->Finalize();
	engineContext_.sceneManager->Finalize();
	engineContext_.lineCommon->Finalize();
	engineContext_.skyBoxCommon->Finalize();
	skyBoxCommon.reset();
	lineCommon.reset();
	object3DCommon.reset();
	spriteCommon.reset();
	particleManager.reset();
	modelManager.reset();
	cameraManager.reset();
	audio.reset();
	input.reset();
	textureManager.reset();
	sceneManager.reset();
	engineContext_ = {};
}

void Framework::Update()
{
	//Windowsのメッセージ処理
	if (winApp->ProcessMessage()) {
		//ゲームループを抜ける
		endRequst_ = true;
	}

	engineContext_.input->Update();
	engineContext_.particleManager->Update();
	engineContext_.sceneManager->Update();
	engineContext_.lineCommon->Update();
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
