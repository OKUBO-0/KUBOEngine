#pragma once
#include "Bullet.h"
#include <unordered_map>

namespace DirectXGame {

class NormalBullet : public Bullet {
public:
    void InitializeForward(const KamataEngine::Vector3& startPos,
        const KamataEngine::Vector3& forward,
        float speed = 1.0f,
        float range = 30.0f,
        int32_t maxHits = 1);

    void Update(const KamataEngine::Vector3& playerPos, float deltaTime) override;
    void Draw(KamataEngine::Camera* camera) override;

    bool CanHitEnemy(void* enemyPtr);
    void RegisterHit(void* enemyPtr);
    bool ConsumeHit();

    void Deactivate() { active_ = false; }

    KamataEngine::Vector3 GetPosition() const { return worldTransform_.translation_; }

private:
    KamataEngine::Vector3 direction_{ 0,0,0 };
    float speed_ = 1.0f;
    float range_ = 30.0f;
    float traveled_ = 0.0f;
    int32_t remainingHits_ = 1;

    std::unordered_map<void*, float> hitCooldowns_;
    static constexpr float kHitInterval = 0.5f;

    KamataEngine::Audio* audio_ = nullptr;
    uint32_t shotSEHandle_ = 0;
};

} // namespace DirectXGame
