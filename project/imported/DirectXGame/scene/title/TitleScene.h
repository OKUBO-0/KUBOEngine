#pragma once

#include "../core/IScene.h"
#include "../../core/InputBindings.h"
#include "../../effects/CurtainTransition.h"
#include "../../world/SkyDome.h"
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

/// <summary>
/// タイトルシーンを管理するクラス。
/// 背景やUI、演出、BGMを制御し、ゲーム開始前の画面を構成する。
/// </summary>
class TitleScene : public IScene {
public:
    explicit TitleScene(std::shared_ptr<GameSessionContext> sessionContext)
        : IScene(std::move(sessionContext)) {}

    /// <summary>初期化処理（背景スプライトやUI、BGMの準備を行う）</summary>
    void Initialize() override;

    /// <summary>毎フレーム更新処理（入力判定や演出更新を行う）</summary>
    void Update(float deltaTime) override;

    /// <summary>描画処理（背景、UI、演出を画面に描画する）</summary>
    void Draw() override;

    /// <summary>終了処理（リソース解放やシーン終了時の後処理を行う）</summary>
    void Finalize() override;

    /// <summary>シーンが終了状態かどうかを判定する</summary>
    bool IsFinished() const override { return finished_; }

private:
    void InitializeLighting();

    enum class GuideTransitionState {
        None,
        FadeIn,
        FadeOut,
    };

    struct LayoutSettings {
        KamataEngine::Vector2 titlePosition{ 0.0f, 0.0f };
        KamataEngine::Vector2 titleSize{ 1280.0f, 720.0f };
        KamataEngine::Vector2 cursorBasePosition{ 0.0f, 0.0f };
        KamataEngine::Vector2 cursorSize{ 1280.0f, 720.0f };
        float cursorStepY = 120.0f;
        KamataEngine::Vector2 menuHitboxPosition{ 145.0f, 340.0f };
        KamataEngine::Vector2 menuHitboxSize{ 300.0f, 88.0f };
        float menuHitboxStepY = 128.0f;
        KamataEngine::Vector2 guidePosition{ 0.0f, 0.0f };
        KamataEngine::Vector2 guideSize{ 1280.0f, 720.0f };
        KamataEngine::Vector3 modelBasePosition{ 20.0f, -10.0f, 0.0f };
        KamataEngine::Vector3 modelScale{ 4.5f, 4.5f, 4.5f };
        bool debugEnabled = false;
    };

    void ApplyLayout();
    void DrawDebugUI();

    KamataEngine::DirectXCommon* dxCommon_ = nullptr; ///< DirectX管理（外部から取得）
    KamataEngine::Input* input_ = nullptr;            ///< 入力管理（外部から取得）
    KamataEngine::Audio* audio_ = nullptr;            ///< オーディオ管理（外部から取得）

    std::unique_ptr<KamataEngine::Sprite> titleSprite_;      ///< タイトル文字スプライト
    std::unique_ptr<KamataEngine::Sprite> titleUISprite_;    ///< タイトルUIスプライト
    std::unique_ptr<KamataEngine::Sprite> cursorSprite_;
    std::unique_ptr<KamataEngine::Sprite> guideSprite_;      ///< ガイドUIスプライト

    CurtainTransition curtain_;
    bool curtainStarted_ = false;
    bool curtainOpening_ = true;
    bool finished_ = false;       ///< シーン終了フラグ

    uint32_t titleBGMHandle_ = 0; ///< タイトルBGMハンドル
    uint32_t selectSEHandle_ = 0; ///< 選択SEハンドル
    uint32_t decideSEHandle_ = 0;

    int32_t menuIndex_ = 0;

    bool guideActive_ = false;
    GuideTransitionState guideTransitionState_ = GuideTransitionState::None;
    float guideAlpha_ = 0.0f;
    static constexpr float kGuideFadeSpeed_ = 4.5f;
    float animationTime_ = 0.0f;
    InputBindings::NavigationInputDevice navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    LayoutSettings layoutSettings_{};

    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::Camera camera_;
    std::unique_ptr<KamataEngine::Model> playerModel_;
    std::unique_ptr<SkyDome> skyDome_;
    std::unique_ptr<KamataEngine::LightGroup> lightGroup_;
};

} // namespace DirectXGame
