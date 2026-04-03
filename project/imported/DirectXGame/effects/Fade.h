#pragma once

#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

/// <summary>
/// クラス名: Fade
/// 目的: 画面のフェードイン／フェードアウト演出を管理する。
/// 責務: スプライトのアルファ値を制御し、シーン遷移や演出に利用できるようにする。
/// </summary>
class Fade {
public:
    /// <summary>
    /// フェードの状態を表す列挙型
    /// 目的: フェードの進行状況を管理する。
    /// </summary>
    enum class State {
        kNone,     ///< フェードなし
        kFadeIn,   ///< フェードイン中
        kStay,     ///< 完全表示状態で待機
        kFadeOut,  ///< フェードアウト中
    };

    /// <summary>
    /// フェード処理の初期化
    /// 目的: スプライト生成や初期アルファ値の設定を行う。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize();

    /// <summary>
    /// フェード状態の更新処理
    /// 目的: 状態に応じてアルファ値を変化させる。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// フェードスプライトの描画処理
    /// 目的: 現在のアルファ値に基づいて画面に表示する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void Draw();

    /// <summary>
    /// フェードインを開始する
    /// 目的: 黒から透明へ徐々に変化させる。
    /// 引数: speed - アルファ値の変化速度（デフォルト kDefaultFadeSpeed）
    /// 戻り値: なし
    /// </summary>
    void StartFadeIn(float speed = kDefaultFadeSpeed);

    /// <summary>
    /// フェードアウトを開始する
    /// 目的: 透明から黒へ徐々に変化させる。
    /// 引数: speed - アルファ値の変化速度（デフォルト kDefaultFadeSpeed）
    /// 戻り値: なし
    /// </summary>
    void StartFadeOut(float speed = kDefaultFadeSpeed);

    State GetState() const { return state_; }
    bool IsFinished() const { return finished_; }

private:
    std::unique_ptr<KamataEngine::Sprite> fadeSprite_; ///< フェード用スプライト

    float alpha_ = 1.0f;              ///< 現在のアルファ値
    float speed_ = kDefaultFadeSpeed; ///< アルファ値の変化速度
    State state_ = State::kNone;      ///< 現在のフェード状態
    bool finished_ = false;           ///< フェード完了フラグ

    // --- 定数群 ---
    static constexpr int kScreenWidth = 1280;    ///< 画面幅（固定値）
    static constexpr int kScreenHeight = 720;    ///< 画面高さ（固定値）
    static constexpr const char* kBlackTexturePath = "textures/color/black.png"; ///< 黒テクスチャパス

    static const float kDefaultFadeSpeed; ///< デフォルトのフェード速度（調整値）
};

} // namespace DirectXGame
