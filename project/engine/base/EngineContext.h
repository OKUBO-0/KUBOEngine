#pragma once

#include "SceneServices.h"

class AbstractSceneFactory;
class Audio;
class CameraManager;
class DirectXCommon;
class ImGuiManager;
class Input;
class LineCommon;
class ModelManager;
class Object3DCommon;
class OfscreenRenderManager;
class ParticleMnager;
class SceneManager;
class SpriteCommon;
class SrvManager;
class SkyBoxCommon;
class TextureManager;
class WinApp;

struct EngineContext {
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	SrvManager* srvManager = nullptr;
	ImGuiManager* imGuiManager = nullptr;
	AbstractSceneFactory* sceneFactory = nullptr;
	OfscreenRenderManager* ofscreenRenderManager = nullptr;

	Input* input = nullptr;
	Audio* audio = nullptr;
	CameraManager* cameraManager = nullptr;
	ModelManager* modelManager = nullptr;
	Object3DCommon* object3DCommon = nullptr;
	ParticleMnager* particleManager = nullptr;
	SceneManager* sceneManager = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	TextureManager* textureManager = nullptr;
	LineCommon* lineCommon = nullptr;
	SkyBoxCommon* skyBoxCommon = nullptr;

	[[nodiscard]]
	SceneServices CreateSceneServices() const {
		return SceneServices{
			.input = input,
			.audio = audio,
			.cameraManager = cameraManager,
			.modelManager = modelManager,
			.object3DCommon = object3DCommon,
			.particleManager = particleManager,
			.spriteCommon = spriteCommon,
			.lineCommon = lineCommon,
			.skyBoxCommon = skyBoxCommon,
		};
	}
};
