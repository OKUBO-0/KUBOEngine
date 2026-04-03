#include "HitParticle.h"
#include "ModelCache.h"
#include <cstdlib> // rand()
#include <cmath>   // cos, sin
using namespace KamataEngine;

namespace DirectXGame {

namespace {

float ScalePerFrameDecay(float decayPerFrame, float deltaTime) {
    return std::pow(decayPerFrame, deltaTime / 0.016f);
}

}

void HitParticle::Initialize(const Vector3& pos) {
    // モデル生成（火花の見た目）
    model_ = ModelCache::Get("cube");

    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();
    objectColor_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    // ワールド変換初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;

    // 初期スケールをやや大きめにしてから徐々に縮むようにする
    initialScale_ = 0.2f;
    enlargedScale_ = 0.6f;
    worldTransform_.scale_ = { enlargedScale_, enlargedScale_, enlargedScale_ };

    // 初期状態
    age_ = 0.0f;
    alpha_ = 1.0f;
    active_ = true;
    bounceCount_ = 0;

    // ランダムな水平成分と強めの上向き成分を与える（はじけて上に飛ぶ）
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
    float horizSpeed = 0.08f + static_cast<float>(rand()) / RAND_MAX * 0.12f;
    float upSpeed = 0.18f + static_cast<float>(rand()) / RAND_MAX * 0.12f; // 上向き速度を大きめに
    velocity_ = { cos(angle) * horizSpeed, upSpeed, sin(angle) * horizSpeed };

    // 行列更新
    worldTransform_.UpdateMatrix();
}

void HitParticle::Update(float deltaTime) {
    age_ += deltaTime;

    const float velocityScale = deltaTime / 0.016f;

    // --- 寿命判定 ---
    // 意図: 一定時間経過後にパーティクルを非アクティブ化する
    if (age_ >= kLifetime) {
        active_ = false;
        return;
    }

    // 重力を適用（y方向速度に逐次加算）
    velocity_.y += gravity_ * velocityScale;

    // 位置更新
    worldTransform_.translation_.x += velocity_.x * velocityScale;
    worldTransform_.translation_.y += velocity_.y * velocityScale;
    worldTransform_.translation_.z += velocity_.z * velocityScale;

    // 地面との衝突（地面より下に行ったらバウンド）
    if (worldTransform_.translation_.y <= kGroundY) {
        // 地面にめり込ませない
        worldTransform_.translation_.y = kGroundY;

        if (bounceCount_ < kMaxBounces) {
            // 反発（y速度を反転して減衰）
            velocity_.y = -velocity_.y * 0.45f;
            // 水平成分も減衰させる
            const float bounceDecay = ScalePerFrameDecay(0.6f, deltaTime);
            velocity_.x *= bounceDecay;
            velocity_.z *= bounceDecay;
            bounceCount_++;
        }
        else {
            // バウンドが終わったら徐々にフェードアウトさせる
            alpha_ = 1.0f - (age_ / kLifetime);
            if (alpha_ <= 0.01f) {
                active_ = false;
                return;
            }
            if (objectColor_) {
                objectColor_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
            }
        }
    }
    else {
        // 空中では時間経過により透明度を下げる（オプション）
        alpha_ = 1.0f - (age_ / kLifetime) * 0.6f;
        if (objectColor_) {
            objectColor_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
        }
    }

    // スケール変化：生成時に大きく、時間で元に戻す（または縮小）
    float t = age_ / kLifetime;
    float curScale = (1.0f - t) * enlargedScale_ + t * initialScale_;
    worldTransform_.scale_ = { curScale, curScale, curScale };

    worldTransform_.UpdateMatrix();
}

void HitParticle::Draw(Camera* camera) {
    if (!active_ || !model_) return;
    model_->Draw(worldTransform_, *camera, objectColor_.get());
}

} // namespace DirectXGame
