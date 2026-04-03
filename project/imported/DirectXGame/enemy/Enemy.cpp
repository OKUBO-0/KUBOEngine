#include "Enemy.h"
#include "ModelCache.h"
#include <cmath>

using namespace KamataEngine;

namespace DirectXGame {

namespace {

float ScalePerFrameDecay(float decayPerFrame, float deltaTime) {
    return std::pow(decayPerFrame, deltaTime / 0.016f);
}

}

void Enemy::Initialize() {
    worldTransform_.Initialize();
    worldTransform_.translation_ = { 0,0,0 };
    active_ = true;

    if (!objectColor_) {
        objectColor_ = std::make_unique<ObjectColor>();
        objectColor_->Initialize();
        objectColor_->SetColor({ 1,1,1,1 });
    }

    static uint32_t sharedWhiteTextureHandle = 0;
    if (sharedWhiteTextureHandle == 0) {
        sharedWhiteTextureHandle = TextureManager::Load("textures/color/white.png");
    }
    whiteTextureHandle_ = sharedWhiteTextureHandle;

    audio_ = Audio::GetInstance();
    static uint32_t sharedDeathSEHandle = 0;
    if (sharedDeathSEHandle == 0) {
        sharedDeathSEHandle = audio_->LoadWave("audio/se/se_death.wav");
    }
    deathSEHandle_ = sharedDeathSEHandle;
}

void Enemy::SetPosition(const Vector3& pos) {
    worldTransform_.translation_ = pos;
}

void Enemy::SetPlayer(Player* player) {
    player_ = player;
}

void Enemy::SetModelByType(int32_t type) {
    switch (type) {
    case 0: enemyModel_ = ModelCache::Get("Enemy1"); break;
    case 1: enemyModel_ = ModelCache::Get("Enemy2"); break;
    case 2: enemyModel_ = ModelCache::Get("Enemy3"); break;
    case 3: enemyModel_ = ModelCache::Get("Enemy4"); break;
    default: enemyModel_ = ModelCache::Get("octopus"); break;
    }
}

void Enemy::SetBehaviorByType(int32_t type) {
    behavior_ = CreateEnemyBehaviorByType(type);
}

Vector3 Enemy::GetPosition() const {
    return worldTransform_.translation_;
}

bool Enemy::IsActive() const {
    return active_;
}

void Enemy::Deactivate() {
    active_ = false;
}

void Enemy::SetHP(int32_t hp) {
    hp_ = hp;
}

int32_t Enemy::GetHP() const {
    return hp_;
}

void Enemy::SetEXP(int32_t exp) {
    exp_ = exp;
}

int32_t Enemy::GetEXP() const {
    return exp_;
}

bool Enemy::JustDied() const {
    return justDied_;
}

void Enemy::ResetJustDied() {
    justDied_ = false;
}

void Enemy::SetBehaviorVisual(const Vector4& color, float scaleMultiplier) {
    behaviorColor_ = color;
    behaviorScaleMultiplier_ = scaleMultiplier;
}

void Enemy::ClearBehaviorVisual() {
    behaviorColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    behaviorScaleMultiplier_ = 1.0f;
}

void Enemy::Update(float deltaTime) {
    if (!active_) return;

    ClearBehaviorVisual();

    // --- ヒット点滅 ---
    if (hitFlashTimer_ > 0.0f) {
        hitFlashTimer_ -= deltaTime;
    }

    // --- ノックバック ---
    if (knockbackTimer_ > 0.0f) {
        const float velocityScale = deltaTime / 0.016f;
        worldTransform_.translation_.x += knockbackVelocity_.x * velocityScale;
        worldTransform_.translation_.z += knockbackVelocity_.z * velocityScale;

        const float knockbackDecay = ScalePerFrameDecay(0.88f, deltaTime);
        knockbackVelocity_.x *= knockbackDecay;
        knockbackVelocity_.z *= knockbackDecay;

        knockbackTimer_ -= deltaTime;
        if (knockbackTimer_ <= 0.0f) {
            knockbackVelocity_ = { 0,0,0 };
        }
    }
    else {
        if (behavior_) {
            behavior_->Update(*this, deltaTime);
        }
    }

    worldTransform_.scale_ = {
        behaviorScaleMultiplier_,
        behaviorScaleMultiplier_,
        behaviorScaleMultiplier_
    };

    if (objectColor_) {
        if (hitFlashTimer_ > 0.0f) {
            objectColor_->SetColor({ 10, 10, 10, 1 });
        } else {
            objectColor_->SetColor(behaviorColor_);
        }
    }

    worldTransform_.UpdateMatrix();
}

void Enemy::Draw(Camera* camera) {
    if (!active_ || !enemyModel_) return;

    if (hitFlashTimer_ > 0.0f && whiteTextureHandle_ != 0 && objectColor_) {
        enemyModel_->Draw(worldTransform_, *camera, whiteTextureHandle_, objectColor_.get());
    }
    else {
        enemyModel_->Draw(worldTransform_, *camera, objectColor_.get());
    }
}

void Enemy::TakeDamage(int32_t damage, const Vector3& knockDir, float strength) {
    hp_ -= damage;

    if (hp_ <= 0) {
        audio_->PlayWave(deathSEHandle_, false, 1.0f);
        Deactivate();
        justDied_ = true;
        return;
    }

    hitFlashTimer_ = kHitFlashDuration;

    float len = std::sqrt(knockDir.x * knockDir.x + knockDir.z * knockDir.z);
    if (len > 0.001f && strength > 0.0f) {
        Vector3 dir = { knockDir.x / len, 0, knockDir.z / len };
        knockbackVelocity_.x = dir.x * strength;
        knockbackVelocity_.z = dir.z * strength;
        knockbackTimer_ = kKnockbackDuration;
    }
}

} // namespace DirectXGame
