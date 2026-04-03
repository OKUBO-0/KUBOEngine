#include "HpGauge.h"
#include "../common/UILayoutIO.h"
using namespace KamataEngine;

namespace DirectXGame {

namespace {

const char* kHudLayoutPath = "Resources/data/ui_layout_hud.csv";

int32_t StepDisplayValue(int32_t displayedValue, int32_t targetValue) {
    if (displayedValue < targetValue) {
        displayedValue += std::max<int32_t>(1, (targetValue - displayedValue) / 10);
    } else if (displayedValue > targetValue) {
        displayedValue -= std::max<int32_t>(1, (displayedValue - targetValue) / 10);
    }

    return displayedValue;
}

float CalculateGaugeWidth(int32_t displayedValue, int32_t maxValue, float maxGaugeWidth) {
    float ratio = static_cast<float>(displayedValue) / static_cast<float>(maxValue);
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    return maxGaugeWidth * ratio;
}

} // namespace

void HpGauge::Initialize() {
    const auto layout = UILayoutIO::Load(kHudLayoutPath);
    if (const auto it = layout.find("hpPosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.position = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("hpSize"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.size = { it->second[0], it->second[1] };
    }

    gauge_.Initialize();
    gauge_.SetColors({ 0.0f, 0.0f, 0.0f, 0.85f }, { 1.0f, 0.0f, 0.0f, 0.95f });
    gauge_.SetRate(0.0f);
    ApplyLayout();
}

void HpGauge::SetHP(int32_t current, int32_t max) {
    // 現在HPと最大HPを設定（maxは最低1にして0除算防止）
    targetHP_ = current;
    maxHP_ = std::max<int32_t>(1, max);
}

void HpGauge::Update() {
    // 表示HPを滑らかに変化させる（目標値に徐々に近づける）
    displayedHP_ = StepDisplayValue(displayedHP_, targetHP_);

    // HP比率を計算し、赤ゲージの幅に反映
    gauge_.SetRate(CalculateGaugeWidth(displayedHP_, maxHP_, 1.0f));
}

void HpGauge::Draw() {
    gauge_.Draw();
}

bool HpGauge::IsDepleted() const {
    // HPが0以下になったかどうかを判定
    return displayedHP_ <= 0;
}

void HpGauge::DebugDrawImGui() {
#ifdef _DEBUG
    if (!ImGui::CollapsingHeader("HUD HP", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    ImGui::Checkbox("Enable HUD Debug##HP", &layoutSettings_.debugEnabled);
    if (!layoutSettings_.debugEnabled) {
        return;
    }

    float position[2]{ layoutSettings_.position.x, layoutSettings_.position.y };
    if (ImGui::DragFloat2("HP Position", position, 1.0f, -400.0f, 1280.0f)) {
        layoutSettings_.position = { position[0], position[1] };
        ApplyLayout();
    }

    float size[2]{ layoutSettings_.size.x, layoutSettings_.size.y };
    if (ImGui::DragFloat2("HP Size", size, 1.0f, 4.0f, 512.0f)) {
        layoutSettings_.size = { size[0], size[1] };
        ApplyLayout();
    }

    if (ImGui::Button("Save HP Layout")) {
        SaveLayout();
    }
#endif
}

void HpGauge::ApplyLayout() {
    gauge_.SetPosition(layoutSettings_.position);
    gauge_.SetSize(layoutSettings_.size);
}

void HpGauge::SaveLayout() const {
    UILayoutIO::Save(kHudLayoutPath, {
        { "hpPosition", { layoutSettings_.position.x, layoutSettings_.position.y } },
        { "hpSize", { layoutSettings_.size.x, layoutSettings_.size.y } },
    });
}

} // namespace DirectXGame
