#include "OrbitBullet.h"
#include "ModelCache.h"
using namespace KamataEngine;

namespace DirectXGame {

void OrbitBullet::Initialize(const Vector3& center, float radius, float angle, float angularSpeed,
                             float scale, float hitInterval) {
    Bullet::Initialize(center);

    orbitRadius_ = radius;
    angle_ = angle;
    angularSpeed_ = angularSpeed;
    hitInterval_ = hitInterval;

    model_ = ModelCache::Get("bullet");
    worldTransform_.scale_ = { scale, scale, scale };

    worldTransform_.translation_ = {
        center.x + std::cos(angle_) * orbitRadius_,
        center.y,
        center.z + std::sin(angle_) * orbitRadius_
    };

    worldTransform_.UpdateMatrix();
}

void OrbitBullet::Update(const Vector3& center, float deltaTime) {
    if (!active_) return;

    angle_ += angularSpeed_ * (deltaTime / 0.016f);

    worldTransform_.translation_ = {
        center.x + std::cos(angle_) * orbitRadius_,
        center.y,
        center.z + std::sin(angle_) * orbitRadius_
    };

    for (auto it = hitCooldowns_.begin(); it != hitCooldowns_.end();) {
        it->second -= deltaTime;
        if (it->second <= 0.0f) it = hitCooldowns_.erase(it);
        else ++it;
    }

    worldTransform_.UpdateMatrix();
}

void OrbitBullet::Draw(Camera* camera) {
    Bullet::Draw(camera);
}

bool OrbitBullet::CanHitEnemy(void* enemyPtr) {
    auto it = hitCooldowns_.find(enemyPtr);
    if (it == hitCooldowns_.end()) return true;
    return (it->second <= 0.0f);
}

void OrbitBullet::RegisterHit(void* enemyPtr) {
    hitCooldowns_[enemyPtr] = hitInterval_;
}

} // namespace DirectXGame
