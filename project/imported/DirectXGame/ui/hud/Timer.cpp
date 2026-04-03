#include "Timer.h"
#include "../common/DigitSpriteUtil.h"
#include "../common/UILayoutIO.h"
#include <algorithm>
using namespace KamataEngine;

namespace DirectXGame {

namespace {

const char* kHudLayoutPath = "Resources/data/ui_layout_hud.csv";

}

void Timer::Initialize() {

    numberTexture_ = TextureManager::Load("ui/number/numbers.png");
    colonTexture_ = TextureManager::Load("ui/number/colon.png");
    const auto layout = UILayoutIO::Load(kHudLayoutPath);
    if (const auto it = layout.find("timerPosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.position = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("timerScale"); it != layout.end() && !it->second.empty()) {
        layoutSettings_.scale = it->second[0];
    }

    // M1 M2 : S1 S2 の 5 つを生成
    for (int i = 0; i < kDigitCount; i++) {
        sprite_[i] = std::unique_ptr<Sprite>(
            Sprite::Create(numberTexture_, { 0.0f, 0.0f })
        );
        sprite_[i]->SetSize(digitSize_);
        sprite_[i]->SetTextureRect({ 0.0f, 0.0f }, digitSize_);
    }

    // コロンは中央に配置（digit 2 の位置）
    colonSprite_ = std::unique_ptr<Sprite>(
        Sprite::Create(colonTexture_, { 0.0f, 0.0f })
    );
    colonSprite_->SetSize(digitSize_);

    UpdateBounds();
    ApplyLayout();
}

void Timer::Update(float dt) {
    time_ += dt;
    UpdateDisplay();
}

void Timer::Draw() const {
    if (!visible_) {
        return;
    }

    // M1 M2 S1 S2
    for (int i = 0; i < kDigitCount; i++) {
        if (i == 2) continue; // 2番目はコロンの位置
        sprite_[i]->Draw();
    }

    // コロン
    colonSprite_->Draw();
}

void Timer::UpdateDisplay() {

    int totalSec = static_cast<int>(time_);
    int minutes = totalSec / 60;
    int seconds = totalSec % 60;

    int m1 = minutes / 10;
    int m2 = minutes % 10;
    int s1 = seconds / 10;
    int s2 = seconds % 10;

    // Score と同じ：SetTextureRect({ left, top }, size_)
    DigitSpriteUtil::SetDigitSprite(*sprite_[0], digitSize_.x, digitSize_, m1);
    DigitSpriteUtil::SetDigitSprite(*sprite_[1], digitSize_.x, digitSize_, m2);
    DigitSpriteUtil::SetDigitSprite(*sprite_[3], digitSize_.x, digitSize_, s1);
    DigitSpriteUtil::SetDigitSprite(*sprite_[4], digitSize_.x, digitSize_, s2);
}

void Timer::SetPosition(const Vector2& pos) {
    UIElement::SetPosition(pos);
}

void Timer::SetScale(float scale) {
    scale_ = scale;
    UpdateBounds();
    RefreshLayout();
}

void Timer::OnTransformChanged() {
    RefreshLayout();
}

void Timer::RefreshLayout() {
    const Vector2 world = GetWorldPosition();
    const Vector2 offset = GetAnchorOffset();
    const Vector2 basePosition = { world.x + offset.x, world.y + offset.y };

    for (int i = 0; i < kDigitCount; i++) {
        if (!sprite_[i]) {
            continue;
        }
        DigitSpriteUtil::UpdateDigitLayout(*sprite_[i], basePosition, digitSize_, scale_, i);
    }

    if (colonSprite_) {
        DigitSpriteUtil::UpdateDigitLayout(*colonSprite_, basePosition, digitSize_, scale_, 2);
    }
}

void Timer::UpdateBounds() {
    size_ = { digitSize_.x * scale_ * static_cast<float>(kDigitCount), digitSize_.y * scale_ };
}

void Timer::SetTime(float time) {
    time_ = (std::max)(0.0f, time);
    UpdateDisplay();
}

void Timer::DebugDrawImGui() {
#ifdef _DEBUG
    if (!ImGui::CollapsingHeader("HUD Timer", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    ImGui::Checkbox("Enable HUD Debug##Timer", &layoutSettings_.debugEnabled);
    if (!layoutSettings_.debugEnabled) {
        return;
    }

    float position[2]{ layoutSettings_.position.x, layoutSettings_.position.y };
    if (ImGui::DragFloat2("Timer Position", position, 1.0f, -400.0f, 1280.0f)) {
        layoutSettings_.position = { position[0], position[1] };
        ApplyLayout();
    }

    if (ImGui::DragFloat("Timer Scale", &layoutSettings_.scale, 0.05f, 0.5f, 6.0f)) {
        ApplyLayout();
    }

    if (ImGui::Button("Save Timer Layout")) {
        SaveLayout();
    }
#endif
}

void Timer::ApplyLayout() {
    position_ = layoutSettings_.position;
    scale_ = layoutSettings_.scale;
    UpdateBounds();
    RefreshLayout();
}

void Timer::SaveLayout() const {
    UILayoutIO::Save(kHudLayoutPath, {
        { "timerPosition", { layoutSettings_.position.x, layoutSettings_.position.y } },
        { "timerScale", { layoutSettings_.scale } },
    });
}

} // namespace DirectXGame
