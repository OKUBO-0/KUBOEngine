#pragma once
#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

class Bullet {
public:
    virtual ~Bullet() = default;

    virtual void Initialize(const KamataEngine::Vector3& startPos) {
        worldTransform_.Initialize();
        worldTransform_.translation_ = startPos;
        active_ = true;
    }

    virtual void Update(const KamataEngine::Vector3& playerPos, float deltaTime) = 0;

    virtual void Draw(KamataEngine::Camera* camera) {
        if (active_ && model_) {
            model_->Draw(worldTransform_, *camera);
        }
    }

    bool IsActive() const { return active_; }

protected:
    KamataEngine::WorldTransform worldTransform_;
    std::shared_ptr<KamataEngine::Model> model_;
    bool active_ = false;
};

} // namespace DirectXGame
