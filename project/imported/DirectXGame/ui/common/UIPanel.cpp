#include "UIPanel.h"

using namespace KamataEngine;

namespace DirectXGame {

void UIPanel::Initialize() {
    const uint32_t whiteTexture = TextureManager::Load("textures/debug/white1x1.png");
    sprite_ = std::unique_ptr<Sprite>(Sprite::Create(whiteTexture, { 0.0f, 0.0f }));
    sprite_->SetColor(color_);
    OnTransformChanged();
    OnVisualChanged();
}

void UIPanel::SetColor(const Vector4& color) {
    color_ = color;
    OnVisualChanged();
}

void UIPanel::Draw() const {
    if (visible_ && sprite_) {
        sprite_->Draw();
    }
    DrawChildren();
}

void UIPanel::OnTransformChanged() {
    if (!sprite_) {
        return;
    }

    const Vector2 world = GetWorldPosition();
    const Vector2 offset = GetAnchorOffset();
    sprite_->SetPosition({ world.x + offset.x, world.y + offset.y });
    sprite_->SetSize(GetScaledSize());
}

void UIPanel::OnVisualChanged() {
    if (!sprite_) {
        return;
    }

    Vector4 appliedColor = color_;
    appliedColor.w *= GetWorldAlpha();
    sprite_->SetColor(appliedColor);
}

} // namespace DirectXGame
