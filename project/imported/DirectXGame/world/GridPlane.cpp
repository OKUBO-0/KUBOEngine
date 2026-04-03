#include "GridPlane.h"
#include <cmath>

using namespace KamataEngine;

namespace DirectXGame {

void GridPlane::Initialize() {
    // 床モデルを読み込み（グリッド模様付き）
    planeModel_.reset(Model::CreateFromOBJ("plane"));

    for (auto& worldTransform : worldTransforms_) {
        worldTransform.Initialize();
        worldTransform.translation_ = { 0.0f, -2.0f, 0.0f };
        worldTransform.scale_ = { kGroundScale, 1.0f, kGroundScale };
        worldTransform.rotation_ = { 0.0f, 0.0f, 0.0f };
    }

    // --- UVスケール算出 ---
    // 意図: 床の広さに応じてテクスチャの繰り返し数を決定し、模様を均等に表示する
    uvScaleX_ = kGroundScale / kTileSize;
    uvScaleY_ = kGroundScale / kTileSize;

    if (planeModel_) {
        for (auto& mesh : planeModel_->GetMeshes()) {
            if (!mesh) continue;
            Material* material = mesh->GetMaterial();
            if (!material) continue;

            // グリッド模様を均等に表示するために UVスケールを反映
            material->uvScale_.x = uvScaleX_;
            material->uvScale_.y = uvScaleY_;
            material->uvOffset_.x = 0.0f;
            material->uvOffset_.y = 0.0f;
            material->uvOffset_.z = 0.0f;
            material->Update();
        }
    }
}

void GridPlane::Update(const Vector3& focusPosition) {
    const float centerX = SnapToTile(focusPosition.x);
    const float centerZ = SnapToTile(focusPosition.z);

    int index = 0;
    for (int z = -1; z <= 1; ++z) {
        for (int x = -1; x <= 1; ++x) {
            auto& worldTransform = worldTransforms_[index++];
            worldTransform.translation_.x = centerX + static_cast<float>(x) * kTileSpan;
            worldTransform.translation_.z = centerZ + static_cast<float>(z) * kTileSpan;
            worldTransform.UpdateMatrix();
        }
    }
}

void GridPlane::Draw(Camera* camera) {
    if (!planeModel_) {
        return;
    }

    for (const auto& worldTransform : worldTransforms_) {
        for (auto& mesh : planeModel_->GetMeshes()) {
            if (!mesh) continue;
            Material* material = mesh->GetMaterial();
            if (!material) continue;

            // タイル境界でも模様が連続するよう、ワールド位置に応じてUV位相をずらす
            material->uvOffset_.x = (worldTransform.translation_.x / kTileSize) * 0.5f;
            material->uvOffset_.y = -(worldTransform.translation_.z / kTileSize) * 0.5f;
            material->Update();
        }

        planeModel_->Draw(worldTransform, *camera);
    }
}

void GridPlane::SetLightGroup(const LightGroup* lightGroup) {
    if (planeModel_) {
        planeModel_->SetLightGroup(lightGroup);
    }
}

float GridPlane::SnapToTile(float value) {
    return std::floor(value / kTileSpan) * kTileSpan;
}

} // namespace DirectXGame
