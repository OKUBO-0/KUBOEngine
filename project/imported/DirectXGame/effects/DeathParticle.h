#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

/// <summary>
/// クラス名: DeathParticle
/// 目的: 敵撃破時に煙のようなパーティクルを生成する。
/// 責務: モデル・色・寿命を管理し、時間経過に応じて拡散・消滅させる。
/// </summary>
class DeathParticle {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: パーティクルを生成位置に配置し、速度や色を設定する。
    /// 引数: pos - 生成位置
    /// 戻り値: なし
    /// </summary>
    void Initialize(const KamataEngine::Vector3& pos);

    /// <summary>
    /// 更新処理
    /// 目的: 時間経過に応じて位置・速度・スケール・透明度を変化させる。
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
    KamataEngine::WorldTransform worldTransform_; ///< ワールド変換（位置・回転・スケールを保持）
    std::shared_ptr<KamataEngine::Model> model_;  ///< パーティクルモデル（煙の見た目を表現）
    std::unique_ptr<KamataEngine::ObjectColor> objectColor_; ///< 色管理（アルファ値や色変化を制御）

    static const float kLifetime; ///< パーティクル寿命（調整値、秒単位）
    float age_ = 0.0f;            ///< 経過時間（秒）
    bool active_ = true;          ///< 有効フラグ（falseなら描画・更新対象外）

    KamataEngine::Vector3 velocity_{ 0.0f, 0.0f, 0.0f }; ///< 移動速度ベクトル（拡散方向と強さ）
};

} // namespace DirectXGame
