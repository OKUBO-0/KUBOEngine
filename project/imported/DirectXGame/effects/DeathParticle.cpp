#include "DeathParticle.h"
#include "ModelCache.h"
#include <cmath>
using namespace KamataEngine;

namespace DirectXGame {

namespace {

float ScalePerFrameDecay(float decayPerFrame, float deltaTime) {
    return std::pow(decayPerFrame, deltaTime / 0.016f);
}

}

// パーティクル寿命（調整値）
const float DeathParticle::kLifetime = 0.6f;

void DeathParticle::Initialize(const Vector3& pos) {
    // モデル生成（煙っぽい球体）
    model_ = ModelCache::Get("bullet");

    // ワールド変換初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    worldTransform_.scale_ = { 1.0f, 1.0f, 1.0f };

    // ランダムな速度を設定（煙が拡散する意図）
    velocity_ = {
        (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.05f,
        0.1f + static_cast<float>(rand()) / RAND_MAX * 0.1f,
        (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.05f
    };

    age_ = 0.0f;
    active_ = true;

    // 色管理（灰色から開始）
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();
    objectColor_->SetColor({ 0.5f, 0.5f, 0.5f, 1.0f });
}

void DeathParticle::Update(float deltaTime) {
    age_ += deltaTime;

    const float velocityScale = deltaTime / 0.016f;

    // 寿命を超えたら非アクティブ化
    if (age_ >= kLifetime) {
        active_ = false;
        return;
    }

    // 移動処理（煙が上昇・拡散する意図）
    worldTransform_.translation_.x += velocity_.x * velocityScale;
    worldTransform_.translation_.y += velocity_.y * velocityScale;
    worldTransform_.translation_.z += velocity_.z * velocityScale;

    // 徐々に減速（煙が自然に消える演出）
    const float horizontalDecay = ScalePerFrameDecay(0.95f, deltaTime);
    const float verticalDecay = ScalePerFrameDecay(0.98f, deltaTime);
    velocity_.x *= horizontalDecay;
    velocity_.y *= verticalDecay;
    velocity_.z *= horizontalDecay;

    // スケール拡大（煙が広がる演出）
    float scale = 1.0f + age_ * 0.3f;
    worldTransform_.scale_ = { scale, scale, scale };

    // アルファ減衰（煙が消える演出）
    float alpha = 1.0f - (age_ / kLifetime);
    if (objectColor_) {
        objectColor_->SetColor({ 0.5f, 0.5f, 0.5f, alpha });
    }

    // 行列更新
    worldTransform_.UpdateMatrix();
}

void DeathParticle::Draw(Camera* camera) {
    if (!active_ || !model_) return;
    model_->Draw(worldTransform_, *camera, objectColor_.get());
}

} // namespace DirectXGame
