#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

class LightningStrikeEffect {
public:
    void Initialize(const KamataEngine::Vector3& position, float radius);
    void Update(float deltaTime);
    void Draw(KamataEngine::Camera* camera);
    bool IsActive() const { return active_; }

private:
    enum class Phase {
        Warning,
        Strike,
        Fade,
    };

    KamataEngine::WorldTransform warningTransform_;
    KamataEngine::WorldTransform strikeTransform_;
    std::shared_ptr<KamataEngine::Model> warningModel_;
    std::shared_ptr<KamataEngine::Model> strikeModel_;
    std::unique_ptr<KamataEngine::ObjectColor> warningColor_;
    std::unique_ptr<KamataEngine::ObjectColor> strikeColor_;
    Phase phase_ = Phase::Warning;
    float warningBaseScale_ = 1.0f;
    float timer_ = 0.0f;
    bool active_ = false;
};

} // namespace DirectXGame
