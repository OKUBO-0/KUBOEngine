#include "ExpOrb.h"
#include "ModelCache.h"
#include <random>
#include <cmath>
using namespace KamataEngine;

namespace DirectXGame {

void ExpOrb::Initialize(const Vector3& pos, int32_t expValue) {
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    worldTransform_.scale_ = { 0.7f, 0.7f, 0.7f };
    model_ = ModelCache::Get("ExpOrb"); // 経験値オーブ用モデル
    expValue_ = expValue;

    // 乱数生成（少し跳ねる感じの初期速度）
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

    velocity_ = {
        dist(rng) * 0.05f,
        0.05f,
        dist(rng) * 0.05f
    };

    audio_ = Audio::GetInstance();
    static uint32_t sharedPickupSEHandle = 0;
    if (sharedPickupSEHandle == 0) {
        sharedPickupSEHandle = audio_->LoadWave("audio/se/se_exp.wav");
    }
    pickupSEHandle_ = sharedPickupSEHandle;
}

void ExpOrb::Update(const Vector3& playerPos, float deltaTime) {
    if (!active_) return;

    const float velocityScale = deltaTime / 0.016f;

    // プレイヤーとの距離
    float dx = playerPos.x - worldTransform_.translation_.x;
    float dz = playerPos.z - worldTransform_.translation_.z;
    float distSq = dx * dx + dz * dz;

    // 一定距離以内なら吸い寄せられる
    if (distSq < 70.0f) {
        float dist = std::sqrt(distSq);
        if (dist > 0.001f) {
            dx /= dist; dz /= dist;
            // プレイヤー方向へ加速
            velocity_.x += dx * 0.05f * velocityScale;
            velocity_.z += dz * 0.05f * velocityScale;
        }
    }

    // 移動
    worldTransform_.translation_.x += velocity_.x * velocityScale;
    worldTransform_.translation_.y += velocity_.y * velocityScale;
    worldTransform_.translation_.z += velocity_.z * velocityScale;

    // 徐々に減速
    velocity_.x *= 0.95f;
    velocity_.y *= 0.95f;
    velocity_.z *= 0.95f;

    // 拾われ判定（近距離）
    if (distSq < 4.0f) { // 半径2.0以内
        Audio::GetInstance()->PlayWave(pickupSEHandle_, false, 1.0f);
        active_ = false;
    }

    worldTransform_.UpdateMatrix();
}

void ExpOrb::Draw(Camera* camera) {
    if (!active_ || !model_) return;
    model_->Draw(worldTransform_, *camera);
}

} // namespace DirectXGame
