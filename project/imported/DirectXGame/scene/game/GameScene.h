#pragma once

#include "../core/IScene.h"
#include "GameLevelUpController.h"
#include "GamePauseController.h"
#include "GameStartController.h"
#include "../../player/core/Player.h"
#include "../../player/core/PlayerManager.h"
#include "../../enemy/EnemyManager.h"
#include "../../world/GridPlane.h"
#include "../../world/SkyDome.h"
#include "../../effects/CurtainTransition.h"
#include "../../ui/gauge/ExpGauge.h"
#include "../../ui/gauge/HpGauge.h"
#include "../../ui/hud/Timer.h"
#include "../../ui/hud/KeyUI.h"
#include <KamataEngine.h>
#include <cstdint>
#include <memory>
#include <list>

namespace DirectXGame {

/// <summary>
/// ゲームプレイ全体を管理するクラス。
/// プレイヤー、敵、UI、演出、シーン遷移を統合し、1プレイ分の進行を制御する。
/// </summary>
class GameScene : public IScene {
public:
    /// <summary>
    /// コンストラクタ
    /// 目的: シーン間共有データを受け取り、ゲームシーンの基底状態を作る。
    /// 引数: sessionContext - リザルトなどを共有するセッション情報
    /// 戻り値: なし
    /// </summary>
    explicit GameScene(std::shared_ptr<GameSessionContext> sessionContext)
        : IScene(std::move(sessionContext)) {}

    /// <summary>
    /// 初期化処理
    /// 目的: 音声、ゲームオブジェクト、UIを生成してプレイ開始前状態を整える。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// 目的: ゲーム進行、入力、演出、遷移条件を毎フレーム更新する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(float deltaTime) override;

    /// <summary>
    /// 描画処理
    /// 目的: 3DオブジェクトとUIを描画順に従って画面へ出力する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Draw() override;

    /// <summary>
    /// 終了処理
    /// 目的: 次回プレイに持ち越さない状態を初期化する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// シーン終了判定
    /// 目的: SceneManager に遷移可能状態かを通知する。
    /// 引数: なし
    /// 戻り値: true ならシーン終了
    /// </summary>
    bool IsFinished() const override { return finished_; }

private:
    void InitializeAudio();
    void InitializeLighting();
    void InitializeSceneObjects();
    void InitializeUI();
    void ApplyLighting();

    bool UpdateCurtainOpening();
    bool UpdateStartWaiting();
    bool UpdatePauseState();
    bool UpdateLevelUpFlow(float deltaTime);
    bool UpdateGameTimer(float deltaTime);
    bool UpdateDeathFlow(float deltaTime);
    bool FinalizeResultTransition();

    void StartResultTransition();
    void UpdateStatusUI();
    void UpdateGameplay(float deltaTime);
    void DrawDebugUI();
    void DrawWorld();
    void DrawUI();

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    GameStartController startController_;

    std::unique_ptr<Player> player_;
    std::unique_ptr<PlayerManager> playerManager_;
    EnemyManager enemyManager_;

    CurtainTransition curtain_;
    bool curtainCloseStarted_ = false;
    bool curtainOpening_ = true;
    bool finished_ = false;

    GamePauseController pauseController_;

    std::unique_ptr<KamataEngine::Sprite> deathOverlay_;
    float deathAlpha_ = 0.0f;
    bool deathFadeInStarted_ = false;
    bool deathFadeInComplete_ = false;
    bool gameStopped_ = false;

    GameLevelUpController levelUpController_;

    std::unique_ptr<KeyUI> keyUI_;

    std::unique_ptr<ExpGauge> expGauge_;
    std::unique_ptr<HpGauge> hpGauge_;

    std::unique_ptr<GridPlane> gridPlane_;
    std::unique_ptr<SkyDome> skyDome_;
    std::unique_ptr<KamataEngine::LightGroup> lightGroup_;

    uint32_t pauseSEHandle_ = 0;
    uint32_t startSEHandle_ = 0;
    uint32_t deathSEHandle_ = 0;
    uint32_t levelUpSEHandle_ = 0;

    float gameTime_ = 0.0f;
    float gameTimeLimit_ = 300.0f;
    std::unique_ptr<Timer> timer_;
    static constexpr float kDeathFadeSpeedPerSecond_ = 1.25f;
};

} // namespace DirectXGame
