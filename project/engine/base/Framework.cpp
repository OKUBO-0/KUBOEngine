#include "Framework.h"
#include <CameraManager.h>
#include "ParticleManager.h"

void Framework::Initialize()
{
	endRequst_ = false;
	InitializePlatform();
	InitializeRenderServices();
	InitializeEngineServices();
	InitializeDebugTools();
}

void Framework::InitializePlatform()
{
	winApp = std::make_unique<WinApp>();
	winApp->Initialize();
	engineContext_.winApp = winApp.get();
}

void Framework::InitializeRenderServices()
{
	dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp.get());
	engineContext_.dxCommon = dxCommon.get();

	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());
	engineContext_.srvManager = srvManager.get();

	offscreenRenderManager = std::make_unique<OffscreenRenderManager>();
	offscreenRenderManager->Initialize(dxCommon.get(), srvManager.get());
	engineContext_.offscreenRenderManager = offscreenRenderManager.get();

	textureManager = std::make_unique<TextureManager>();
	textureManager->Initialize(dxCommon.get(), srvManager.get());
	engineContext_.textureManager = textureManager.get();
}

void Framework::InitializeEngineServices()
{
	input = std::make_unique<Input>();
	input->Initialize(winApp.get());
	engineContext_.input = input.get();

	audio = std::make_unique<Audio>();
	audio->Initialize();
	engineContext_.audio = audio.get();

	cameraManager = std::make_unique<CameraManager>();
	cameraManager->Initialize();
	engineContext_.cameraManager = cameraManager.get();

	modelManager = std::make_unique<ModelManager>();
	modelManager->Initialize(dxCommon.get(), srvManager.get(), engineContext_.textureManager);
	engineContext_.modelManager = modelManager.get();

	particleManager = std::make_unique<ParticleManager>();
	particleManager->Initialize(dxCommon.get(), srvManager.get(), engineContext_.cameraManager, engineContext_.modelManager, engineContext_.textureManager);
	engineContext_.particleManager = particleManager.get();

	spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize(dxCommon.get());
	spriteCommon->SetCameraManager(engineContext_.cameraManager);
	spriteCommon->SetTextureManager(engineContext_.textureManager);
	engineContext_.spriteCommon = spriteCommon.get();

	object3DCommon = std::make_unique<Object3DCommon>();
	object3DCommon->Initialize(dxCommon.get(),srvManager.get(), engineContext_.cameraManager, engineContext_.modelManager, engineContext_.textureManager);
	engineContext_.object3DCommon = object3DCommon.get();

	lineCommon = std::make_unique<LineCommon>();
	lineCommon->Initialize(dxCommon.get(), srvManager.get(), engineContext_.cameraManager);
	engineContext_.lineCommon = lineCommon.get();

	skyBoxCommon = std::make_unique<SkyBoxCommon>();
	skyBoxCommon->Initialize(dxCommon.get(), srvManager.get(), engineContext_.cameraManager, engineContext_.textureManager);
	engineContext_.skyBoxCommon = skyBoxCommon.get();

	sceneManager = std::make_unique<SceneManager>();
	engineContext_.sceneManager = sceneManager.get();
}

void Framework::InitializeDebugTools()
{
#ifdef _DEBUG
	imGuiMnager = std::make_unique<ImGuiManager>();
	imGuiMnager->Initialize(dxCommon.get(), winApp.get());
	engineContext_.imGuiManager = imGuiMnager.get();
#endif // _DEBUG
}

void Framework::Finalize()
{
	FinalizeDebugTools();
	FinalizeEngineServices();
	ResetOwnedServices();
	engineContext_ = {};
}

void Framework::FinalizeDebugTools()
{
#ifdef _DEBUG
	if (engineContext_.imGuiManager) {
		engineContext_.imGuiManager->Finalize();
	}
	imGuiMnager.reset();
	engineContext_.imGuiManager = nullptr;
#endif // DEBUG
}

void Framework::FinalizeEngineServices()
{
	engineContext_.audio->Finalize();
	engineContext_.winApp->Finalize();
	engineContext_.textureManager->Finalize();
	engineContext_.modelManager->Finalize();
	engineContext_.cameraManager->Finalize();
	engineContext_.particleManager->Finalize();
	engineContext_.input->Finalize();
	engineContext_.spriteCommon->Finalize();
	engineContext_.object3DCommon->Finalize();
	engineContext_.sceneManager->Finalize();
	engineContext_.lineCommon->Finalize();
	engineContext_.skyBoxCommon->Finalize();
}

void Framework::ResetOwnedServices()
{
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
	offscreenRenderManager.reset();
	srvManager.reset();
	dxCommon.reset();
	winApp.reset();
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
