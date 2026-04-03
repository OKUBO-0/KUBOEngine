#pragma once
#include <KamataEngine.h>
#include <array>
#include <memory>

namespace DirectXGame {

/// <summary>
/// クラス名: GridPlane
/// 目的: グリッド床を表示する。
/// 責務: ワールド変換とモデルを保持し、初期化・更新・描画を行う。
/// </summary>
class GridPlane {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: モデルやワールド変換の準備を行う。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize();

    /// <summary>
    /// 毎フレーム更新処理
    /// 目的: 床の状態を更新する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(const KamataEngine::Vector3& focusPosition);

    /// <summary>
    /// 描画処理
    /// 目的: カメラに基づいて床を描画する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void Draw(KamataEngine::Camera* camera);
    void SetLightGroup(const KamataEngine::LightGroup* lightGroup);

private:
    std::array<KamataEngine::WorldTransform, 9> worldTransforms_; ///< 3x3 の床タイル
    std::unique_ptr<KamataEngine::Model> planeModel_; ///< 床のモデル（グリッド模様付き）
    float uvScaleX_ = 1.0f;
    float uvScaleY_ = 1.0f;

    static constexpr float kGroundScale = 160.0f;   ///< 1枚あたりの床サイズ
    static constexpr float kTileSize = 4.0f;       ///< 1グリッドの見た目サイズ
    static constexpr float kTileSpan = kGroundScale * 2.0f; ///< タイル配置間隔

    static float SnapToTile(float value);
};

} // namespace DirectXGame
