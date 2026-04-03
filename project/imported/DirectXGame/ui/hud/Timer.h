#pragma once
#include "../common/UIElement.h"
#include <KamataEngine.h>
#include <memory>
#include <array>

namespace DirectXGame {

class Timer : public UIElement {
public:
    void Initialize();
    void Update(float dt);
    void Draw() const override;

    void SetPosition(const KamataEngine::Vector2& pos);
    void SetScale(float scale);
    void SetTime(float time);

    float GetTime() const { return time_; }
    void Reset() { time_ = 0.0f; }
    void DebugDrawImGui();
    void SaveLayout() const;

private:
    struct LayoutSettings {
        KamataEngine::Vector2 position{ 580.0f, 70.0f };
        float scale = 1.0f;
        bool debugEnabled = false;
    };

    void OnTransformChanged() override;
    void RefreshLayout();
    void UpdateBounds();
    void ApplyLayout();

    float time_ = 0.0f; // 0 → カウントアップ

    static constexpr int32_t kDigitCount = 5; // M1 M2 : S1 S2

    std::array<std::unique_ptr<KamataEngine::Sprite>, kDigitCount> sprite_;
    std::unique_ptr<KamataEngine::Sprite> colonSprite_;

    uint32_t numberTexture_ = 0;
    uint32_t colonTexture_ = 0;

    KamataEngine::Vector2 digitSize_ = { 24.0f, 32.0f };
    float scale_ = 1.0f;
    LayoutSettings layoutSettings_{};

    void UpdateDisplay();
};

} // namespace DirectXGame
