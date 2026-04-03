#pragma once

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <unordered_map>
#include <KamataEngine.h>
#include "Enemy.h"
#include "DeathParticle.h"
#include "ExpOrb.h"
#include "HitParticle.h"

namespace DirectXGame {

class Player;
class PlayerManager;

/// <summary>
/// 敵全体の生成と戦闘処理を管理するクラス。
/// 敵の湧き、更新、当たり判定、ドロップ、パーティクルをまとめて制御する。
/// </summary>
class EnemyManager {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: 敵定義、参照先、効果音を読み込んで管理状態を整える。
    /// 引数: csvPath - 敵定義CSVのパス
    /// 引数: player - 追跡対象プレイヤー
    /// 引数: playerManager - ダメージやEXP付与に使うプレイヤー管理
    /// 戻り値: なし
    /// </summary>
    void Initialize(const std::string& csvPath, Player* player, PlayerManager* playerManager);

    /// <summary>
    /// 更新処理
    /// 目的: 敵の生成、更新、再配置、演出更新を毎フレーム進める。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// 描画処理
    /// 目的: 敵本体、EXPオーブ、死亡パーティクルを描画する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void Draw(KamataEngine::Camera* camera);

    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return enemies_; }

    /// <summary>
    /// 敵定義読み込み
    /// 目的: CSV から敵の基礎ステータスと出現数を取得する。
    /// 引数: filePath - 読み込むCSVファイルパス
    /// 戻り値: なし
    /// </summary>
    void LoadEnemyTypes(const std::string& filePath);

    /// <summary>
    /// 当たり判定処理
    /// 目的: 弾、敵、プレイヤーの衝突を判定してダメージ処理を行う。
    /// 引数: player - 接触判定を行うプレイヤー
    /// 引数: playerManager - 攻撃力や被ダメージ処理を扱う管理クラス
    /// 戻り値: なし
    /// </summary>
    void CheckCollisions(Player* player, PlayerManager* playerManager);

    /// <summary>
    /// ヒットパーティクル描画
    /// 目的: ヒット時に発生した火花演出だけを別パスで描画する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void DrawHitParticles(KamataEngine::Camera* camera);

    const std::list<std::unique_ptr<ExpOrb>>& GetExpOrbs() const { return expOrbs_; }
    int32_t GetTotalKillCount() const { return totalKillCount_; }
    std::vector<KamataEngine::Vector3> PickLightningTargets(int32_t count) const;
    void ApplyLightningDamage(const KamataEngine::Vector3& center, float radius, int32_t damage);

private:
    using EnemyCellMap = std::unordered_map<int64_t, std::vector<Enemy*>>;

    static constexpr size_t kMaxActiveEnemies = 84;
    static constexpr size_t kMaxDeathParticles = 256;
    static constexpr size_t kMaxHitParticles = 256;
    static constexpr float kSpawnUnlockInterval = 18.0f;
    static constexpr float kSpawnDistance = 50.0f;
    static constexpr float kRespawnDistance = 75.0f;
    static constexpr float kRespawnRadius = 60.0f;
    static constexpr float kMinSpawnInterval = 0.85f;
    static constexpr float kBaseSpawnInterval = 1.9f;
    static constexpr float kSpawnAcceleration = 0.0075f;
    static constexpr float kEnemySeparationDistance = 3.2f;
    static constexpr float kEnemySeparationStrength = 1.1f;
    static constexpr float kSpatialCellSize = 8.0f;
    static constexpr float kNormalBulletHitDistanceSq = 4.0f;
    static constexpr float kOrbitBulletHitDistanceSq = 25.0f;
    static constexpr float kPlayerContactDistance = 2.5f;
    static constexpr float kPlayerContactDistanceSq = kPlayerContactDistance * kPlayerContactDistance;
    static constexpr int32_t kDeathParticleSpawnCount = 5;
    static constexpr int32_t kHitParticleSpawnCount = 4;

    struct EnemyTypeData {
        int32_t type;
        int32_t baseHP;
        float   baseSpeed;
        int32_t baseEXP;
        int32_t spawnCount;
    };

    void SpawnEnemies();                     // 無限湧き
    void SpawnOneEnemy(const EnemyTypeData& data);
    size_t GetActiveEnemyCount() const;
    void UpdateSpawnState(float deltaTime);
    void UpdateEnemies(float deltaTime);
    void RemoveInactiveEnemies();
    void RelocateFarEnemies();
    void UpdateEffects(float deltaTime);
    void ResolveEnemySeparation();
    static int32_t ToCellCoord(float value);
    static int64_t MakeCellKey(int32_t cellX, int32_t cellZ);
    void BuildActiveEnemySpatialMap(EnemyCellMap& outMap, std::vector<Enemy*>& activeEnemies) const;
    void CollectNearbyEnemies(const EnemyCellMap& spatialMap, const KamataEngine::Vector3& center, float radius,
                              std::vector<Enemy*>& outEnemies) const;
    void SpawnDeathEffects(const Enemy& enemy);
    void SpawnHitParticles(const KamataEngine::Vector3& position);
    bool TryHandleBulletHit(Enemy& enemy, const KamataEngine::Vector3& impactPosition,
                            int32_t damage, float knockStrength);
    void CheckNormalBulletCollisions(PlayerManager& playerManager, const EnemyCellMap& spatialMap);
    void CheckOrbitBulletCollisions(PlayerManager& playerManager, const EnemyCellMap& spatialMap);
    void CheckDroneBulletCollisions(PlayerManager& playerManager, const EnemyCellMap& spatialMap);
    void CheckPlayerCollisions(Player& player, PlayerManager& playerManager, const EnemyCellMap& spatialMap);

private:
    std::vector<std::unique_ptr<Enemy>> enemies_;
    Player* player_ = nullptr;
    PlayerManager* playerManager_ = nullptr;

    std::list<std::unique_ptr<DeathParticle>> deathParticles_;
    std::list<std::unique_ptr<ExpOrb>> expOrbs_;
    std::list<std::unique_ptr<HitParticle>> hitParticles_;

    KamataEngine::Audio* audio_ = nullptr;
    uint32_t hitSEHandle_ = 0;
    uint32_t playerDamageSEHandle_ = 0;

    // 敵タイプ定義
    std::vector<EnemyTypeData> enemyTypes_;

    // 無限湧き用
    float elapsedTime_ = 0.0f;
    float spawnTimer_ = 0.0f;
    float spawnInterval_ = kBaseSpawnInterval;
    int32_t totalKillCount_ = 0;
};

} // namespace DirectXGame
