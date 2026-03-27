#pragma once

class Audio;
class CameraManager;
class Input;
class ModelManager;
class Object3DCommon;
class ParticleMnager;
class SpriteCommon;
class LineCommon;
class SkyBoxCommon;

struct SceneServices {
	Input* input = nullptr;
	Audio* audio = nullptr;
	CameraManager* cameraManager = nullptr;
	ModelManager* modelManager = nullptr;
	Object3DCommon* object3DCommon = nullptr;
	ParticleMnager* particleManager = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	LineCommon* lineCommon = nullptr;
	SkyBoxCommon* skyBoxCommon = nullptr;
};
