#include "EnemyManager.h"
#include "../player/core/PlayerManager.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace KamataEngine;

namespace DirectXGame {

int32_t EnemyManager::ToCellCoord(float value) {
    return static_cast<int32_t>(std::floor(value / kSpatialCellSize));
}

int64_t EnemyManager::MakeCellKey(int32_t cellX, int32_t cellZ) {
    return (static_cast<int64_t>(cellX) << 32) ^ static_cast<uint32_t>(cellZ);
}

size_t EnemyManager::GetActiveEnemyCount() const {
    return static_cast<size_t>(std::count_if(enemies_.begin(), enemies_.end(),
        [](const std::unique_ptr<Enemy>& enemy) {
            return enemy && enemy->IsActive();
        }));
}

void EnemyManager::Initialize(const std::string& csvPath, Player* player, PlayerManager* playerManager) {
    player_ = player;
    playerManager_ = playerManager;

    // 敵タイプ定義を読み込む
    LoadEnemyTypes(csvPath);

    audio_ = Audio::GetInstance();
    hitSEHandle_ = audio_->LoadWave("audio/se/se_hit.wav");
    playerDamageSEHandle_ = audio_->LoadWave("audio/se/se_hit.wav");

    if (playerManager_) {
        playerManager_->SetEnemyManager(this);
    }
}

void EnemyManager::LoadEnemyTypes(const std::string& filePath) {
    enemyTypes_.clear();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        OutputDebugStringA(("CSV読み込み失敗: " + filePath + "\n").c_str());
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string value;

        EnemyTypeData data{};

        // CSV: type, baseHP, baseSpeed, baseEXP, spawnCount
        std::getline(ss, value, ','); data.type = std::stoi(value);
        std::getline(ss, value, ','); data.baseHP = std::stoi(value);
        std::getline(ss, value, ','); data.baseSpeed = std::stof(value);
        std::getline(ss, value, ','); data.baseEXP = std::stoi(value);
        std::getline(ss, value, ','); data.spawnCount = std::stoi(value);

        enemyTypes_.push_back(data);
    }

    file.close();
}

void EnemyManager::SpawnEnemies() {
    if (enemyTypes_.empty() || !player_) return;
    if (GetActiveEnemyCount() >= kMaxActiveEnemies) return;

    // 経過時間で解禁される敵タイプを増やす（20秒ごとに1タイプ解禁）
    int maxIndex = static_cast<int>(elapsedTime_ / kSpawnUnlockInterval);
    maxIndex = std::clamp(maxIndex, 0, (int)enemyTypes_.size() - 1);

    const EnemyTypeData& data = enemyTypes_[rand() % (maxIndex + 1)];

    for (int i = 0; i < data.spawnCount; i++) {
        if (GetActiveEnemyCount() >= kMaxActiveEnemies) {
            break;
        }
        SpawnOneEnemy(data);
    }
}

void EnemyManager::SpawnOneEnemy(const EnemyTypeData& data) {
    Vector3 pPos = player_->GetWorldPosition();

    float angle = (float(rand()) / RAND_MAX) * 2.0f * 3.14159265f;

    Vector3 pos = {
        pPos.x + std::cos(angle) * kSpawnDistance,
        0.0f,
        pPos.z + std::sin(angle) * kSpawnDistance
    };

    auto enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetPlayer(player_);
    enemy->SetPosition(pos);
    enemy->SetModelByType(data.type);
    enemy->SetBehaviorByType(data.type);

    // 時間経過で強化
    int hp = data.baseHP + static_cast<int>(elapsedTime_ / 45.0f);
    int exp = data.baseEXP + static_cast<int>(elapsedTime_ / 35.0f);
    float speed = data.baseSpeed + elapsedTime_ * 0.0015f;

    enemy->SetHP(hp);
    enemy->SetEXP(exp);

    // Enemy に速度設定メソッドがある前提
    enemy->SetSpeed(speed);

    enemies_.push_back(std::move(enemy));
}

void EnemyManager::UpdateSpawnState(float deltaTime) {
    elapsedTime_ += deltaTime;
    spawnTimer_ += deltaTime;
    spawnInterval_ = std::max<float>(kMinSpawnInterval, kBaseSpawnInterval - elapsedTime_ * kSpawnAcceleration);

    while (spawnTimer_ >= spawnInterval_) {
        SpawnEnemies();
        spawnTimer_ -= spawnInterval_;
    }
}

void EnemyManager::SpawnDeathEffects(const Enemy& enemy) {
    ++totalKillCount_;

    auto orb = std::make_unique<ExpOrb>();
    orb->Initialize(enemy.GetPosition(), enemy.GetEXP());
    expOrbs_.push_back(std::move(orb));

    for (int32_t i = 0; i < kDeathParticleSpawnCount; ++i) {
        if (deathParticles_.size() >= kMaxDeathParticles) {
            deathParticles_.pop_front();
        }
        auto particle = std::make_unique<DeathParticle>();
        particle->Initialize(enemy.GetPosition());
        deathParticles_.push_back(std::move(particle));
    }
}

void EnemyManager::UpdateEnemies(float deltaTime) {
    for (auto& enemy : enemies_) {
        if (enemy->IsActive()) {
            enemy->Update(deltaTime);
        }
        else if (enemy->GetHP() <= 0 && enemy->JustDied()) {
            SpawnDeathEffects(*enemy);
            enemy->ResetJustDied();
        }
    }
}

void EnemyManager::RemoveInactiveEnemies() {
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy && !enemy->IsActive() && !enemy->JustDied();
            }),
        enemies_.end());
}

void EnemyManager::RelocateFarEnemies() {
    Vector3 playerPosition = player_->GetWorldPosition();

    for (auto& enemy : enemies_) {
        if (!enemy->IsActive()) {
            continue;
        }

        Vector3 enemyPosition = enemy->GetPosition();
        float dx = enemyPosition.x - playerPosition.x;
        float dz = enemyPosition.z - playerPosition.z;
        float distSq = dx * dx + dz * dz;

        if (distSq <= kRespawnDistance * kRespawnDistance) {
            continue;
        }

        float angle = (float(rand()) / RAND_MAX) * 2.0f * 3.14159265f;
        Vector3 newPos = {
            playerPosition.x + std::cos(angle) * kRespawnRadius,
            0.0f,
            playerPosition.z + std::sin(angle) * kRespawnRadius
        };
        enemy->SetPosition(newPos);
    }
}

void EnemyManager::UpdateEffects(float deltaTime) {
    for (auto it = deathParticles_.begin(); it != deathParticles_.end();) {
        (*it)->Update(deltaTime);
        if (!(*it)->IsActive()) it = deathParticles_.erase(it);
        else ++it;
    }

    for (auto it = expOrbs_.begin(); it != expOrbs_.end();) {
        (*it)->Update(player_->GetWorldPosition(), deltaTime);
        if (!(*it)->IsActive()) {
            playerManager_->AddEXP((*it)->GetEXP());
            it = expOrbs_.erase(it);
        }
        else ++it;
    }

    for (auto it = hitParticles_.begin(); it != hitParticles_.end();) {
        (*it)->Update(deltaTime);
        if (!(*it)->IsActive()) it = hitParticles_.erase(it);
        else ++it;
    }
}

void EnemyManager::BuildActiveEnemySpatialMap(EnemyCellMap& outMap, std::vector<Enemy*>& activeEnemies) const {
    outMap.clear();
    activeEnemies.clear();
    activeEnemies.reserve(enemies_.size());

    for (const auto& enemy : enemies_) {
        if (!enemy || !enemy->IsActive()) {
            continue;
        }

        Enemy* enemyPtr = enemy.get();
        activeEnemies.push_back(enemyPtr);
        const Vector3 position = enemyPtr->GetPosition();
        outMap[MakeCellKey(ToCellCoord(position.x), ToCellCoord(position.z))].push_back(enemyPtr);
    }
}

void EnemyManager::CollectNearbyEnemies(const EnemyCellMap& spatialMap, const Vector3& center, float radius,
                                        std::vector<Enemy*>& outEnemies) const {
    outEnemies.clear();

    const int32_t centerCellX = ToCellCoord(center.x);
    const int32_t centerCellZ = ToCellCoord(center.z);
    const int32_t cellRange = (std::max)(1, static_cast<int32_t>(std::ceil(radius / kSpatialCellSize)));
    for (int32_t z = centerCellZ - cellRange; z <= centerCellZ + cellRange; ++z) {
        for (int32_t x = centerCellX - cellRange; x <= centerCellX + cellRange; ++x) {
            const auto it = spatialMap.find(MakeCellKey(x, z));
            if (it == spatialMap.end()) {
                continue;
            }

            outEnemies.insert(outEnemies.end(), it->second.begin(), it->second.end());
        }
    }
}

void EnemyManager::ResolveEnemySeparation() {
    EnemyCellMap spatialMap;
    std::vector<Enemy*> activeEnemies;
    std::vector<Enemy*> nearbyEnemies;
    BuildActiveEnemySpatialMap(spatialMap, activeEnemies);

    for (Enemy* a : activeEnemies) {
        if (!a || !a->IsActive()) continue;

        CollectNearbyEnemies(spatialMap, a->GetPosition(), kEnemySeparationDistance, nearbyEnemies);
        for (Enemy* b : nearbyEnemies) {
            if (!b || !b->IsActive() || a >= b) continue;

            Vector3 posA = a->GetPosition();
            Vector3 posB = b->GetPosition();

            float dx = posB.x - posA.x;
            float dz = posB.z - posA.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < kEnemySeparationDistance * kEnemySeparationDistance && distSq > 0.0001f) {
                float dist = std::sqrt(distSq);
                float overlap = kEnemySeparationDistance - dist;

                float nx = dx / dist;
                float nz = dz / dist;

                posA.x -= nx * overlap * kEnemySeparationStrength;
                posA.z -= nz * overlap * kEnemySeparationStrength;
                posB.x += nx * overlap * kEnemySeparationStrength;
                posB.z += nz * overlap * kEnemySeparationStrength;

                a->SetPosition(posA);
                b->SetPosition(posB);
            }
        }
    }
}

void EnemyManager::Update(float deltaTime) {
    UpdateSpawnState(deltaTime);
    UpdateEnemies(deltaTime);
    RemoveInactiveEnemies();
    RelocateFarEnemies();
    UpdateEffects(deltaTime);
    ResolveEnemySeparation();
}

void EnemyManager::SpawnHitParticles(const Vector3& position) {
    for (int32_t i = 0; i < kHitParticleSpawnCount; ++i) {
        if (hitParticles_.size() >= kMaxHitParticles) {
            hitParticles_.pop_front();
        }
        auto spark = std::make_unique<HitParticle>();
        spark->Initialize(position);
        hitParticles_.push_back(std::move(spark));
    }
}

bool EnemyManager::TryHandleBulletHit(Enemy& enemy, const Vector3& impactPosition, int32_t damage, float knockStrength) {
    if (audio_) {
        audio_->PlayWave(hitSEHandle_, false, 0.5f);
    }

    Vector3 enemyPosition = enemy.GetPosition();
    Vector3 knockDir = { enemyPosition.x - impactPosition.x, 0, enemyPosition.z - impactPosition.z };
    float len = std::sqrt(knockDir.x * knockDir.x + knockDir.z * knockDir.z);
    if (len > 0.0f) {
        knockDir.x /= len;
        knockDir.z /= len;
    }

    enemy.TakeDamage(damage, knockDir, knockStrength);
    SpawnHitParticles(impactPosition);
    return true;
}

void EnemyManager::CheckNormalBulletCollisions(PlayerManager& playerManager, const EnemyCellMap& spatialMap) {
    int32_t damage = playerManager.GetNormalBulletDamage();
    std::vector<Enemy*> nearbyEnemies;

    for (auto& bullet : playerManager.GetNormalBullets()) {
        if (!bullet->IsActive()) continue;

        const Vector3 bPos = bullet->GetPosition();
        CollectNearbyEnemies(spatialMap, bPos, std::sqrt(kNormalBulletHitDistanceSq), nearbyEnemies);
        for (Enemy* enemy : nearbyEnemies) {
            if (!enemy || !enemy->IsActive()) continue;

            Vector3 ePos = enemy->GetPosition();

            float dx = bPos.x - ePos.x;
            float dz = bPos.z - ePos.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < kNormalBulletHitDistanceSq) {
                if (!bullet->CanHitEnemy(enemy)) continue;

                bullet->RegisterHit(enemy);
                TryHandleBulletHit(*enemy, bPos, damage, 0.8f + damage * 0.18f);
                if (!bullet->ConsumeHit()) {
                    break;
                }
            }
        }
    }
}

void EnemyManager::CheckOrbitBulletCollisions(PlayerManager& playerManager, const EnemyCellMap& spatialMap) {
    int32_t damage = playerManager.GetOrbitBulletDamage();
    std::vector<Enemy*> nearbyEnemies;

    for (auto& orb : playerManager.GetOrbitBullets()) {
        if (!orb->IsActive()) continue;

        const Vector3 oPos = orb->GetPosition();
        CollectNearbyEnemies(spatialMap, oPos, std::sqrt(kOrbitBulletHitDistanceSq), nearbyEnemies);
        for (Enemy* enemy : nearbyEnemies) {
            if (!enemy || !enemy->IsActive()) continue;

            Vector3 ePos = enemy->GetPosition();

            float dx = oPos.x - ePos.x;
            float dz = oPos.z - ePos.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < kOrbitBulletHitDistanceSq) {
                if (!orb->CanHitEnemy(enemy)) continue;

                orb->RegisterHit(enemy);
                TryHandleBulletHit(*enemy, oPos, damage, 0.7f + damage * 0.12f);
            }
        }
    }
}

void EnemyManager::CheckDroneBulletCollisions(PlayerManager& playerManager, const EnemyCellMap& spatialMap) {
    if (!playerManager.HasDrone()) {
        return;
    }

    auto& drone = playerManager.GetDrone();
    std::vector<Enemy*> nearbyEnemies;

    for (auto& bullet : drone->GetBullets()) {
        if (!bullet->IsActive()) continue;

        const Vector3 bPos = bullet->GetPosition();
        CollectNearbyEnemies(spatialMap, bPos, std::sqrt(kNormalBulletHitDistanceSq), nearbyEnemies);
        for (Enemy* enemy : nearbyEnemies) {
            if (!enemy || !enemy->IsActive()) continue;

            Vector3 ePos = enemy->GetPosition();

            float dx = bPos.x - ePos.x;
            float dz = bPos.z - ePos.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < kNormalBulletHitDistanceSq) {
                if (!bullet->CanHitEnemy(enemy)) continue;

                int32_t droneDamage = playerManager.GetDroneDamage();
                bullet->RegisterHit(enemy);
                TryHandleBulletHit(*enemy, bPos, droneDamage, 0.65f);
                if (!bullet->ConsumeHit()) {
                    break;
                }
            }
        }
    }
}

void EnemyManager::CheckPlayerCollisions(Player& player, PlayerManager& playerManager, const EnemyCellMap& spatialMap) {
    Vector3 playerPosition = player.GetWorldPosition();
    std::vector<Enemy*> nearbyEnemies;
    CollectNearbyEnemies(spatialMap, playerPosition, kPlayerContactDistance, nearbyEnemies);

    for (Enemy* enemy : nearbyEnemies) {
        if (!enemy || !enemy->IsActive()) continue;

        Vector3 ePos = enemy->GetPosition();
        float dx = ePos.x - playerPosition.x;
        float dz = ePos.z - playerPosition.z;
        float distSq = dx * dx + dz * dz;
        if (distSq < kPlayerContactDistanceSq && distSq > 0.0001f) {

            float dist = std::sqrt(distSq);
            float overlap = kPlayerContactDistance - dist;

            float nx = dx / dist;
            float nz = dz / dist;

            ePos.x += nx * overlap;
            ePos.z += nz * overlap;
            enemy->SetPosition(ePos);

            if (!playerManager.IsInvincible()) {
                playerManager.TakeDamage();
                if (audio_) {
                    audio_->PlayWave(playerDamageSEHandle_, false, 0.8f);
                }
            }
        }
    }
}

void EnemyManager::CheckCollisions(Player* player, PlayerManager* playerManager) {
    if (!player || !playerManager) {
        return;
    }

    EnemyCellMap spatialMap;
    std::vector<Enemy*> activeEnemies;
    BuildActiveEnemySpatialMap(spatialMap, activeEnemies);

    CheckNormalBulletCollisions(*playerManager, spatialMap);
    CheckOrbitBulletCollisions(*playerManager, spatialMap);
    CheckDroneBulletCollisions(*playerManager, spatialMap);
    CheckPlayerCollisions(*player, *playerManager, spatialMap);
}

std::vector<Vector3> EnemyManager::PickLightningTargets(int32_t count) const {
    std::vector<Vector3> candidates;
    candidates.reserve(enemies_.size());
    for (const auto& enemy : enemies_) {
        if (enemy && enemy->IsActive()) {
            candidates.push_back(enemy->GetPosition());
        }
    }

    std::vector<Vector3> targets;
    if (candidates.empty() || count <= 0) {
        return targets;
    }

    targets.reserve(count);
    for (int32_t i = 0; i < count && !candidates.empty(); ++i) {
        const size_t pickedIndex = static_cast<size_t>(rand() % static_cast<int>(candidates.size()));
        targets.push_back(candidates[pickedIndex]);
        candidates.erase(candidates.begin() + static_cast<std::ptrdiff_t>(pickedIndex));
    }

    return targets;
}

void EnemyManager::ApplyLightningDamage(const Vector3& center, float radius, int32_t damage) {
    const float radiusSq = radius * radius;
    EnemyCellMap spatialMap;
    std::vector<Enemy*> activeEnemies;
    std::vector<Enemy*> nearbyEnemies;
    BuildActiveEnemySpatialMap(spatialMap, activeEnemies);
    CollectNearbyEnemies(spatialMap, center, radius, nearbyEnemies);

    for (Enemy* enemy : nearbyEnemies) {
        if (!enemy || !enemy->IsActive()) {
            continue;
        }

        const Vector3 enemyPosition = enemy->GetPosition();
        const float dx = enemyPosition.x - center.x;
        const float dz = enemyPosition.z - center.z;
        const float distSq = dx * dx + dz * dz;
        if (distSq > radiusSq) {
            continue;
        }

        TryHandleBulletHit(*enemy, center, damage, 0.9f + damage * 0.1f);
    }
}

void EnemyManager::Draw(Camera* camera) {
    for (auto& enemy : enemies_) {
        if (enemy->IsActive()) enemy->Draw(camera);
    }
    for (auto& orb : expOrbs_) orb->Draw(camera);
    for (auto& p : deathParticles_) p->Draw(camera);
}

void EnemyManager::DrawHitParticles(Camera* camera) {
    for (auto& p : hitParticles_) p->Draw(camera);
}

} // namespace DirectXGame
