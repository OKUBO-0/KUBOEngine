#include "LightningStrikeEffect.h"
#include "../core/ModelCache.h"
#include <algorithm>

using namespace KamataEngine;

namespace DirectXGame {

namespace {

constexpr float kWarningDuration = 0.20f;
constexpr float kStrikeDuration = 0.08f;
constexpr float kFadeDuration = 0.12f;

}

void LightningStrikeEffect::Initialize(const Vector3& position, float radius) {
    warningModel_ = ModelCache::Get("ripples");
    strikeModel_ = ModelCache::Get("cube");

    warningColor_ = std::make_unique<ObjectColor>();
    warningColor_->Initialize();
    warningColor_->SetColor({ 1.0f, 0.92f, 0.35f, 0.85f });

    strikeColor_ = std::make_unique<ObjectColor>();
    strikeColor_->Initialize();
    strikeColor_->SetColor({ 0.85f, 0.95f, 1.0f, 1.0f });

    warningTransform_.Initialize();
    warningTransform_.translation_ = position;
    warningBaseScale_ = radius * 0.18f;
    warningTransform_.scale_ = { warningBaseScale_, warningBaseScale_, warningBaseScale_ };
    warningTransform_.UpdateMatrix();

    strikeTransform_.Initialize();
    strikeTransform_.translation_ = { position.x, position.y + 5.0f, position.z };
    strikeTransform_.scale_ = { 0.45f, 8.0f, 0.45f };
    strikeTransform_.UpdateMatrix();

    phase_ = Phase::Warning;
    timer_ = 0.0f;
    active_ = true;
}

void LightningStrikeEffect::Update(float deltaTime) {
    if (!active_) {
        return;
    }

    timer_ += deltaTime;

    if (phase_ == Phase::Warning) {
        const float t = (std::min)(1.0f, timer_ / kWarningDuration);
        const float scale = 0.55f + t * 0.55f;
        const float actualScale = warningBaseScale_ * scale;
        warningTransform_.scale_ = { actualScale, actualScale, actualScale };
        warningTransform_.UpdateMatrix();
        if (warningColor_) {
            warningColor_->SetColor({ 1.0f, 0.92f, 0.35f, 0.45f + 0.4f * (1.0f - t) });
        }
        if (timer_ >= kWarningDuration) {
            phase_ = Phase::Strike;
            timer_ = 0.0f;
        }
        return;
    }

    if (phase_ == Phase::Strike) {
        if (timer_ >= kStrikeDuration) {
            phase_ = Phase::Fade;
            timer_ = 0.0f;
        }
        return;
    }

    const float t = (std::min)(1.0f, timer_ / kFadeDuration);
    if (strikeColor_) {
        strikeColor_->SetColor({ 0.85f, 0.95f, 1.0f, 1.0f - t });
    }
    if (warningColor_) {
        warningColor_->SetColor({ 1.0f, 0.92f, 0.35f, 0.25f * (1.0f - t) });
    }
    if (timer_ >= kFadeDuration) {
        active_ = false;
    }
}

void LightningStrikeEffect::Draw(Camera* camera) {
    if (!active_) {
        return;
    }

    if ((phase_ == Phase::Warning || phase_ == Phase::Fade) && warningModel_) {
        warningModel_->Draw(warningTransform_, *camera, warningColor_.get());
    }
    if ((phase_ == Phase::Strike || phase_ == Phase::Fade) && strikeModel_) {
        strikeModel_->Draw(strikeTransform_, *camera, strikeColor_.get());
    }
}

} // namespace DirectXGame
