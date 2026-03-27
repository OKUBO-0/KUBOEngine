#pragma once
#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "CameraManager.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "Object3DCommon.h"
#include "RenderingData.h"
#include "ModelManager.h"
#include "ParticleMnager.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "EngineContext.h"
#include <imgui.h>
#include "Audio.h"
#include "SrvManager.h"
#include "SceneManager.h"
#include <SceneFactory.h>
#include "OfscreenRenderManager.h"

#include "Linecommon.h"
#include "Line.h"
#include "SkyBoxCommon.h"

class Framework {
public:
	// ゲームの初期化
	virtual void Initialize();
	// 終了
	virtual void Finalize();
	// 更新
	virtual void Update();
	// 描画
	virtual void Draw() = 0;

	void Run();

	// ゲーム終了フラグの取得
	virtual bool IsEndRequest() const { return endRequst_; }

protected:
	EngineContext& GetEngineContext() { return engineContext_; }
	const EngineContext& GetEngineContext() const { return engineContext_; }

public:
	// ゲーム終了フラグ
	bool endRequst_ = false;

	// WinAppのポインタ
	std::unique_ptr<WinApp> winApp;
	// DirectXCommonのポインタ
	std::unique_ptr<DirectXCommon> dxCommon;
	// SrvManagerのポインタ
	std::unique_ptr<SrvManager> srvManager;
	// ImGuiManagerのポインタ
	std::unique_ptr<ImGuiManager> imGuiMnager;
	std::unique_ptr<TextureManager> textureManager;
	std::unique_ptr<Input> input;
	std::unique_ptr<Audio> audio;
	std::unique_ptr<CameraManager> cameraManager;
	std::unique_ptr<ModelManager> modelManager;
	std::unique_ptr<ParticleMnager> particleManager;
	std::unique_ptr<SpriteCommon> spriteCommon;
	std::unique_ptr<Object3DCommon> object3DCommon;
	std::unique_ptr<LineCommon> lineCommon;
	std::unique_ptr<SkyBoxCommon> skyBoxCommon;
	// SceneManagerのポインタ
	std::unique_ptr<SceneManager> sceneManager;
	// SceneFactoryのポインタ
	std::unique_ptr<AbstractSceneFactory> sceneFactory;
	std::unique_ptr<OfscreenRenderManager> ofscreenRenderManager;
	EngineContext engineContext_{};
};
