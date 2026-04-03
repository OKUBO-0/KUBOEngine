#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

/// <summary>
/// クラス名: SkyDome
/// 目的: 天球モデルを用いて背景を描画する。
/// 責務: シーン全体の空を表現し、カメラやワールドトランスフォームを管理する。
/// </summary>
class SkyDome {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: 天球モデルの読み込みやワールドトランスフォームの初期設定を行う。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize();

    /// <summary>
    /// 毎フレーム更新処理
    /// 目的: カメラやトランスフォームの更新を行う。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// 目的: 天球モデルを画面に描画する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Draw();
    void SetLightGroup(const KamataEngine::LightGroup* lightGroup);

private:
    KamataEngine::WorldTransform worldTransform_; ///< 天球の位置・回転・スケールを保持するワールドトランスフォーム
    KamataEngine::Camera camera_;                 ///< 天球描画用カメラ（背景専用）
    std::unique_ptr<KamataEngine::Model> skyModel_; ///< 天球モデル（スマートポインタで管理）
};

} // namespace DirectXGame
