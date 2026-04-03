#pragma once
#include <KamataEngine.h>
#include "../common/UIBar.h"
#include <algorithm>
#include <cstdint>

namespace DirectXGame {

/// <summary>
/// プレイヤーのHPを可視化するゲージUIクラス。
/// 現在のHPと最大HPを基にゲージを描画し、減少時には補間表示を行う。
/// </summary>
class HpGauge {
public:
    /// <summary>初期化処理（スプライトやテクスチャの準備を行う）</summary>
    void Initialize();

    /// <summary>毎フレーム更新処理（HPゲージの補間や表示状態を更新する）</summary>
    void Update();

    /// <summary>描画処理（現在のHPゲージを画面に描画する）</summary>
    void Draw();

    /// <summary>プレイヤーのHPを設定する</summary>
    void SetHP(int32_t current, int32_t max);

    /// <summary>HPがゼロになっているかを判定する</summary>
    bool IsDepleted() const;
    void DebugDrawImGui();
    void SaveLayout() const;

private:
    struct LayoutSettings {
        KamataEngine::Vector2 position{ 600.0f, 450.0f };
        KamataEngine::Vector2 size{ 80.0f, 10.0f };
        bool debugEnabled = false;
    };

    void ApplyLayout();

    UIBar gauge_;

    int32_t displayedHP_ = 0; ///< 表示中のHP（補間用）
    int32_t targetHP_ = 0;    ///< 実際のHP
    int32_t maxHP_ = kDefaultMaxHP; ///< 最大HP（0除算防止）

    static constexpr int32_t kDefaultMaxHP = 1; ///< 最大HPの初期値（0除算防止用）
    LayoutSettings layoutSettings_{};
};

} // namespace DirectXGame
