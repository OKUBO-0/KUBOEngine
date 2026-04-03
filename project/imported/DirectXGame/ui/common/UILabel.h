#pragma once

#include "UIElement.h"
#include <memory>

namespace DirectXGame {

/// <summary>
/// テクスチャ画像を使って固定ラベルを表示する UI クラス。
/// 文字描画基盤が入るまでの共通ラベル窓口として使う。
/// </summary>
class UILabel : public UIElement {
public:
    void Initialize(uint32_t textureHandle, const KamataEngine::Vector2& position);
    void SetColor(const KamataEngine::Vector4& color);
    void Draw() const;

private:
    void OnTransformChanged() override;
    void OnVisualChanged() override;

    KamataEngine::Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::unique_ptr<KamataEngine::Sprite> sprite_;
};

} // namespace DirectXGame
