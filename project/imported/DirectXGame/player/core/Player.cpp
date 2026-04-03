#include "Player.h"
#include "../../core/InputBindings.h"
#include "../../core/ModelCache.h"
#include "../../core/ScreenUtil.h"
#include <math/MathUtility.h>
#include <algorithm>
using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace DirectXGame {

namespace {

constexpr float kPi = 3.14159265f;
float NormalizeAngle(float angle) {
    while (angle > kPi) angle -= 2.0f * kPi;
    while (angle < -kPi) angle += 2.0f * kPi;
    return angle;
}

}

void Player::Initialize() {
    input_ = Input::GetInstance();
    camera_.Initialize();

    playerModel_ = ModelCache::Get("octopus");

    worldTransform_.Initialize();
    worldTransform_.translation_ = { 0.0f, 0.0f, 0.0f };

    camera_.translation_ = { 0.0f, 80.0f, -45.0f };
    camera_.rotation_.x = 1.0f;
    camera_.UpdateMatrix();
}

void Player::Update(float deltaTime) {
    UpdateMovement(deltaTime);
    UpdateCamera();
    UpdateAim(deltaTime);
    worldTransform_.UpdateMatrix();
}

void Player::Draw() {
    if (visible_ && playerModel_) {
        playerModel_->Draw(worldTransform_, camera_);
    }
}

void Player::SetLightGroup(const LightGroup* lightGroup) {
    if (playerModel_) {
        playerModel_->SetLightGroup(lightGroup);
    }
}

void Player::UpdateMovement(float deltaTime) {
    const float movePerFrame = moveSpeedPerSecond_ * deltaTime;

    const Vector2 moveInput = InputBindings::GetMoveVector(input_);
    Vector3 move = { moveInput.x * movePerFrame, 0.0f, moveInput.y * movePerFrame };
    if (std::abs(move.x) > 0.0f || std::abs(move.z) > 0.0f) {
        worldTransform_.translation_.x += move.x;
        worldTransform_.translation_.z += move.z;
    }
}

void Player::UpdateAim(float deltaTime) {
    Vector2 padAim{};
    if (InputBindings::GetAimVector(input_, padAim)) {
        const float targetAngle = std::atan2(padAim.x, padAim.y);
        float currentAngle = worldTransform_.rotation_.y;
        float diff = NormalizeAngle(targetAngle - currentAngle);
        const float kRotateLerp = std::clamp(deltaTime * 30.0f, 0.0f, 1.0f);
        currentAngle = NormalizeAngle(currentAngle + diff * kRotateLerp);
        worldTransform_.rotation_.y = currentAngle;
        return;
    }

    const Vector2 clientSize = ScreenUtil::GetClientSize();
    if (clientSize.x <= 0.0f || clientSize.y <= 0.0f) {
        return;
    }

    const Vector2 mousePosition = input_->GetMousePosition();
    const float ndcX = (mousePosition.x / clientSize.x) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (mousePosition.y / clientSize.y) * 2.0f;

    const Matrix4x4 viewProjection = camera_.matView * camera_.matProjection;
    const Matrix4x4 inverseViewProjection = Inverse(viewProjection);

    const Vector3 nearPointNdc = {ndcX, ndcY, 0.0f};
    const Vector3 farPointNdc = {ndcX, ndcY, 1.0f};
    const Vector3 nearPointWorld = TransformCoord(nearPointNdc, inverseViewProjection);
    const Vector3 farPointWorld = TransformCoord(farPointNdc, inverseViewProjection);

    Vector3 rayDirection = farPointWorld - nearPointWorld;
    const float rayLength = Length(rayDirection);
    if (rayLength <= 0.0001f) {
        return;
    }
    rayDirection /= rayLength;

    const float planeY = worldTransform_.translation_.y;
    if (std::abs(rayDirection.y) <= 0.0001f) {
        return;
    }

    const float t = (planeY - nearPointWorld.y) / rayDirection.y;
    if (t <= 0.0f) {
        return;
    }

    const Vector3 targetPoint = nearPointWorld + rayDirection * t;
    Vector3 aimDirection = targetPoint - worldTransform_.translation_;
    aimDirection.y = 0.0f;

    const float aimLength = Length(aimDirection);
    if (aimLength <= 0.0001f) {
        return;
    }

    const float targetAngle = std::atan2(aimDirection.x, aimDirection.z);
    float currentAngle = worldTransform_.rotation_.y;
    float diff = NormalizeAngle(targetAngle - currentAngle);
    const float kRotateLerp = std::clamp(deltaTime * 30.0f, 0.0f, 1.0f);
    currentAngle = NormalizeAngle(currentAngle + diff * kRotateLerp);
    worldTransform_.rotation_.y = currentAngle;
}

void Player::UpdateCamera() {
    camera_.translation_.x = worldTransform_.translation_.x;
    camera_.translation_.z = worldTransform_.translation_.z - 45.0f;
    camera_.rotation_.x = 1.0f;
    camera_.UpdateMatrix();
}

} // namespace DirectXGame
