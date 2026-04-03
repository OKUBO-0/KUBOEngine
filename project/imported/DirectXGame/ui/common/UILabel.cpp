#include "UILabel.h"

using namespace KamataEngine;

namespace DirectXGame {

void UILabel::Initialize(uint32_t textureHandle, const Vector2& position) {
    sprite_ = std::unique_ptr<Sprite>(Sprite::Create(textureHandle, position));
    SetPosition(position);
    OnTransformChanged();
    OnVisualChanged();
}

void UILabel::SetColor(const Vector4& color) {
    color_ = color;
    OnVisualChanged();
}

void UILabel::Draw() const {
    if (visible_ && sprite_) {
        sprite_->Draw();
    }
    DrawChildren();
}

void UILabel::OnTransformChanged() {
    if (!sprite_) {
        return;
    }

    const Vector2 world = GetWorldPosition();
    const Vector2 offset = GetAnchorOffset();
    sprite_->SetPosition({ world.x + offset.x, world.y + offset.y });
    sprite_->SetSize(GetScaledSize());
}

void UILabel::OnVisualChanged() {
    if (!sprite_) {
        return;
    }

    Vector4 appliedColor = color_;
    appliedColor.w *= GetWorldAlpha();
    sprite_->SetColor(appliedColor);
}

} // namespace DirectXGame
