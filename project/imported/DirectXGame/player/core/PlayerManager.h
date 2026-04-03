#pragma once

#include <KamataEngine.h>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>

#include "Player.h"
#include "../../effects/LightningStrikeEffect.h"
#include "../../effects/RippleEffect.h"
#include "../weapons/NormalBullet.h"
#include "../weapons/OrbitBullet.h"
#include "../weapons/Drone.h"

namespace DirectXGame {

class EnemyManager;

/// <summary>
/// プレイヤーの成長と戦闘リソースを管理するクラス。
/// HP、EXP、武器、ドローン、無敵時間などのプレイヤー状態を一元管理する。
/// </summary>
class PlayerManager {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: 管理対象プレイヤーを関連付け、表示状態を同期する。
    /// 引数: player - 管理対象のプレイヤー本体
    /// 戻り値: なし
    /// </summary>
    void Initialize(Player* player);

    /// <summary>
    /// CSV 読み込み
    /// 目的: プレイヤー初期パラメータを外部ファイルから反映する。
    /// 引数: filePath - 読み込むCSVファイルパス
    /// 戻り値: なし
    /// </summary>
    void LoadStatusFromCSV(const std::string& filePath);

    /// <summary>
    /// 更新処理
    /// 目的: 無敵時間、弾、ドローン、エフェクトを毎フレーム更新する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Update(float deltaTime);

    /// <summary>
    /// 描画処理
    /// 目的: プレイヤー付随の弾やエフェクトを描画する。
    /// 引数: camera - 描画に使用するカメラ
    /// 戻り値: なし
    /// </summary>
    void Draw(KamataEngine::Camera* camera);

    // HP管理
    /// <summary>
    /// ダメージ処理
    /// 目的: プレイヤーのHPを減少させ、無敵時間を開始する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void TakeDamage();

    /// <summary>
    /// HP回復処理
    /// 目的: HPを1だけ回復し、最大値を超えないよう制限する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void RecoverHP();
    int32_t GetHP() const { return lifeStock_; }
    int32_t GetMaxHP() const { return maxLifeStock_; }
    bool IsInvincible() const { return invincible_; }
    bool IsDead() const { return lifeStock_ <= 0; }

    // EXP / レベル
    /// <summary>
    /// 経験値加算
    /// 目的: EXP を増加させ、必要ならレベルアップ要求を立てる。
    /// 引数: amount - 加算する経験値量
    /// 戻り値: なし
    /// </summary>
    void AddEXP(int32_t amount);
    int32_t GetEXP() const { return exp_; }
    int32_t GetTotalEXP() const { return totalExp_; }
    int32_t GetLevel() const { return level_; }
    int32_t GetNextLevelEXP() const { return nextLevelExp_; }
    bool IsLevelUpRequested() const { return levelUpRequested_; }
    void ClearLevelUpRequest() { levelUpRequested_ = false; }

    // 攻撃力
    int32_t GetAttackPower() const { return attackPower_; }
    void UpgradeAttackPower() { attackPower_++; }

    // 基礎ステータス強化
    void IncreaseMaxHP();
    void UpgradeMoveSpeed();
    int32_t GetMoveSpeedLevel() const { return moveSpeedLevel_; }

    // 通常弾
    /// <summary>
    /// 通常弾強化
    /// 目的: 発射間隔を短縮して通常弾性能を上げる。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void UpgradeNormalBullets();
    const std::vector<std::unique_ptr<NormalBullet>>& GetNormalBullets() const { return normalBullets_; }
    float GetNormalBulletInterval() const { return normalBulletInterval_; }
    int32_t GetNormalBulletLevel() const { return normalBulletLevel_; }
    static constexpr int32_t kNormalBulletMaxLevel = 8;
    bool IsNormalBulletMaxLevel() const { return normalBulletLevel_ >= kNormalBulletMaxLevel; }
    int32_t GetNormalBulletDamage() const { return attackPower_ + normalBulletDamageBonus_; }

    // 周囲弾
    /// <summary>
    /// 周囲弾追加
    /// 目的: 周囲弾を有効化し、初期弾を生成する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void AddOrbitBullets();

    /// <summary>
    /// 周囲弾強化
    /// 目的: 周囲弾の数を増やして攻撃範囲を拡張する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void UpgradeOrbitBullets();
    bool HasOrbitBullets() const { return hasOrbitBullets_; }
    const std::vector<std::unique_ptr<OrbitBullet>>& GetOrbitBullets() const { return orbitBullets_; }
    int32_t GetOrbitBulletLevel() const { return orbitBulletLevel_; }
    static constexpr int32_t kOrbitBulletMaxLevel = 8;
    bool IsOrbitBulletMaxLevel() const { return hasOrbitBullets_ && orbitBulletLevel_ >= kOrbitBulletMaxLevel; }
    int32_t GetOrbitBulletDamage() const { return attackPower_; }

    // ドローン
    /// <summary>
    /// ドローン追加
    /// 目的: ドローンを生成して自動攻撃を有効化する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void AddDrone();

    /// <summary>
    /// ドローン強化
    /// 目的: ドローンの攻撃間隔を短縮する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void UpgradeDrone();
    bool HasDrone() const { return hasDrone_; }
    const std::unique_ptr<Drone>& GetDrone() const { return drone_; }
    int32_t GetDroneLevel() const { return droneLevel_; }
    static constexpr int32_t kDroneMaxLevel = 8;
    bool IsDroneMaxLevel() const { return hasDrone_ && droneLevel_ >= kDroneMaxLevel; }
    int32_t GetDroneDamage() const { return (std::max)(1, attackPower_ / 2 + droneDamageBonus_); }

    // ライトニング
    void AddLightning();
    void UpgradeLightning();
    bool HasLightning() const { return hasLightning_; }
    int32_t GetLightningLevel() const { return lightningLevel_; }
    static constexpr int32_t kLightningMaxLevel = 8;
    bool IsLightningMaxLevel() const { return hasLightning_ && lightningLevel_ >= kLightningMaxLevel; }

    /// <summary>
    /// 敵マネージャ設定
    /// 目的: ドローンの索敵などに使う敵管理参照を保持する。
    /// 引数: manager - 関連付ける敵マネージャ
    /// 戻り値: なし
    /// </summary>
    void SetEnemyManager(EnemyManager* manager) { enemyManager_ = manager; }

    /// <summary>
    /// レベルアップ演出生成
    /// 目的: レベルアップ開始時にプレイヤー周囲へ波紋エフェクトを発生させる。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void PlayLevelUpEffect();

private:
    Player* player_ = nullptr;
    EnemyManager* enemyManager_ = nullptr;

    // HP管理
    bool invincible_ = false;
    float invincibleTimer_ = 0.0f;
    bool visible_ = true;
    float invincibilityDuration_ = 1.25f;

    // EXP / レベル
    int32_t level_ = 1;
    int32_t nextLevelExp_ = 10;
    int32_t maxLifeStock_ = 3;
    int32_t lifeStock_ = 3;
    int32_t exp_ = 0;
    int32_t totalExp_ = 0;
    int32_t attackPower_ = 1;
    int32_t moveSpeedLevel_ = 0;
    bool levelUpRequested_ = false;

    // エフェクト
    std::vector<std::unique_ptr<RippleEffect>> effects_;
    std::vector<std::unique_ptr<LightningStrikeEffect>> lightningEffects_;
    float effectTimer_ = 0.0f;
    static constexpr float kEffectInterval = 0.2f;
    KamataEngine::Vector3 previousEffectPosition_{ 0.0f, 0.0f, 0.0f };
    bool hasPreviousEffectPosition_ = false;

    // 通常弾
    std::vector<std::unique_ptr<NormalBullet>> normalBullets_;
    bool hasNormalBullets_ = true;
    float normalBulletInterval_ = 0.85f;
    float normalBulletTimer_ = 0.0f;
    int32_t normalBulletLevel_ = 1;
    int32_t normalBulletAmount_ = 1;
    int32_t normalBulletDamageBonus_ = 0;
    int32_t normalBulletPierceCount_ = 1;
    float normalBulletSpeed_ = 1.0f;
    float normalBulletRange_ = 30.0f;

    int32_t maxLifeStockCap_ = 6;
    int32_t moveSpeedUpgradeCap_ = 5;
    float moveSpeedUpgradeStep_ = 3.0f;
    float moveSpeedMax_ = 45.0f;
    float normalBulletUpgradeMultiplier_ = 0.84f;
    float normalBulletMinInterval_ = 0.18f;

    // 周囲弾
    std::vector<std::unique_ptr<OrbitBullet>> orbitBullets_;
    bool hasOrbitBullets_ = false;
    int32_t orbitBulletLevel_ = 0;
    int32_t orbitBulletCount_ = 1;
    float orbitRadius_ = 10.0f;
    float orbitRadiusUpgradeStep_ = 2.0f;
    float orbitAngularSpeed_ = 0.03f;
    float orbitAngularSpeedUpgradeStep_ = 0.01f;
    float orbitBulletScale_ = 1.0f;
    float orbitBulletScaleUpgradeStep_ = 0.2f;
    float orbitHitInterval_ = 0.5f;
    float orbitHitIntervalUpgradeMultiplier_ = 0.8f;

    // ドローン
    std::unique_ptr<Drone> drone_;
    bool hasDrone_ = false;
    int32_t droneLevel_ = 0;
    int32_t droneShotCount_ = 1;
    int32_t droneDamageBonus_ = 0;
    int32_t dronePierceCount_ = 1;
    float droneInterval_ = 2.0f;
    float droneTimer_ = 0.0f;
    float droneUpgradeMultiplier_ = 0.8f;
    float droneBulletSpeed_ = 1.0f;
    float droneBulletRange_ = 30.0f;

    // ライトニング
    bool hasLightning_ = false;
    int32_t lightningLevel_ = 0;
    int32_t lightningStrikeCount_ = 1;
    int32_t lightningDamageBonus_ = 0;
    float lightningRadius_ = 6.0f;
    float lightningInterval_ = 2.4f;
    float lightningTimer_ = 0.0f;

    void UpdateInvincibility(float deltaTime);
    void UpdateNormalBullets(float deltaTime);
    void UpdateOrbitBullets(float deltaTime);
    void UpdateDrone(float deltaTime);
    void UpdateLightning(float deltaTime);
    void UpdateEffects(float deltaTime);
    void SpawnRippleEffect(const KamataEngine::Vector3& position);
    void RebuildOrbitBullets();
};

} // namespace DirectXGame
