#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

class ExpOrb {
public:
    void Initialize(const KamataEngine::Vector3& pos, int32_t expValue);
    void Update(const KamataEngine::Vector3& playerPos, float deltaTime);
    void Draw(KamataEngine::Camera* camera);

    bool IsActive() const { return active_; }
    int32_t GetEXP() const { return expValue_; }
    void Deactivate() { active_ = false; }

    KamataEngine::Vector3 GetPosition() const { return worldTransform_.translation_; }

private:
    KamataEngine::WorldTransform worldTransform_;
    std::shared_ptr<KamataEngine::Model> model_; ///< 経験値オーブ用モデル
    bool active_ = true;
    int32_t expValue_ = 0;

    KamataEngine::Vector3 velocity_{ 0, 0, 0 };

    KamataEngine::Audio* audio_ = nullptr;
    uint32_t pickupSEHandle_ = 0;
};

} // namespace DirectXGame
