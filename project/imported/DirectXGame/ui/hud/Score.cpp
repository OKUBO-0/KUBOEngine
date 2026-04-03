#include "Score.h"
#include "../common/DigitSpriteUtil.h"

using namespace KamataEngine;

namespace DirectXGame {

void Score::Initialize() {
    // 数字表示用テクスチャを読み込み
    textureHandle_ = TextureManager::Load("ui/number/numbers.png");
    position_ = { 800.0f, 10.0f };

    // 各桁分のスプライトを生成し、横並びに配置
    for (int32_t i = 0; i < kDigitCount; ++i) {
        sprite_[i] = std::unique_ptr<Sprite>(
            Sprite::Create(textureHandle_, { 0.0f, 0.0f })
        );
        sprite_[i]->SetSize(digitSize_);
        sprite_[i]->SetTextureRect({ 0.0f, 0.0f }, digitSize_);
    }

    UpdateBounds();
    RefreshLayout();
}

void Score::Update() {
    // スコアは外部から更新されるため、ここでは特別な処理は不要
}

void Score::Draw() const {
    if (!visible_) {
        return;
    }

    // 各桁スプライトを順に描画
    for (int32_t i = 0; i < kDigitCount; ++i) {
        if (sprite_[i]) { sprite_[i]->Draw(); }
    }
}

void Score::SetNumber(int32_t number) {
    // 数値を桁ごとに分解してスプライトに反映
    static constexpr int32_t kInitialDigit = 10000; // 5桁対応（10000の位から処理）
    DigitSpriteUtil::SetNumberSprites(sprite_, digitSize_.x, digitSize_, number, kInitialDigit);
}

void Score::SetPosition(const Vector2& pos) {
    UIElement::SetPosition(pos);
}

void Score::SetScale(float scale) {
    // スケール値を更新し、各桁スプライトのサイズと位置を再設定
    scale_ = scale;
    UpdateBounds();
    RefreshLayout();
}

void Score::OnTransformChanged() {
    RefreshLayout();
}

void Score::RefreshLayout() {
    const Vector2 world = GetWorldPosition();
    const Vector2 offset = GetAnchorOffset();
    const Vector2 basePosition = { world.x + offset.x, world.y + offset.y };

    for (int32_t i = 0; i < kDigitCount; ++i) {
        if (!sprite_[i]) {
            continue;
        }
        DigitSpriteUtil::UpdateDigitLayout(*sprite_[i], basePosition, digitSize_, scale_, i);
    }
}

void Score::UpdateBounds() {
    size_ = { digitSize_.x * scale_ * static_cast<float>(kDigitCount), digitSize_.y * scale_ };
}

} // namespace DirectXGame
