#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

/// <summary>
/// クラス名: HitParticle
/// 目的: ヒット時に表示される火花パーティクルを管理する。
/// 責務: 寿命や拡散方向を持ち、跳ねて重力で落ちる挙動とサイズ変化を行う。
/// </summary>
class HitParticle {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: パーティクルを生成位置に配置し、速度を設定する。
    /// 引数: pos - 生成位置
    /// 戻り値: なし
    /// </summary>
    void Initialize(const KamataEngine::Vector3& pos);

    /// <summary>
    /// 毎フレーム更新処理
    /// 目的: 時間経過に応じて位置・透明度を変化させる。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// 描画処理
    /// 目的: 有効状態であればパーティクルを描画する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void Draw(KamataEngine::Camera* camera);

    /// <summary>
    /// パーティクルが有効かどうかを判定する
    /// 目的: 外部から寿命状態を確認できるようにする。
    /// 引数: なし
    /// 戻り値: true - 有効 / false - 無効
    /// </summary>
    bool IsActive() const { return active_; }

private:
    KamataEngine::WorldTransform worldTransform_; ///< パーティクルの位置・回転・スケールを保持するワールド変換
    std::shared_ptr<KamataEngine::Model> model_;  ///< 火花の見た目を表現するモデル
    std::unique_ptr<KamataEngine::ObjectColor> objectColor_; ///< インスタンスごとの色・透明度管理

    static constexpr float kLifetime = 0.9f; ///< 火花の寿命（秒） — 少し長めにして落下を表現
    float age_ = 0.0f;                       ///< 経過時間（秒）
    float alpha_ = 1.0f;                     ///< 現在の透明度（1.0=不透明, 0.0=完全透明）
    bool active_ = true;                     ///< 有効フラグ（falseなら更新・描画対象外）

    KamataEngine::Vector3 velocity_{ 0.0f, 0.0f, 0.0f }; ///< 拡散方向の速度ベクトル

    // 落下／バウンド用パラメータ
    float gravity_ = -0.04f;   ///< 重力加速度（フレームごとに加算）
    int bounceCount_ = 0;      ///< 現在のバウンド回数
    static constexpr int kMaxBounces = 2; ///< 最大バウンド回数

    // サイズ変化
    float initialScale_ = 0.2f;    ///< 生成時の基本スケール
    float enlargedScale_ = 0.6f;   ///< 当たりで一時的に大きくするスケール

    // 地面Y座標（グリッドの Y と合わせる）
    static constexpr float kGroundY = -2.0f;
};

} // namespace DirectXGame
