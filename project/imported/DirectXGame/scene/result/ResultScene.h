#pragma once
#include "../core/IScene.h"
#include "../../effects/CurtainTransition.h"
#include "../../ui/hud/Score.h"
#include <KamataEngine.h>
#include <cstdint>
#include <memory>

namespace DirectXGame {

/// <summary>
/// リザルトシーンを管理するクラス。
/// スコア表示や演出を行い、ゲーム終了後の画面を制御する。
/// </summary>
class ResultScene : public IScene {
public:
    explicit ResultScene(std::shared_ptr<GameSessionContext> sessionContext)
        : IScene(std::move(sessionContext)) {}

    /// <summary>初期化処理（背景やUI、スコア表示の準備を行う）</summary>
    void Initialize() override;

    /// <summary>毎フレーム更新処理（演出やスコアのアニメーションを更新する）</summary>
    void Update(float deltaTime) override;

    /// <summary>描画処理（背景、UI、スコアを画面に描画する）</summary>
    void Draw() override;

    /// <summary>終了処理（リソース解放やシーン終了時の後処理を行う）</summary>
    void Finalize() override;

    /// <summary>シーンが終了状態かどうかを判定する</summary>
    bool IsFinished() const override { return finished_; }

private:
    struct LayoutSettings {
        KamataEngine::Vector2 backgroundPosition{ 0.0f, 0.0f };
        KamataEngine::Vector2 backgroundSize{ 1280.0f, 720.0f };
        KamataEngine::Vector2 resultPosition{ 0.0f, 0.0f };
        KamataEngine::Vector2 resultSize{ 1280.0f, 720.0f };
        KamataEngine::Vector2 resultUIPosition{ 0.0f, 0.0f };
        KamataEngine::Vector2 resultUISize{ 1280.0f, 720.0f };
        KamataEngine::Vector2 expPosition{ 500.0f, 200.0f };
        KamataEngine::Vector2 levelPosition{ 500.0f, 300.0f };
        KamataEngine::Vector2 killPosition{ 500.0f, 400.0f };
        float scoreScale = 2.0f;
        bool debugEnabled = false;
    };

    void ApplyLayout();
    void DrawDebugUI();

    KamataEngine::DirectXCommon* dxCommon_ = nullptr; ///< DirectX管理（外部から取得）
    KamataEngine::Input* input_ = nullptr;            ///< 入力管理（外部から取得）
    KamataEngine::Audio* audio_ = nullptr;            ///< オーディオ管理（外部から取得）

    std::unique_ptr<KamataEngine::Sprite> backgroundSprite_; ///< 背景スプライト
    std::unique_ptr<KamataEngine::Sprite> resultSprite_;     ///< リザルト文字スプライト
    std::unique_ptr<KamataEngine::Sprite> resultUI_;         ///< リザルトUIスプライト

    CurtainTransition curtain_;   ///< カーテン演出
    bool curtainOutStarted_ = false;
    bool curtainOpening_ = true;
    bool finished_ = false;       ///< シーン終了フラグ

    std::unique_ptr<Score> expUI_;   ///< 経験値表示UI
    std::unique_ptr<Score> levelUI_; ///< レベル表示UI
    std::unique_ptr<Score> killUI_;  ///< 撃破数表示UI

    int32_t currentExp_ = 0;       ///< 現在のスコア
    int32_t targetExp_ = 0;      ///< 目標スコア

    int32_t currentLevel_ = 0;    ///< 表示中レベル
    int32_t targetLevel_ = 0;     ///< 最終レベル

    int32_t currentKill_ = 0;     ///< 表示中撃破数
    int32_t targetKill_ = 0;      ///< 最終撃破数

    uint32_t selectSEHandle_ = 0; ///< 選択SEハンドル
    uint32_t countupSEHandle_ = 0;
    int32_t countupSECooldown_ = 0;
    LayoutSettings layoutSettings_{};
};

} // namespace DirectXGame
