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

void GamePlayScene::Initialize()
{
	// メインカメラ生成と登録
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10 }); // カメラ位置
	GetServices().cameraManager->AddCamera("maincam", camera1.get());

	// サブカメラ生成と登録
	camera2 = std::make_unique<Camera>();
	camera2->SetTranslate(Vector3(0, 6.0f, -20.0f)); // カメラ位置
	camera2->SetRotate({ 0.35f,0.0f,0.0f });         // カメラ角度
	GetServices().cameraManager->AddCamera("subcam", camera2.get());

	// デフォルトカメラをメインに設定
	GetServices().cameraManager->SetActiveCamera("maincam");

	// ロード処理全体の時間を計測
	auto start = std::chrono::high_resolution_clock::now();

	LoadModel();
	Loadparticle();
	LoadAudio();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	Logger::Log("Total loading time: " + std::to_string(duration.count()) + " milliseconds");

	// スカイボックス初期化
	skyBox = std::make_unique<SkyBox>();
	skyBox->Initialize(GetServices().skyBoxCommon, "Resources/test.dds");

	// プレイヤーモデル初期化
    object3D = std::make_unique<Object3D>();
    object3D->Initialize(GetServices().object3DCommon);
    object3D->SetModel("walk.gltf");
    object3D->SetLighting(true);
    object3D->SetPointLightEnable(false);
    object3D->SetDirectionalLightIntensity(1.0f);
    object3D->SetRotate({ 0.0f,-3.0f,0.0f });
    object3D->setskyboxfilepath(skyBox->GetTextureFilePath());

	// 地形モデル初期化
    terrain = std::make_unique<Object3D>();
    terrain->Initialize(GetServices().object3DCommon);
    terrain->SetModel("terrain.obj");
    terrain->SetTransform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,-1.0f,0.0f} });
    terrain->SetLighting(true);
    terrain->SetDirectionalLightIntensity(1.0f);
    terrain->setskyboxfilepath(skyBox->GetTextureFilePath());

	// スプライト初期化
    sprite = std::make_unique<Sprite>();
    sprite->Initialize(GetServices().spriteCommon, "Resources/uvChecker.png");

	light = true;

	// パーティクル初期化
	GetServices().particleManager->CreateParticleGroup("Particle1", "Resources/gradationLine.png", VerticesType::Cylinder, std::make_unique<MagicCircleBehavior>());
	GetServices().particleManager->CreateParticleGroup("Particle2", "Resources/gradationLine.png", VerticesType::Ring, std::make_unique<AttackBehavior>());

	particleEmitter = std::make_unique<ParticleEmitter>(Vector3(0, 0, 0), 1.0f, 0.0f, 10, "Particle2", GetServices().particleManager);
	particleEmitter2 = std::make_unique<ParticleEmitter>(Vector3(0, 0, 0), 1.0f, 0.0f, 1, "Particle1", GetServices().particleManager);

	// 線描画用
	line = std::make_unique<Line>();
	line->Initialize(GetServices().lineCommon);
	startline = { 0,0,0 };
	endline = { 1,0,0 };
}

void GamePlayScene::Finalize()
{
	// カメラ破棄
	GetServices().cameraManager->RemoveCamera("maincam");
	GetServices().cameraManager->RemoveCamera("subcam");
}

void GamePlayScene::Update()
{
	// スカイボックス更新
	skyBox->Update();

	float velocity = 0.05f;

	// ゲームパッド入力による移動
	float lx = GetServices().input->GetGamePadStickX();
	float ly = GetServices().input->GetGamePadStickY();

	Vector3 translate = object3D->GetTransform().translate;
	translate.x += lx * velocity;
	translate.z += ly * velocity;
	object3D->SetTranslate(translate);

	// LTで縮小 / RTで拡大
	if (GetServices().input->GetGamePadTrigger()) {
		object3D->SetScale(object3D->GetTransform().scale + Vector3(-0.01f, -0.01f, -0.01f));
	}
	if (GetServices().input->GetGamePadTrigger(1)) {
		object3D->SetScale(object3D->GetTransform().scale + Vector3(0.01f, 0.01f, 0.01f));
	}

	// LB/RBで回転
	if (GetServices().input->PushGamePadButton(XINPUT_GAMEPAD_LEFT_SHOULDER)) {
		object3D->SetRotate(object3D->GetTransform().rotate + Vector3(0.0f, 0.01f, 0.0f));
	}
	if (GetServices().input->PushGamePadButton(XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
		object3D->SetRotate(object3D->GetTransform().rotate + Vector3(0.0f, -0.01f, 0.0f));
	}

	// Aボタンでフラグ切り替え
	if (GetServices().input->TriggerGamePadButton(XINPUT_GAMEPAD_A)) {
		number = !number;
	}

	// カメラ更新
	GetServices().cameraManager->GetActiveCamera()->Update();

	// モデル更新
	object3D->Update();
	terrain->Update();

	// パーティクル更新
	particleEmitter->Update();
	particleEmitter2->Update();

	// スプライト更新
	sprite->Update();

#ifdef _DEBUG

    //==============================
    // Debug Info
    //==============================
    ImGui::Text("number %d", number);

    //==============================
    // Scene Control
    //==============================
    if (ImGui::CollapsingHeader("Scene Control", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("GameClearScene")) {
            GetSceneManager()->ChangeScene("GAMECLEAR");
        }
        if (ImGui::Button("GameOverScene")) {
            GetSceneManager()->ChangeScene("GAMEOVER");
        }
    }

    //==============================
    // Camera Control
    //==============================
    if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (ImGui::Button("Switch to Main Camera")) {
            GetServices().cameraManager->SetActiveCamera("maincam");
        }
        if (ImGui::Button("Switch to Sub Camera")) {
            GetServices().cameraManager->SetActiveCamera("subcam");
        }

        EulerTransform cameraTransform = GetServices().cameraManager->GetActiveCamera()->GetTransform();
        if (ImGui::DragFloat3("Camera Position", &cameraTransform.translate.x, 0.01f)) {
            GetServices().cameraManager->GetActiveCamera()->SetTranslate(cameraTransform.translate);
        }
        if (ImGui::DragFloat3("Camera Rotation", &cameraTransform.rotate.x, 0.01f)) {
            GetServices().cameraManager->GetActiveCamera()->SetRotate(cameraTransform.rotate);
        }
    }

    //==============================
    // Object3D
    //==============================
    if (ImGui::CollapsingHeader("Object3D", ImGuiTreeNodeFlags_DefaultOpen)) {

        EulerTransform transform = object3D->GetTransform();
        if (ImGui::DragFloat3("Object Position", &transform.translate.x, 0.01f)) {
            object3D->SetTransform(transform);
        }
        if (ImGui::DragFloat3("Object Rotation", &transform.rotate.x, 0.01f)) {
            object3D->SetTransform(transform);
        }
        if (ImGui::DragFloat3("Object Scale", &transform.scale.x, 0.01f)) {
            object3D->SetTransform(transform);
        }

        Vector4 color = object3D->GetColor();
        if (ImGui::ColorEdit4("Object Color", &color.x)) {
            object3D->SetColor(color);
        }

        if (ImGui::Checkbox("Enable Lighting", &light)) {
            object3D->SetLighting(light);
        }
    }

    //==============================
    // Sprite
    //==============================
    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {

        Vector2 position = sprite->GetPosition();
        if (ImGui::DragFloat2("Sprite Position", &position.x, 0.01f)) {
            sprite->SetPosition(position);
        }

        float rotation = sprite->GetRotation();
        if (ImGui::DragFloat("Sprite Rotation", &rotation, 0.01f)) {
            sprite->SetRotation(rotation);
        }

        Vector2 scale = sprite->GetSize();
        if (ImGui::DragFloat2("Sprite Scale", &scale.x, 0.01f)) {
            sprite->SetSize(scale);
        }

        Vector4 color = sprite->GetColor();
        if (ImGui::ColorEdit4("Sprite Color", &color.x)) {
            sprite->setColor(color);
        }
    }

    //==============================
    // Lighting
    //==============================

    // Directional Light
    if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {

        Vector4 color = object3D->GetDirectionalLight().color;
        Vector3 direction = object3D->GetDirectionalLight().direction;
        float intensity = object3D->GetDirectionalLight().intensity;

        if (ImGui::ColorEdit4("Color", &color.x)) {
            object3D->SetDirectionalLightColor(color);
        }
        if (ImGui::DragFloat3("Direction", &direction.x, 0.01f)) {
            object3D->SetDirectionalLightDirection(direction);
        }
        if (ImGui::DragFloat("Intensity", &intensity, 0.01f)) {
            object3D->SetDirectionalLightIntensity(intensity);
        }
        if (ImGui::Checkbox("Enable DirectionalLight", &directionLight)) {
            object3D->SetDirectionalLightEnable(directionLight);
        }
    }

    // Point Light
    if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {

        Vector4 color = object3D->GetPointLight().color;
        Vector3 position = object3D->GetPointLight().position;
        float intensity = object3D->GetPointLight().intensity;
        float decay = object3D->GetPointLightDecay();
        float radius = object3D->GetPointLightRadius();

        if (ImGui::Checkbox("Enable PointLight", &pointLight)) {
            object3D->SetPointLightEnable(pointLight);
            terrain->SetPointLightEnable(pointLight);
        }

        if (ImGui::ColorEdit4("pointColor", &color.x)) {
            object3D->SetPointLightColor(color);
        }
        if (ImGui::DragFloat3("pointPosition", &position.x, 0.01f)) {
            object3D->SetPointLightPosition(position);
            terrain->SetPointLightPosition(position);
        }
        if (ImGui::DragFloat("pointIntensity", &intensity, 0.01f)) {
            object3D->SetPointLightIntensity(intensity);
            terrain->SetPointLightIntensity(intensity);
        }
        if (ImGui::DragFloat("pointRadius", &radius, 0.01f)) {
            object3D->SetPointLightRadius(radius);
            terrain->SetPointLightRadius(radius);
        }
        if (ImGui::DragFloat("pointDecay", &decay, 0.01f)) {
            object3D->SetPointLightDecay(decay);
            terrain->SetPointLightDecay(decay);
        }
    }

    // Spot Light
    if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen)) {

        Vector4 color = object3D->GetSpotLight().color;
        Vector3 position = object3D->GetSpotLight().position;
        Vector3 direction = object3D->GetSpotLight().direction;
        float intensity = object3D->GetSpotLight().intensity;
        float distance = object3D->GetSpotLight().distance;
        float decay = object3D->GetSpotLight().decay;
        float consAngle = object3D->GetSpotLight().consAngle;
        float cosFalloffstrt = object3D->GetSpotLight().cosFalloffstrt;

        if (ImGui::Checkbox("Enable SpotLight", &spotLight)) {
            object3D->SetSpotLightEnable(spotLight);
            terrain->SetSpotLightEnable(spotLight);
        }

        if (ImGui::ColorEdit4("spotColor", &color.x)) {
            object3D->SetSpotLightColor(color);
            terrain->SetSpotLightColor(color);
        }
        if (ImGui::DragFloat3("spotPosition", &position.x, 0.01f)) {
            object3D->SetSpotLightPosition(position);
            terrain->SetSpotLightPosition(position);
        }
        if (ImGui::DragFloat3("spotDirection", &direction.x, 0.01f)) {
            object3D->SetSpotLightDirection(direction);
            terrain->SetSpotLightDirection(direction);
        }
        if (ImGui::DragFloat("spotIntensity", &intensity, 0.01f)) {
            object3D->SetSpotLightIntensity(intensity);
            terrain->SetSpotLightIntensity(intensity);
        }
        if (ImGui::DragFloat("spotDistance", &distance, 0.01f)) {
            object3D->SetSpotLightDistance(distance);
            terrain->SetSpotLightDistance(distance);
        }
        if (ImGui::DragFloat("spotDecay", &decay, 0.01f)) {
            object3D->SetSpotLightDecay(decay);
            terrain->SetSpotLightDecay(decay);
        }
        if (ImGui::DragFloat("spotConsAngle", &consAngle, 0.01f)) {
            object3D->SetSpotLightConsAngle(consAngle);
            terrain->SetSpotLightConsAngle(consAngle);
        }
        if (ImGui::DragFloat("spotCosFalloffstrt", &cosFalloffstrt, 0.01f)) {
            object3D->SetSpotLightCosFalloffstrt(cosFalloffstrt);
            terrain->SetSpotLightCosFalloffstrt(cosFalloffstrt);
        }
    }

    //==============================
    // ParticleEmitter
    //==============================
    if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::Text("ParticleEmitter");

        if (ImGui::Button("Emit")) {
            particleEmitter->Emit();
        }

        Vector3 position = particleEmitter->GetPosition();
        if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
            particleEmitter->SetPosition(position);
        }
    }

    //==============================
    // Skybox Debug
    //==============================
    skyBox->imguidebug();

    //==============================
    // Environment Map
    //==============================
    ImGui::Begin("Environment Map");

    float reflectionStrength = object3D->GetreflectionStrengthforkankyouMap();
    float toughness = object3D->GetoughnessforkankyouMap();

    if (ImGui::DragFloat("Reflection Strength", &reflectionStrength, 0.01f, 0.0f, 1.0f)) {
        object3D->SetreflectionStrengthforkankyouMap(reflectionStrength);
        terrain->SetreflectionStrengthforkankyouMap(reflectionStrength);
    }

    if (ImGui::DragFloat("Roughness", &toughness, 0.01f, 0.0f, 1.0f)) {
        object3D->SettoughnessforkankyouMap(toughness);
        terrain->SettoughnessforkankyouMap(toughness);
    }

    ImGui::End();

#endif // _DEBUG
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画
	GetServices().object3DCommon->CommonDraw();
	terrain->Draw();

	GetServices().object3DCommon->SkinNingCommonDraw();
	object3D->DrawSkinning();

	//ParticleMnager::GetInstance()->Draw();
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
