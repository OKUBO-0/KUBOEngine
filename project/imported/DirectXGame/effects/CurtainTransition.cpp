#include "CurtainTransition.h"
#include <algorithm>
using namespace KamataEngine;

namespace DirectXGame {

namespace {

constexpr float kFrameDeltaBaseline = 0.016f;
constexpr float kMaxTransitionDelta = 0.033f;
constexpr float kCurtainHalfHeight = 360.0f;
constexpr float kCurtainOverlap = 24.0f;
constexpr float kTopOpenY = -(kCurtainHalfHeight + kCurtainOverlap);
constexpr float kBottomClosedY = kCurtainHalfHeight - kCurtainOverlap;
constexpr float kBottomOpenY = 720.0f;

}

void CurtainTransition::Initialize() {
    uint32_t tex = TextureManager::Load("textures/color/black.png");

    topCurtain_.reset(Sprite::Create(tex, { 0, kTopOpenY }));
    bottomCurtain_.reset(Sprite::Create(tex, { 0, kBottomOpenY }));

    topCurtain_->SetSize({ 1280, kCurtainHalfHeight + kCurtainOverlap });
    bottomCurtain_->SetSize({ 1280, kCurtainHalfHeight + kCurtainOverlap });

    state_ = State::kNone;
}

void CurtainTransition::StartClose(float speed) {
    speed_ = speed;
    state_ = State::kClose;

    topCurtain_->SetPosition({ 0, kTopOpenY });
    bottomCurtain_->SetPosition({ 0, kBottomOpenY });
}

void CurtainTransition::StartOpen(float speed) {
    speed_ = speed;
    state_ = State::kOpen;

    topCurtain_->SetPosition({ 0, 0 });
    bottomCurtain_->SetPosition({ 0, kBottomClosedY });
}

void CurtainTransition::Update(float deltaTime) {
    const float clampedDeltaTime = (std::min)(deltaTime, kMaxTransitionDelta);
    const float deltaScale = clampedDeltaTime / kFrameDeltaBaseline;

    if (state_ == State::kClose) {
        auto posTop = topCurtain_->GetPosition();
        auto posBottom = bottomCurtain_->GetPosition();

        posTop.y += speed_ * deltaScale;
        posBottom.y -= speed_ * deltaScale;

        topCurtain_->SetPosition(posTop);
        bottomCurtain_->SetPosition(posBottom);

        if (posTop.y >= 0) {
            topCurtain_->SetPosition({ 0, 0 });
            bottomCurtain_->SetPosition({ 0, kBottomClosedY });
            state_ = State::kFinished;
        }
    }
    else if (state_ == State::kOpen) {
        auto posTop = topCurtain_->GetPosition();
        auto posBottom = bottomCurtain_->GetPosition();

        posTop.y -= speed_ * deltaScale;
        posBottom.y += speed_ * deltaScale;

        topCurtain_->SetPosition(posTop);
        bottomCurtain_->SetPosition(posBottom);

        if (posTop.y <= kTopOpenY) {
            topCurtain_->SetPosition({ 0, kTopOpenY });
            bottomCurtain_->SetPosition({ 0, kBottomOpenY });
            state_ = State::kNone;
        }
    }
}

void CurtainTransition::Draw() {
    if (state_ == State::kNone) return;

    topCurtain_->Draw();
    bottomCurtain_->Draw();
}

} // namespace DirectXGame
