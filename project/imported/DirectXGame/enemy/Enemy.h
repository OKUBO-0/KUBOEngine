#pragma once

#include <KamataEngine.h>
#include <memory>
#include "EnemyBehavior.h"

namespace DirectXGame {

class Player;

class Enemy {
public:
    void Initialize();
    void Update(float deltaTime);
    void Draw(KamataEngine::Camera* camera);

    void SetPosition(const KamataEngine::Vector3& pos);
    void SetRotationY(float rotationY) { worldTransform_.rotation_.y = rotationY; }
    void SetPlayer(Player* player);
    void SetModelByType(int32_t type);
    void SetBehaviorByType(int32_t type);

    KamataEngine::Vector3 GetPosition() const;
    Player* GetPlayer() const { return player_; }

    bool IsActive() const;
    void Deactivate();

    void SetHP(int32_t hp);
    int32_t GetHP() const;

    void TakeDamage(int32_t damage,
        const KamataEngine::Vector3& knockDir = { 0,0,0 },
        float strength = 0.0f);

    void SetEXP(int32_t exp);
    int32_t GetEXP() const;

    bool JustDied() const;
    void ResetJustDied();

    void SetSpeed(float speed) { speed_ = speed; }
    float GetSpeed() const { return speed_; }
    void SetBehaviorVisual(const KamataEngine::Vector4& color, float scaleMultiplier = 1.0f);
    void ClearBehaviorVisual();

private:
    KamataEngine::WorldTransform worldTransform_;
    std::shared_ptr<KamataEngine::Model> enemyModel_;
    std::unique_ptr<IEnemyBehavior> behavior_;

    float speed_ = 0.0f;

    int32_t hp_ = 0;
    int32_t exp_ = 0;
    bool active_ = true;

    Player* player_ = nullptr;

    std::unique_ptr<KamataEngine::ObjectColor> objectColor_;
    KamataEngine::Vector4 behaviorColor_{ 1.0f, 1.0f, 1.0f, 1.0f };
    float behaviorScaleMultiplier_ = 1.0f;
    float hitFlashTimer_ = 0.0f;
    static constexpr float kHitFlashDuration = 0.12f;
    uint32_t whiteTextureHandle_ = 0;

    KamataEngine::Vector3 knockbackVelocity_ = { 0,0,0 };
    float knockbackTimer_ = 0.0f;
    static constexpr float kKnockbackDuration = 0.22f;

    bool justDied_ = false;

    KamataEngine::Audio* audio_ = nullptr;
    uint32_t deathSEHandle_ = 0;
};

} // namespace DirectXGame
