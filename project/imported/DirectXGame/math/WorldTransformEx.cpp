#include <KamataEngine.h>

using namespace KamataEngine;
using namespace MathUtility;

/// <summary>
/// ワールド行列を更新する関数。
/// スケーリング → 回転 → 平行移動 の順で行列を合成し、
/// 定数バッファへ転送して描画処理で利用可能にする。
/// </summary>
void WorldTransform::UpdateMatrix() {
    // スケーリング行列を生成
    Matrix4x4 matScale = MakeScaleMatrix(scale_);

    // 各軸の回転行列を生成
    Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
    Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
    Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);

    // 回転行列を合成（Z→X→Y の順で適用）
    Matrix4x4 matRot = matRotZ * matRotX * matRotY;

    // 平行移動行列を生成
    Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

    // スケーリング → 回転 → 平行移動 の順で合成
    matWorld_ = matScale * matRot * matTrans;

    // 定数バッファへ転送
    this->TransferMatrix();
}