#pragma once

#include "UIElement.h"
#include "UIPanel.h"
#include <KamataEngine.h>

namespace DirectXGame {

/// <summary>
/// 背景と前景の2枚で構成する簡易ゲージUI。
/// 残量に応じて前景幅を変化させる。
/// </summary>
class UIBar : public UIElement {
public:
    void Initialize();
    void SetColors(const KamataEngine::Vector4& backgroundColor, const KamataEngine::Vector4& fillColor);
    void SetRate(float rate);
    void Draw() const;

private:
    void OnTransformChanged() override;
    void RefreshLayout();

    UIPanel background_;
    UIPanel fill_;
    float rate_ = 1.0f;
};

} // namespace DirectXGame
