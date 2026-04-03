#pragma once

#include <KamataEngine.h>
#include <memory>

namespace DirectXGame {

class PlayerManager;

class Player {
public:
    void Initialize();
    void Update(float deltaTime);
    void Draw();

    // 基本情報
    KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
    float GetWorldRotationY() const { return worldTransform_.rotation_.y; }
    KamataEngine::Camera& GetCamera() { return camera_; }
    float GetMoveSpeed() const { return moveSpeedPerSecond_; }
    void SetMoveSpeed(float moveSpeedPerSecond) { moveSpeedPerSecond_ = moveSpeedPerSecond; }
    void SetLightGroup(const KamataEngine::LightGroup* lightGroup);

    // 可視制御（外部から点滅制御するためのセッター）
    void SetVisible(bool visible) { visible_ = visible; }

private:
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::Camera camera_;
    std::shared_ptr<KamataEngine::Model> playerModel_;

    bool visible_ = true;
    float effectTimer_ = 0.0f;
    static constexpr float kEffectInterval = 0.2f;
    float moveSpeedPerSecond_ = 30.0f;

    void UpdateMovement(float deltaTime);
    void UpdateAim(float deltaTime);
    void UpdateCamera();
};

} // namespace DirectXGame
