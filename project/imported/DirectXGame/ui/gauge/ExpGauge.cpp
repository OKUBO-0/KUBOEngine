#include "ExpGauge.h"
#include "../common/DigitSpriteUtil.h"
#include "../common/UILayoutIO.h"
#include <algorithm>
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

void ExpGauge::Initialize() {
    lvLabelHandle_ = TextureManager::Load("ui/game/lv_label.png");
    lvDigitsHandle_ = TextureManager::Load("ui/number/numbers.png");

    const auto layout = UILayoutIO::Load(kHudLayoutPath);
    if (const auto it = layout.find("expFramePosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.framePosition = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("expFrameSize"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.frameSize = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("expGaugePosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.gaugePosition = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("expGaugeSize"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.gaugeSize = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("lvLabelPosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.lvLabelPosition = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("lvLabelSize"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.lvLabelSize = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("lvDigitsPosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.lvDigitsPosition = { it->second[0], it->second[1] };
    }

    frameBar_.Initialize();
    frameBar_.SetColors({ 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f });
    frameBar_.SetRate(1.0f);

    gaugeBar_.Initialize();
    gaugeBar_.SetColors({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });
    gaugeBar_.SetRate(0.0f);

    // LVラベル（「LV」文字画像）
    lvLabel_.Initialize(lvLabelHandle_, layoutSettings_.lvLabelPosition);
    lvLabel_.SetSize(layoutSettings_.lvLabelSize);

    // レベル数字（2桁分を用意）
    for (int32_t i = 0; i < kLvDigits; ++i) {
        sprite_[i] = std::unique_ptr<Sprite>(Sprite::Create(lvDigitsHandle_, { 0.0f, 0.0f }));
        sprite_[i]->SetSize(size_);
        sprite_[i]->SetTextureRect({ 0.0f, 0.0f }, size_);
    }

    ApplyLayout();
}

void ExpGauge::SetEXP(int32_t current, int32_t max) {
    // 現在EXPと最大EXPを設定（maxは最低1にして0除算防止）
    targetExp_ = current;
    maxExp_ = std::max<int32_t>(1, max);
}

void ExpGauge::SetLevel(int32_t level) {
    // レベル数値を桁ごとに分解してスプライトに反映
    DigitSpriteUtil::SetNumberSprites(sprite_, size_.x, size_, level, 10);
}

void ExpGauge::Update() {
    // EXP表示を滑らかに変化させる（目標値に徐々に近づける）
    displayedExp_ = StepDisplayValue(displayedExp_, targetExp_);

    // EXP比率を計算してゲージ幅に反映
    gaugeBar_.SetRate(CalculateGaugeWidth(displayedExp_, maxExp_, 1.0f));
}

void ExpGauge::Draw() {
    // 各スプライトを描画（順序は背景→ゲージ→ラベル→数字）
    frameBar_.Draw();
    gaugeBar_.Draw();
    lvLabel_.Draw();
    for (int32_t i = 0; i < kLvDigits; ++i) {
        if (sprite_[i]) { sprite_[i]->Draw(); }
    }
}

bool ExpGauge::IsFilled() const {
    // 現在EXPが最大値に到達しているか判定
    return displayedExp_ >= maxExp_;
}

void ExpGauge::DebugDrawImGui() {
#ifdef _DEBUG
    if (!ImGui::CollapsingHeader("HUD EXP", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    ImGui::Checkbox("Enable HUD Debug##EXP", &layoutSettings_.debugEnabled);
    if (!layoutSettings_.debugEnabled) {
        return;
    }

    float framePosition[2]{ layoutSettings_.framePosition.x, layoutSettings_.framePosition.y };
    if (ImGui::DragFloat2("EXP Frame Pos", framePosition, 1.0f, -400.0f, 1280.0f)) {
        layoutSettings_.framePosition = { framePosition[0], framePosition[1] };
        ApplyLayout();
    }

    float frameSize[2]{ layoutSettings_.frameSize.x, layoutSettings_.frameSize.y };
    if (ImGui::DragFloat2("EXP Frame Size", frameSize, 1.0f, 16.0f, 1600.0f)) {
        layoutSettings_.frameSize = { frameSize[0], frameSize[1] };
        ApplyLayout();
    }

    float gaugePosition[2]{ layoutSettings_.gaugePosition.x, layoutSettings_.gaugePosition.y };
    if (ImGui::DragFloat2("EXP Gauge Pos", gaugePosition, 1.0f, -400.0f, 1280.0f)) {
        layoutSettings_.gaugePosition = { gaugePosition[0], gaugePosition[1] };
        ApplyLayout();
    }

    float gaugeSize[2]{ layoutSettings_.gaugeSize.x, layoutSettings_.gaugeSize.y };
    if (ImGui::DragFloat2("EXP Gauge Size", gaugeSize, 1.0f, 16.0f, 1600.0f)) {
        layoutSettings_.gaugeSize = { gaugeSize[0], gaugeSize[1] };
        ApplyLayout();
    }

    float labelPosition[2]{ layoutSettings_.lvLabelPosition.x, layoutSettings_.lvLabelPosition.y };
    if (ImGui::DragFloat2("LV Label Pos", labelPosition, 1.0f, -400.0f, 1280.0f)) {
        layoutSettings_.lvLabelPosition = { labelPosition[0], labelPosition[1] };
        ApplyLayout();
    }

    float digitsPosition[2]{ layoutSettings_.lvDigitsPosition.x, layoutSettings_.lvDigitsPosition.y };
    if (ImGui::DragFloat2("LV Digits Pos", digitsPosition, 1.0f, -400.0f, 1280.0f)) {
        layoutSettings_.lvDigitsPosition = { digitsPosition[0], digitsPosition[1] };
        ApplyLayout();
    }

    if (ImGui::Button("Save EXP Layout")) {
        SaveLayout();
    }
#endif
}

void ExpGauge::ApplyLayout() {
    frameBar_.SetPosition(layoutSettings_.framePosition);
    frameBar_.SetSize(layoutSettings_.frameSize);
    gaugeBar_.SetPosition(layoutSettings_.gaugePosition);
    gaugeBar_.SetSize(layoutSettings_.gaugeSize);
    lvLabel_.SetPosition(layoutSettings_.lvLabelPosition);
    lvLabel_.SetSize(layoutSettings_.lvLabelSize);

    for (int32_t i = 0; i < kLvDigits; ++i) {
        if (!sprite_[i]) {
            continue;
        }
        sprite_[i]->SetPosition({ layoutSettings_.lvDigitsPosition.x + size_.x * static_cast<float>(i),
                                  layoutSettings_.lvDigitsPosition.y });
        sprite_[i]->SetSize(size_);
    }
}

void ExpGauge::SaveLayout() const {
    UILayoutIO::Save(kHudLayoutPath, {
        { "expFramePosition", { layoutSettings_.framePosition.x, layoutSettings_.framePosition.y } },
        { "expFrameSize", { layoutSettings_.frameSize.x, layoutSettings_.frameSize.y } },
        { "expGaugePosition", { layoutSettings_.gaugePosition.x, layoutSettings_.gaugePosition.y } },
        { "expGaugeSize", { layoutSettings_.gaugeSize.x, layoutSettings_.gaugeSize.y } },
        { "lvLabelPosition", { layoutSettings_.lvLabelPosition.x, layoutSettings_.lvLabelPosition.y } },
        { "lvLabelSize", { layoutSettings_.lvLabelSize.x, layoutSettings_.lvLabelSize.y } },
        { "lvDigitsPosition", { layoutSettings_.lvDigitsPosition.x, layoutSettings_.lvDigitsPosition.y } },
    });
}

} // namespace DirectXGame
