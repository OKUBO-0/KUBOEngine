#include "EnemyBehavior.h"
#include "Enemy.h"
#include "../player/core/Player.h"
#include <KamataEngine.h>
#include <cmath>

using namespace KamataEngine;

namespace DirectXGame {

namespace {

float ToFrameScaledSpeed(float unitsPerFrameBase, float deltaTime) {
    return unitsPerFrameBase * (deltaTime / 0.016f);
}

void MoveEnemy(Enemy& enemy, const Vector3& direction, float speed) {
    Vector3 position = enemy.GetPosition();
    position.x += direction.x * speed;
    position.z += direction.z * speed;
    enemy.SetPosition(position);
    enemy.SetRotationY(std::atan2(direction.x, direction.z));
}

class ChaseEnemyBehavior final : public IEnemyBehavior {
public:
    void Update(Enemy& enemy, float deltaTime) override {
        Player* player = enemy.GetPlayer();
        if (!player) {
            return;
        }

        Vector3 position = enemy.GetPosition();
        Vector3 playerPos = player->GetWorldPosition();
        Vector3 dir = {
            playerPos.x - position.x,
            0.0f,
            playerPos.z - position.z
        };

        const float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);
        if (len <= 0.0f) {
            return;
        }

        dir.x /= len;
        dir.z /= len;

        enemy.ClearBehaviorVisual();
        MoveEnemy(enemy, dir, ToFrameScaledSpeed(enemy.GetSpeed(), deltaTime));
    }
};

class BurstChaseEnemyBehavior final : public IEnemyBehavior {
public:
    void Update(Enemy& enemy, float deltaTime) override {
        Player* player = enemy.GetPlayer();
        if (!player) {
            return;
        }

        Vector3 position = enemy.GetPosition();
        Vector3 playerPos = player->GetWorldPosition();
        Vector3 toPlayer = {
            playerPos.x - position.x,
            0.0f,
            playerPos.z - position.z
        };

        float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
        if (dist <= 0.001f) {
            return;
        }

        toPlayer.x /= dist;
        toPlayer.z /= dist;

        if (dashTimer_ > 0.0f) {
            dashTimer_ -= deltaTime;
            enemy.SetBehaviorVisual({ 1.0f, 0.55f, 0.3f, 1.0f }, 1.12f);
            MoveEnemy(enemy, dashDirection_, ToFrameScaledSpeed(enemy.GetSpeed() * 2.8f, deltaTime));
            return;
        }

        if (dist < 18.0f) {
            windupTimer_ += deltaTime;
            enemy.SetBehaviorVisual({ 1.0f, 0.9f, 0.35f, 1.0f }, 1.18f);
            if (windupTimer_ >= 0.55f) {
                dashDirection_ = toPlayer;
                dashTimer_ = 0.28f;
                windupTimer_ = 0.0f;
                return;
            }
        } else {
            windupTimer_ = (std::max)(0.0f, windupTimer_ - deltaTime * 1.5f);
            enemy.ClearBehaviorVisual();
        }

        MoveEnemy(enemy, toPlayer, ToFrameScaledSpeed(enemy.GetSpeed() * 0.75f, deltaTime));
    }

private:
    float windupTimer_ = 0.0f;
    float dashTimer_ = 0.0f;
    Vector3 dashDirection_ = { 0.0f, 0.0f, 1.0f };
};

class CircleApproachEnemyBehavior final : public IEnemyBehavior {
public:
    void Update(Enemy& enemy, float deltaTime) override {
        Player* player = enemy.GetPlayer();
        if (!player) {
            return;
        }

        Vector3 position = enemy.GetPosition();
        Vector3 playerPos = player->GetWorldPosition();
        Vector3 toPlayer = {
            playerPos.x - position.x,
            0.0f,
            playerPos.z - position.z
        };

        float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
        if (dist <= 0.001f) {
            return;
        }

        toPlayer.x /= dist;
        toPlayer.z /= dist;

        const Vector3 side = { -toPlayer.z, 0.0f, toPlayer.x };
        const float approachWeight = dist > 12.0f ? 1.2f : 0.55f;
        const float sideWeight = 1.1f;
        Vector3 finalDir = {
            toPlayer.x * approachWeight + side.x * sideWeight,
            0.0f,
            toPlayer.z * approachWeight + side.z * sideWeight
        };

        const float len = std::sqrt(finalDir.x * finalDir.x + finalDir.z * finalDir.z);
        if (len <= 0.001f) {
            return;
        }

        finalDir.x /= len;
        finalDir.z /= len;

        enemy.SetBehaviorVisual({ 0.65f, 0.95f, 1.0f, 1.0f }, 0.96f);
        MoveEnemy(enemy, finalDir, ToFrameScaledSpeed(enemy.GetSpeed() * 1.05f, deltaTime));
    }
};

class KeepDistanceRushEnemyBehavior final : public IEnemyBehavior {
public:
    void Update(Enemy& enemy, float deltaTime) override {
        Player* player = enemy.GetPlayer();
        if (!player) {
            return;
        }

        Vector3 position = enemy.GetPosition();
        Vector3 playerPos = player->GetWorldPosition();
        Vector3 toPlayer = {
            playerPos.x - position.x,
            0.0f,
            playerPos.z - position.z
        };

        float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
        if (dist <= 0.001f) {
            return;
        }

        toPlayer.x /= dist;
        toPlayer.z /= dist;

        strafeSwapTimer_ -= deltaTime;
        if (strafeSwapTimer_ <= 0.0f) {
            strafeSign_ *= -1.0f;
            strafeSwapTimer_ = 1.4f;
        }

        rushTimer_ -= deltaTime;
        rushCooldown_ -= deltaTime;
        if (rushCooldown_ <= 0.0f && dist > 8.0f && dist < 22.0f) {
            rushTimer_ = 0.35f;
            rushCooldown_ = 2.3f;
        }

        const Vector3 side = { -toPlayer.z * strafeSign_, 0.0f, toPlayer.x * strafeSign_ };
        float towardWeight = 0.0f;
        if (dist > 16.0f) {
            towardWeight = 1.0f;
        } else if (dist < 9.0f) {
            towardWeight = -1.25f;
        } else {
            towardWeight = 0.18f;
        }

        Vector3 moveDir = {
            toPlayer.x * towardWeight + side.x,
            0.0f,
            toPlayer.z * towardWeight + side.z
        };

        if (rushTimer_ > 0.0f) {
            moveDir = toPlayer;
        }

        const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
        if (len <= 0.001f) {
            return;
        }

        moveDir.x /= len;
        moveDir.z /= len;

        const float speedMultiplier = rushTimer_ > 0.0f ? 2.1f : 1.15f;
        if (rushTimer_ > 0.0f) {
            enemy.SetBehaviorVisual({ 1.0f, 0.35f, 0.35f, 1.0f }, 1.2f);
        } else {
            enemy.SetBehaviorVisual({ 0.82f, 0.72f, 1.0f, 1.0f }, 1.05f);
        }
        MoveEnemy(enemy, moveDir, ToFrameScaledSpeed(enemy.GetSpeed() * speedMultiplier, deltaTime));
    }

private:
    float strafeSign_ = 1.0f;
    float strafeSwapTimer_ = 1.2f;
    float rushTimer_ = 0.0f;
    float rushCooldown_ = 1.6f;
};

} // namespace

std::unique_ptr<IEnemyBehavior> CreateEnemyBehaviorByType(int32_t type) {
    if (type == 1) {
        return std::make_unique<BurstChaseEnemyBehavior>();
    }

    if (type == 2) {
        return std::make_unique<CircleApproachEnemyBehavior>();
    }

    if (type == 3) {
        return std::make_unique<KeepDistanceRushEnemyBehavior>();
    }

    return std::make_unique<ChaseEnemyBehavior>();
}

} // namespace DirectXGame
