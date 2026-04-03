#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

/// <summary>
/// クラス名: RippleEffect
/// 目的: 水面に広がる波紋エフェクトを管理する。
/// 責務: 一定時間で消滅し、スケールと透明度を変化させることで演出を行う。
/// </summary>
class RippleEffect {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: 波紋エフェクトを指定位置に生成し、初期スケールや透明度を設定する。
    /// 引数: pos - 波紋の生成位置
    /// 戻り値: なし
    /// </summary>
    void Initialize(const KamataEngine::Vector3& pos);

    /// <summary>
    /// 毎フレーム更新処理
    /// 目的: 経過時間に応じてスケール拡大・透明度減少を行い、寿命を管理する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// 描画処理
    /// 目的: 有効状態であれば波紋を描画する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void Draw(KamataEngine::Camera* camera);

    /// <summary>
    /// エフェクトが有効かどうかを判定する
    /// 目的: 外部から波紋の有効状態を確認できるようにする。
    /// 引数: なし
    /// 戻り値: true - 有効 / false - 無効
    /// </summary>
    bool IsActive() const { return active_; }

private:
    KamataEngine::WorldTransform worldTransform_; ///< 波紋の位置・回転・スケールを保持するワールド変換
    std::shared_ptr<KamataEngine::Model> model_;  ///< 波紋モデル（スマートポインタで管理）
    std::unique_ptr<KamataEngine::ObjectColor> objectColor_; ///< インスタンスごとの色・透明度管理

    static constexpr float kLifetime = 1.0f;      ///< 波紋の総寿命（秒）
    float age_ = 0.0f;                            ///< 経過時間（秒）
    float alpha_ = 1.0f;                          ///< 現在の透明度（1.0=不透明, 0.0=完全透明）

    static constexpr float kStartScale = 0.5f;    ///< 初期サイズ（生成直後のスケール）
    static constexpr float kEndScale = 1.5f;      ///< 最大サイズ（寿命終了時のスケール）

    bool active_ = true;                          ///< 有効フラグ（falseなら更新・描画対象外）
};

} // namespace DirectXGame
