#pragma once

#include "UIElement.h"
#include <memory>

namespace DirectXGame {

/// <summary>
/// 単色スプライトを使った簡易 UI パネル。
/// 半透明背景や情報パネルなどの共通表現に使う。
/// </summary>
class UIPanel : public UIElement {
public:
    void Initialize();
    void SetColor(const KamataEngine::Vector4& color);
    void Draw() const;

private:
    void OnTransformChanged() override;
    void OnVisualChanged() override;

    KamataEngine::Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::unique_ptr<KamataEngine::Sprite> sprite_;
};

} // namespace DirectXGame
