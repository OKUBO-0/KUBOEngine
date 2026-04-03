#pragma once

#include "../../core/InputBindings.h"
#include "../../ui/common/UIPanel.h"
#include "../../ui/common/UILabel.h"
#include "../../ui/hud/MiniMap.h"
#include <cstdint>
#include <memory>

namespace KamataEngine {
class Audio;
class DirectXCommon;
class Input;
class Sprite;
}

namespace DirectXGame {

class EnemyManager;
class Player;
class PlayerManager;

/// <summary>
/// ポーズ画面の入力と表示を管理するクラス。
/// ポーズメニュー、ガイド表示、ミニマップ表示、リザルト遷移要求を制御する。
/// </summary>
class GamePauseController {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: ポーズUIとミニマップを生成して初期状態を整える。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// 目的: ポーズ切替、メニュー操作、ガイド表示、遷移要求を更新する。
    /// 引数: player - ミニマップ更新に使うプレイヤー
    /// 引数: enemyManager - ミニマップ更新に使う敵管理
    /// 引数: input - 入力取得に使う入力管理
    /// 引数: audio - 効果音再生に使う音声管理
    /// 引数: toggleSEHandle - ポーズ開閉に使うSEハンドル
    /// 戻り値: true ならポーズ中としてゲーム更新を止める
    /// </summary>
    bool Update(Player* player, const EnemyManager& enemyManager, const PlayerManager& playerManager,
                KamataEngine::Input* input, KamataEngine::Audio* audio, uint32_t toggleSEHandle);

    /// <summary>
    /// 描画処理
    /// 目的: ポーズオーバーレイ、ガイド、カーソル、ミニマップを描画する。
    /// 引数: dxCommon - スプライト描画に使う DirectX 共通オブジェクト
    /// 戻り値: なし
    /// </summary>
    void Draw() const;

    /// <summary>
    /// 状態リセット
    /// 目的: 次回利用に備えてポーズ状態を初期値へ戻す。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Reset();

    /// <summary>
    /// ポーズ有効判定
    /// 目的: 現在ポーズ画面が有効かを返す。
    /// 引数: なし
    /// 戻り値: true ならポーズ中
    /// </summary>
    bool IsActive() const;

    /// <summary>
    /// ガイド表示判定
    /// 目的: ガイドUIが表示中かを返す。
    /// 引数: なし
    /// 戻り値: true ならガイド表示中
    /// </summary>
    bool IsGuideActive() const;

    /// <summary>
    /// リザルト遷移要求判定
    /// 目的: Finish 選択が行われたかを返す。
    /// 引数: なし
    /// 戻り値: true ならリザルトへ遷移すべき
    /// </summary>
    bool ShouldGoResult() const;
    void DebugDrawImGui();

private:
    enum class GuideTransitionState {
        None,
        FadeIn,
        FadeOut,
    };

    struct LayoutSettings {
        KamataEngine::Vector2 buildRowLocalPosition{ 22.0f, 82.0f };
        float buildStepX = 128.0f;
        KamataEngine::Vector2 buildIconSize{ 128.0f, 72.0f };
        KamataEngine::Vector2 menuHitboxPositions[2]{
            { 840.0f, 294.0f },
            { 840.0f, 462.0f },
        };
        KamataEngine::Vector2 menuHitboxSize{ 280.0f, 92.0f };
    };

    bool active_ = false;
    KamataEngine::Audio* audio_ = nullptr;

    std::unique_ptr<KamataEngine::Sprite> pauseOverlay_;
    std::unique_ptr<KamataEngine::Sprite> guideSprite_;
    std::unique_ptr<KamataEngine::Sprite> cursorSprite_;
    std::unique_ptr<MiniMap> miniMap_;
    std::unique_ptr<UIPanel> statsPanel_;
    std::unique_ptr<UILabel> buildNormalLabel_;
    std::unique_ptr<UILabel> buildOrbitLabel_;
    std::unique_ptr<UILabel> buildDroneLabel_;
    std::unique_ptr<UILabel> buildLightningLabel_;
    std::unique_ptr<UILabel> buildAttackLabel_;

    uint32_t selectSEHandle_ = 0;
    uint32_t decideSEHandle_ = 0;

    int32_t menuIndex_ = 0;
    bool guideActive_ = false;
    GuideTransitionState guideTransitionState_ = GuideTransitionState::None;
    float guideAlpha_ = 0.0f;
    static constexpr float kGuideFadeSpeed_ = 4.5f;
    bool goResult_ = false;
    bool debugLayoutEnabled_ = false;
    InputBindings::NavigationInputDevice navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    LayoutSettings layoutSettings_{};

    void ApplyLayout();
    void UpdateStats(const EnemyManager& enemyManager, const PlayerManager& playerManager);
    void UpdateVisibility();
};

} // namespace DirectXGame
