#pragma once

#include "Camera.h" 
#include "Model.h"
#include "Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"
#include "GameObject.h"

#include "SceneManager.h"
#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include "Line.h"
#include "SkyBox.h"

/// <summary>
/// ゲームプレイシーン（ゲーム中のメイン処理を担当）
/// BaseScene を継承し、初期化・更新・描画・終了処理を実装
/// </summary>
class GamePlayScene : public BaseScene
{
public:
    /// シーン初期化（カメラ・モデル・パーティクル・サウンドなどの準備）
    void Initialize() override;

    /// シーン終了処理（リソース解放など）
    void Finalize() override;

    /// シーン更新（入力処理・アニメーション・ゲームロジック）
    void Update() override;

    /// シーン描画（3Dモデル・スプライト・パーティクルなど）
    void Draw() override;
    const char* GetSceneName() const override { return "GAMEPLAY"; }
    void DrawEditorImGui() override;

    /// モデル読み込み
    void LoadModel();

    /// パーティクル読み込み
    void Loadparticle();

    /// オーディオ読み込み
    void LoadAudio();

private:
    void InitializeCameras();
    void InitializeSceneObjects();
    void InitializeParticleEmitters();
    void UpdatePlayerInput();
    void UpdateSceneObjects();
    void BuildEditorGameObjects();

#ifdef _DEBUG
    void DrawHierarchyWindow();
    void DrawInspectorWindow();
    void DrawSceneControlInspector();
    void DrawSelectedGameObjectInspector();

    size_t selectedGameObjectIndex_ = 0;
    std::vector<std::unique_ptr<GameObject>> editorGameObjects_;
#endif // _DEBUG

    // カメラ
    std::unique_ptr<Camera> camera1;   // メインカメラ
    std::unique_ptr<Camera> camera2;   // サブカメラ

    // 3Dオブジェクト
    std::unique_ptr<Object3D> object3D; // メインオブジェクト
    std::unique_ptr<Object3D> terrain;  // 地形オブジェクト

    // パーティクル
    std::unique_ptr<ParticleEmitter> particleEmitter;   // パーティクルエミッタ1
    std::unique_ptr<ParticleEmitter> particleEmitter2;  // パーティクルエミッタ2

    // ライト設定フラグ
    bool light = true;           // ライト全体のON/OFF
    bool directionLight = true;  // 平行光源ON/OFF
    bool pointLight = false;     // ポイントライトON/OFF
    bool spotLight = false;      // スポットライトON/OFF

    // 2Dスプライト
    std::unique_ptr<Sprite> sprite;

    // サウンド
    SoundData sampleSoundData; // サンプルサウンドデータ

    // デバッグ用フラグ
    bool number = 0;

    // 線描画
    std::unique_ptr<Line> line;
    Vector3 startline; // 線の始点
    Vector3 endline;   // 線の終点

    // スカイボックス
    std::unique_ptr<SkyBox> skyBox;
};
