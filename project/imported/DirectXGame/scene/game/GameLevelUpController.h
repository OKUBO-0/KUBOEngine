#pragma once

#include "../../core/InputBindings.h"
#include <KamataEngine.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace DirectXGame {

class PlayerManager;

struct LevelUpOption {
    std::string name;
    std::function<void(PlayerManager*)> action;
    std::function<uint32_t(PlayerManager*)> getTexture;
    std::function<uint32_t(PlayerManager*)> getIconTexture;
    float weight = 1.0f;
};

/// <summary>
/// レベルアップ演出と選択処理を管理するクラス。
/// 選択肢の登録、抽選、入力、適用、UI描画を担当する。
/// </summary>
class GameLevelUpController {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: レベルアップUIに必要なスプライトを生成する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize();

    /// <summary>
    /// 既定選択肢登録
    /// 目的: 標準のレベルアップ候補を一覧へ追加する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void RegisterDefaultOptions();

    /// <summary>
    /// レベルアップ開始判定
    /// 目的: プレイヤーのレベルアップ要求を検知し、候補抽選を開始する。
    /// 引数: playerManager - レベルアップ要求元
    /// 引数: audio - 効果音再生に使う音声管理
    /// 引数: levelUpSEHandle - レベルアップ開始SEハンドル
    /// 戻り値: true なら開始した
    /// </summary>
    bool TryStart(PlayerManager* playerManager, KamataEngine::Audio* audio, uint32_t levelUpSEHandle);

    /// <summary>
    /// 更新処理
    /// 目的: 選択入力を受け取り、確定時に強化内容を適用する。
    /// 引数: playerManager - 強化先プレイヤー管理
    /// 引数: input - 入力取得に使う入力管理
    /// 引数: audio - 効果音再生に使う音声管理
    /// 引数: moveSEHandle - 選択移動SEハンドル
    /// 引数: decideSEHandle - 決定SEハンドル
    /// 引数: deltaTime - フレーム経過時間
    /// 戻り値: true ならレベルアップ処理中
    /// </summary>
    bool Update(PlayerManager* playerManager, KamataEngine::Input* input, KamataEngine::Audio* audio,
                uint32_t moveSEHandle, uint32_t decideSEHandle, float deltaTime);

    /// <summary>
    /// 描画処理
    /// 目的: レベルアップUIと選択中候補を画面に表示する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Draw() const;
    void DebugDrawImGui();

    /// <summary>
    /// 状態リセット
    /// 目的: UI状態と選択状態を初期値へ戻す。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Reset();

    /// <summary>
    /// アクティブ判定
    /// 目的: レベルアップUIが有効かどうかを返す。
    /// 引数: なし
    /// 戻り値: true なら表示中
    /// </summary>
    bool IsActive() const { return active_; }

private:
    enum class AnimationState {
        Hidden,
        Entering,
        Idle,
        Exiting,
    };

    struct LayoutSettings {
        KamataEngine::Vector2 choicePositions[3]{
            { 0.0f, 0.0f },
            { 0.0f, 140.0f },
            { 0.0f, 280.0f },
        };
        KamataEngine::Vector2 choiceSize{ 1280.0f, 720.0f };
        KamataEngine::Vector2 arrowBasePosition{ 0.0f, 0.0f };
        float choiceSpacingY = 140.0f;
        KamataEngine::Vector2 choiceHitboxOffset{ 465.0f, 214.0f };
        KamataEngine::Vector2 choiceHitboxSize{ 435.0f, 68.0f };
        bool debugEnabled = false;
    };

    int32_t PickWeightedOptionIndex(const std::vector<LevelUpOption>& candidateOptions) const;
    void ApplyLayout();
    void UpdateSlideAnimation(float deltaTime);
    void SpawnConfetti();
    void UpdateConfetti(float deltaTime);

    std::vector<LevelUpOption> options_;
    std::vector<LevelUpOption> currentChoices_;
    struct ConfettiParticle {
        std::unique_ptr<KamataEngine::Sprite> sprite;
        KamataEngine::Vector2 position{ 0.0f, 0.0f };
        KamataEngine::Vector2 velocity{ 0.0f, 0.0f };
        KamataEngine::Vector2 size{ 0.0f, 0.0f };
        float rotation = 0.0f;
        float angularVelocity = 0.0f;
        float lifetime = 0.0f;
        float age = 0.0f;
    };

    std::unique_ptr<KamataEngine::Sprite> overlaySprite_;
    std::unique_ptr<KamataEngine::Sprite> arrowSprite_;
    std::unique_ptr<KamataEngine::Sprite> choiceSprites_[3];
    std::unique_ptr<KamataEngine::Sprite> iconSprites_[3];
    std::vector<ConfettiParticle> confettiParticles_;
    bool active_ = false;
    int32_t selection_ = 0;
    AnimationState animationState_ = AnimationState::Hidden;
    float slideOffsetX_ = 1280.0f;
    std::function<void(PlayerManager*)> pendingAction_;
    InputBindings::NavigationInputDevice navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    LayoutSettings layoutSettings_{};
};

} // namespace DirectXGame
