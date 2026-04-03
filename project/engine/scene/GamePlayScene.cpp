#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "SceneServices.h"
#include "CameraManager.h"
#include <Logger.h>
#include "LineCommon.h"
#include "AttackBehavior.h"
#include "MagicCircleBehavior.h"
#include <chrono>
#include <functional>

namespace {
constexpr char kMainCameraName[] = "maincam";
constexpr char kSubCameraName[] = "subcam";
constexpr char kParticle1GroupName[] = "Particle1";
constexpr char kParticle2GroupName[] = "Particle2";

#ifdef _DEBUG
class LambdaInspectorComponent : public Component
{
public:
	LambdaInspectorComponent(std::string typeName, std::function<void()> drawInspector)
		: typeName_(std::move(typeName)), drawInspector_(std::move(drawInspector)) {
	}

	const char* GetTypeName() const override
	{
		return typeName_.c_str();
	}

	void DrawInspectorImGui() override
	{
		drawInspector_();
	}

private:
	std::string typeName_;
	std::function<void()> drawInspector_;
};
#endif // _DEBUG
}

void GamePlayScene::Initialize()
{
	InitializeCameras();

	// ロード処理全体の時間を計測
	auto start = std::chrono::high_resolution_clock::now();

	LoadModel();
	Loadparticle();
	LoadAudio();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	Logger::Log("Total loading time: " + std::to_string(duration.count()) + " milliseconds");

	InitializeSceneObjects();
	InitializeParticleEmitters();
#ifdef _DEBUG
	BuildEditorGameObjects();
#endif // _DEBUG
}

void GamePlayScene::Finalize()
{
	// カメラ破棄
	GetServices().cameraManager->RemoveCamera(kMainCameraName);
	GetServices().cameraManager->RemoveCamera(kSubCameraName);
}

void GamePlayScene::Update()
{
	UpdatePlayerInput();
	UpdateSceneObjects();
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画
	GetServices().object3DCommon->CommonDraw();
	terrain->Draw();

	GetServices().object3DCommon->SkinNingCommonDraw();
	object3D->DrawSkinning();

	//ParticleManager::GetInstance()->Draw();
	//LineCommon::GetInstance()->Draw();

	//スカイボックスの描画
	//skyBox->Draw();
#pragma endregion

#pragma region スプライト描画
	// スプライト描画
	GetServices().spriteCommon->CommonDraw();
	// sprite->Draw();
#pragma endregion
}

void GamePlayScene::LoadModel()
{
	// 必要なモデルを事前ロード
	GetServices().modelManager->LoadModel("axis.obj");
	GetServices().modelManager->LoadModel("plane.gltf");
	GetServices().modelManager->LoadModel("sphere.obj");
	GetServices().modelManager->LoadModel("terrain.obj");
	GetServices().modelManager->LoadModel("animationfly.gltf");
	GetServices().modelManager->LoadModel("sphere.gltf");
	GetServices().modelManager->LoadModel("player.gltf");
	GetServices().modelManager->LoadModel("walk.gltf");
	GetServices().modelManager->LoadModel("testanimation.gltf");
}

void GamePlayScene::Loadparticle()
{

}

void GamePlayScene::LoadAudio()
{
	// BGM読み込み（現在は wav のみ対応）
	sampleSoundData = GetServices().audio->SoundLoadWave("Resources/gamePlayBGM.wav");
}

void GamePlayScene::InitializeCameras()
{
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10 });
	GetServices().cameraManager->AddCamera(kMainCameraName, camera1.get());

	camera2 = std::make_unique<Camera>();
	camera2->SetTranslate(Vector3(0, 6.0f, -20.0f));
	camera2->SetRotate({ 0.35f,0.0f,0.0f });
	GetServices().cameraManager->AddCamera(kSubCameraName, camera2.get());

	GetServices().cameraManager->SetActiveCamera(kMainCameraName);
}

void GamePlayScene::InitializeSceneObjects()
{
	skyBox = std::make_unique<SkyBox>();
	skyBox->Initialize(GetServices().skyBoxCommon, "Resources/test.dds");

	object3D = std::make_unique<Object3D>();
	object3D->Initialize(GetServices().object3DCommon);
	object3D->SetModel("walk.gltf");
	object3D->SetLighting(true);
	object3D->SetPointLightEnable(false);
	object3D->SetDirectionalLightIntensity(1.0f);
	object3D->SetRotate({ 0.0f,-3.0f,0.0f });
	object3D->setskyboxfilepath(skyBox->GetTextureFilePath());

	terrain = std::make_unique<Object3D>();
	terrain->Initialize(GetServices().object3DCommon);
	terrain->SetModel("terrain.obj");
	terrain->SetTransform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,-1.0f,0.0f} });
	terrain->SetLighting(true);
	terrain->SetDirectionalLightIntensity(1.0f);
	terrain->setskyboxfilepath(skyBox->GetTextureFilePath());

	sprite = std::make_unique<Sprite>();
	sprite->Initialize(GetServices().spriteCommon, "Resources/uvChecker.png");

	line = std::make_unique<Line>();
	line->Initialize(GetServices().lineCommon);
	startline = { 0,0,0 };
	endline = { 1,0,0 };
	light = true;
}

void GamePlayScene::InitializeParticleEmitters()
{
	GetServices().particleManager->CreateParticleGroup(kParticle1GroupName, "Resources/gradationLine.png", VerticesType::Cylinder, std::make_unique<MagicCircleBehavior>());
	GetServices().particleManager->CreateParticleGroup(kParticle2GroupName, "Resources/gradationLine.png", VerticesType::Ring, std::make_unique<AttackBehavior>());

	particleEmitter = std::make_unique<ParticleEmitter>(Vector3(0, 0, 0), 1.0f, 0.0f, 10, kParticle2GroupName, GetServices().particleManager);
	particleEmitter2 = std::make_unique<ParticleEmitter>(Vector3(0, 0, 0), 1.0f, 0.0f, 1, kParticle1GroupName, GetServices().particleManager);
}

void GamePlayScene::UpdatePlayerInput()
{
	const float velocity = 0.05f;
	float lx = GetServices().input->GetGamePadStickX();
	float ly = GetServices().input->GetGamePadStickY();

	Vector3 translate = object3D->GetTransform().translate;
	translate.x += lx * velocity;
	translate.z += ly * velocity;
	object3D->SetTranslate(translate);

	if (GetServices().input->GetGamePadTrigger()) {
		object3D->SetScale(object3D->GetTransform().scale + Vector3(-0.01f, -0.01f, -0.01f));
	}
	if (GetServices().input->GetGamePadTrigger(1)) {
		object3D->SetScale(object3D->GetTransform().scale + Vector3(0.01f, 0.01f, 0.01f));
	}

	if (GetServices().input->PushGamePadButton(XINPUT_GAMEPAD_LEFT_SHOULDER)) {
		object3D->SetRotate(object3D->GetTransform().rotate + Vector3(0.0f, 0.01f, 0.0f));
	}
	if (GetServices().input->PushGamePadButton(XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
		object3D->SetRotate(object3D->GetTransform().rotate + Vector3(0.0f, -0.01f, 0.0f));
	}

	if (GetServices().input->TriggerGamePadButton(XINPUT_GAMEPAD_A)) {
		number = !number;
	}
}

void GamePlayScene::UpdateSceneObjects()
{
	skyBox->Update();
	GetServices().cameraManager->GetActiveCamera()->Update();
	object3D->Update();
	terrain->Update();
	particleEmitter->Update();
	particleEmitter2->Update();
	sprite->Update();
}

#ifdef _DEBUG
void GamePlayScene::BuildEditorGameObjects()
{
	editorGameObjects_.clear();

	auto mainCameraObject = std::make_unique<GameObject>("Main Camera");
	mainCameraObject->AddComponent<LambdaInspectorComponent>("Camera", [this]() {
		ImGui::Text("Active: %s", GetServices().cameraManager->GetActiveCamera() == camera1.get() ? "Yes" : "No");
		if (GetServices().cameraManager->GetActiveCamera() != camera1.get() && ImGui::Button("Set Active Camera")) {
			GetServices().cameraManager->SetActiveCamera(kMainCameraName);
		}

		EulerTransform transform = camera1->GetTransform();
		if (ImGui::DragFloat3("Position", &transform.translate.x, 0.01f)) {
			camera1->SetTranslate(transform.translate);
		}
		if (ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f)) {
			camera1->SetRotate(transform.rotate);
		}
	});
	editorGameObjects_.push_back(std::move(mainCameraObject));

	auto subCameraObject = std::make_unique<GameObject>("Sub Camera");
	subCameraObject->AddComponent<LambdaInspectorComponent>("Camera", [this]() {
		ImGui::Text("Active: %s", GetServices().cameraManager->GetActiveCamera() == camera2.get() ? "Yes" : "No");
		if (GetServices().cameraManager->GetActiveCamera() != camera2.get() && ImGui::Button("Set Active Camera")) {
			GetServices().cameraManager->SetActiveCamera(kSubCameraName);
		}

		EulerTransform transform = camera2->GetTransform();
		if (ImGui::DragFloat3("Position", &transform.translate.x, 0.01f)) {
			camera2->SetTranslate(transform.translate);
		}
		if (ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f)) {
			camera2->SetRotate(transform.rotate);
		}
	});
	editorGameObjects_.push_back(std::move(subCameraObject));

	auto playerObject = std::make_unique<GameObject>("Player");
	playerObject->AddComponent<LambdaInspectorComponent>("Transform", [this]() {
		EulerTransform transform = object3D->GetTransform();
		if (ImGui::DragFloat3("Position", &transform.translate.x, 0.01f)) {
			object3D->SetTransform(transform);
		}
		if (ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f)) {
			object3D->SetTransform(transform);
		}
		if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f)) {
			object3D->SetTransform(transform);
		}
	});
	playerObject->AddComponent<LambdaInspectorComponent>("Mesh Renderer", [this]() {
		Vector4 color = object3D->GetColor();
		if (ImGui::ColorEdit4("Color", &color.x)) {
			object3D->SetColor(color);
		}
		if (ImGui::Checkbox("Enable Lighting", &light)) {
			object3D->SetLighting(light);
		}
	});
	playerObject->AddComponent<LambdaInspectorComponent>("Lighting", [this]() {
		if (ImGui::Checkbox("Directional Light", &directionLight)) {
			object3D->SetDirectionalLightEnable(directionLight);
			terrain->SetDirectionalLightEnable(directionLight);
		}
		if (ImGui::Checkbox("Point Light", &pointLight)) {
			object3D->SetPointLightEnable(pointLight);
			terrain->SetPointLightEnable(pointLight);
		}
		if (ImGui::Checkbox("Spot Light", &spotLight)) {
			object3D->SetSpotLightEnable(spotLight);
			terrain->SetSpotLightEnable(spotLight);
		}

		Vector4 directionalColor = object3D->GetDirectionalLight().color;
		Vector3 directionalDirection = object3D->GetDirectionalLight().direction;
		float directionalIntensity = object3D->GetDirectionalLight().intensity;
		if (ImGui::ColorEdit4("Dir Color", &directionalColor.x)) {
			object3D->SetDirectionalLightColor(directionalColor);
			terrain->SetDirectionalLightColor(directionalColor);
		}
		if (ImGui::DragFloat3("Dir Direction", &directionalDirection.x, 0.01f)) {
			object3D->SetDirectionalLightDirection(directionalDirection);
			terrain->SetDirectionalLightDirection(directionalDirection);
		}
		if (ImGui::DragFloat("Dir Intensity", &directionalIntensity, 0.01f)) {
			object3D->SetDirectionalLightIntensity(directionalIntensity);
			terrain->SetDirectionalLightIntensity(directionalIntensity);
		}

		Vector4 pointColorValue = object3D->GetPointLight().color;
		Vector3 pointPositionValue = object3D->GetPointLight().position;
		float pointIntensityValue = object3D->GetPointLight().intensity;
		float pointRadiusValue = object3D->GetPointLightRadius();
		float pointDecayValue = object3D->GetPointLightDecay();
		if (ImGui::ColorEdit4("Point Color", &pointColorValue.x)) {
			object3D->SetPointLightColor(pointColorValue);
			terrain->SetPointLightColor(pointColorValue);
		}
		if (ImGui::DragFloat3("Point Position", &pointPositionValue.x, 0.01f)) {
			object3D->SetPointLightPosition(pointPositionValue);
			terrain->SetPointLightPosition(pointPositionValue);
		}
		if (ImGui::DragFloat("Point Intensity", &pointIntensityValue, 0.01f)) {
			object3D->SetPointLightIntensity(pointIntensityValue);
			terrain->SetPointLightIntensity(pointIntensityValue);
		}
		if (ImGui::DragFloat("Point Radius", &pointRadiusValue, 0.01f)) {
			object3D->SetPointLightRadius(pointRadiusValue);
			terrain->SetPointLightRadius(pointRadiusValue);
		}
		if (ImGui::DragFloat("Point Decay", &pointDecayValue, 0.01f)) {
			object3D->SetPointLightDecay(pointDecayValue);
			terrain->SetPointLightDecay(pointDecayValue);
		}

		Vector4 spotColorValue = object3D->GetSpotLight().color;
		Vector3 spotPositionValue = object3D->GetSpotLight().position;
		Vector3 spotDirectionValue = object3D->GetSpotLight().direction;
		float spotIntensityValue = object3D->GetSpotLight().intensity;
		float spotDistanceValue = object3D->GetSpotLight().distance;
		float spotDecayValue = object3D->GetSpotLight().decay;
		float spotAngleValue = object3D->GetSpotLight().consAngle;
		float spotFalloffValue = object3D->GetSpotLight().cosFalloffstrt;
		if (ImGui::ColorEdit4("Spot Color", &spotColorValue.x)) {
			object3D->SetSpotLightColor(spotColorValue);
			terrain->SetSpotLightColor(spotColorValue);
		}
		if (ImGui::DragFloat3("Spot Position", &spotPositionValue.x, 0.01f)) {
			object3D->SetSpotLightPosition(spotPositionValue);
			terrain->SetSpotLightPosition(spotPositionValue);
		}
		if (ImGui::DragFloat3("Spot Direction", &spotDirectionValue.x, 0.01f)) {
			object3D->SetSpotLightDirection(spotDirectionValue);
			terrain->SetSpotLightDirection(spotDirectionValue);
		}
		if (ImGui::DragFloat("Spot Intensity", &spotIntensityValue, 0.01f)) {
			object3D->SetSpotLightIntensity(spotIntensityValue);
			terrain->SetSpotLightIntensity(spotIntensityValue);
		}
		if (ImGui::DragFloat("Spot Distance", &spotDistanceValue, 0.01f)) {
			object3D->SetSpotLightDistance(spotDistanceValue);
			terrain->SetSpotLightDistance(spotDistanceValue);
		}
		if (ImGui::DragFloat("Spot Decay", &spotDecayValue, 0.01f)) {
			object3D->SetSpotLightDecay(spotDecayValue);
			terrain->SetSpotLightDecay(spotDecayValue);
		}
		if (ImGui::DragFloat("Spot Cone", &spotAngleValue, 0.01f)) {
			object3D->SetSpotLightConsAngle(spotAngleValue);
			terrain->SetSpotLightConsAngle(spotAngleValue);
		}
		if (ImGui::DragFloat("Spot Falloff", &spotFalloffValue, 0.01f)) {
			object3D->SetSpotLightCosFalloffstrt(spotFalloffValue);
			terrain->SetSpotLightCosFalloffstrt(spotFalloffValue);
		}
	});
	editorGameObjects_.push_back(std::move(playerObject));

	auto terrainObject = std::make_unique<GameObject>("Terrain");
	terrainObject->AddComponent<LambdaInspectorComponent>("Transform", [this]() {
		EulerTransform transform = terrain->GetTransform();
		if (ImGui::DragFloat3("Position", &transform.translate.x, 0.01f)) {
			terrain->SetTransform(transform);
		}
		if (ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f)) {
			terrain->SetTransform(transform);
		}
		if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f)) {
			terrain->SetTransform(transform);
		}
	});
	terrainObject->AddComponent<LambdaInspectorComponent>("Mesh Renderer", [this]() {
		Vector4 color = terrain->GetColor();
		if (ImGui::ColorEdit4("Color", &color.x)) {
			terrain->SetColor(color);
		}
	});
	editorGameObjects_.push_back(std::move(terrainObject));

	auto spriteObject = std::make_unique<GameObject>("Sprite");
	spriteObject->AddComponent<LambdaInspectorComponent>("Sprite Renderer", [this]() {
		Vector2 position = sprite->GetPosition();
		if (ImGui::DragFloat2("Position", &position.x, 0.01f)) {
			sprite->SetPosition(position);
		}

		float rotation = sprite->GetRotation();
		if (ImGui::DragFloat("Rotation", &rotation, 0.01f)) {
			sprite->SetRotation(rotation);
		}

		Vector2 size = sprite->GetSize();
		if (ImGui::DragFloat2("Size", &size.x, 0.01f)) {
			sprite->SetSize(size);
		}

		Vector4 color = sprite->GetColor();
		if (ImGui::ColorEdit4("Color", &color.x)) {
			sprite->setColor(color);
		}
	});
	editorGameObjects_.push_back(std::move(spriteObject));

	auto particle1Object = std::make_unique<GameObject>("Particle Emitter 1");
	particle1Object->AddComponent<LambdaInspectorComponent>("Particle Emitter", [this]() {
		Vector3 position = particleEmitter->GetPosition();
		if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
			particleEmitter->SetPosition(position);
		}

		float frequency = particleEmitter->GetFrequency();
		if (ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.0f, 10.0f)) {
			particleEmitter->SetFrequency(frequency);
		}

		int count = static_cast<int>(particleEmitter->GetCount());
		if (ImGui::DragInt("Count", &count, 1.0f, 1, 1000)) {
			particleEmitter->SetCount(static_cast<uint32_t>(count));
		}

		if (ImGui::Button("Emit")) {
			particleEmitter->Emit();
		}
	});
	editorGameObjects_.push_back(std::move(particle1Object));

	auto particle2Object = std::make_unique<GameObject>("Particle Emitter 2");
	particle2Object->AddComponent<LambdaInspectorComponent>("Particle Emitter", [this]() {
		Vector3 position = particleEmitter2->GetPosition();
		if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
			particleEmitter2->SetPosition(position);
		}

		float frequency = particleEmitter2->GetFrequency();
		if (ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.0f, 10.0f)) {
			particleEmitter2->SetFrequency(frequency);
		}

		int count = static_cast<int>(particleEmitter2->GetCount());
		if (ImGui::DragInt("Count", &count, 1.0f, 1, 1000)) {
			particleEmitter2->SetCount(static_cast<uint32_t>(count));
		}

		if (ImGui::Button("Emit")) {
			particleEmitter2->Emit();
		}
	});
	editorGameObjects_.push_back(std::move(particle2Object));

	auto environmentObject = std::make_unique<GameObject>("Environment");
	environmentObject->AddComponent<LambdaInspectorComponent>("Reflection Settings", [this]() {
		float reflectionStrength = object3D->GetreflectionStrengthforkankyouMap();
		float roughness = object3D->GetoughnessforkankyouMap();

		if (ImGui::DragFloat("Reflection Strength", &reflectionStrength, 0.01f, 0.0f, 1.0f)) {
			object3D->SetreflectionStrengthforkankyouMap(reflectionStrength);
			terrain->SetreflectionStrengthforkankyouMap(reflectionStrength);
		}

		if (ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f)) {
			object3D->SettoughnessforkankyouMap(roughness);
			terrain->SettoughnessforkankyouMap(roughness);
		}
	});
	environmentObject->AddComponent<LambdaInspectorComponent>("Skybox", [this]() {
		skyBox->imguidebug();
	});
	editorGameObjects_.push_back(std::move(environmentObject));

	if (selectedGameObjectIndex_ >= editorGameObjects_.size()) {
		selectedGameObjectIndex_ = 0;
	}
}

void GamePlayScene::DrawEditorImGui()
{
	DrawHierarchyWindow();
	DrawInspectorWindow();
}

void GamePlayScene::DrawHierarchyWindow()
{
	ImGui::Begin("Hierarchy");
	ImGui::TextUnformatted("GamePlay Scene");
	ImGui::Separator();

	for (size_t index = 0; index < editorGameObjects_.size(); ++index) {
		if (ImGui::Selectable(editorGameObjects_[index]->GetName().c_str(), selectedGameObjectIndex_ == index)) {
			selectedGameObjectIndex_ = index;
		}
	}

	ImGui::End();
}

void GamePlayScene::DrawInspectorWindow()
{
	ImGui::Begin("Inspector");
	DrawSceneControlInspector();
	ImGui::Separator();
	DrawSelectedGameObjectInspector();

	ImGui::End();
}

void GamePlayScene::DrawSceneControlInspector()
{
	ImGui::Text("Active Camera: %s",
		GetServices().cameraManager->GetActiveCamera() == camera1.get() ? "Main Camera" : "Sub Camera");
	ImGui::Text("Runtime Flag: %d", number);
	if (ImGui::Button("Go To GameClear")) {
		GetSceneManager()->ChangeScene("GAMECLEAR");
	}
	ImGui::SameLine();
	if (ImGui::Button("Go To GameOver")) {
		GetSceneManager()->ChangeScene("GAMEOVER");
	}
}

void GamePlayScene::DrawSelectedGameObjectInspector()
{
	if (editorGameObjects_.empty()) {
		ImGui::TextUnformatted("No GameObjects");
		return;
	}

	GameObject* selectedObject = editorGameObjects_[selectedGameObjectIndex_].get();
	ImGui::Text("GameObject: %s", selectedObject->GetName().c_str());
	bool isActive = selectedObject->IsActive();
	if (ImGui::Checkbox("Active", &isActive)) {
		selectedObject->SetActive(isActive);
	}

	for (const auto& component : selectedObject->GetComponents()) {
		if (ImGui::CollapsingHeader(component->GetTypeName(), ImGuiTreeNodeFlags_DefaultOpen)) {
			component->DrawInspectorImGui();
		}
	}
}
#endif // _DEBUG
