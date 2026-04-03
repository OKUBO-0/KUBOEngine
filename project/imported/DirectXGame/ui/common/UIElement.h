#pragma once

#include <KamataEngine.h>
#include <vector>

namespace DirectXGame {

class UIElement {
public:
    enum class Anchor {
        TopLeft,
        TopCenter,
        TopRight,
        MiddleLeft,
        Center,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    virtual ~UIElement() = default;

    void SetPosition(const KamataEngine::Vector2& position);
    void SetSize(const KamataEngine::Vector2& size);
    void SetVisible(bool visible);
    void SetScale(float scale);
    void SetAlpha(float alpha);
    void SetAnchor(Anchor anchor);
    void SetParent(UIElement* parent);
    void AddChild(UIElement* child);
    void RemoveChild(UIElement* child);
    void LayoutChildrenVertically(float spacing);
    void LayoutChildrenHorizontally(float spacing);

    const KamataEngine::Vector2& GetPosition() const { return position_; }
    const KamataEngine::Vector2& GetSize() const { return size_; }
    bool IsVisible() const { return visible_; }
    float GetScale() const { return scale_; }
    float GetAlpha() const { return alpha_; }
    Anchor GetAnchor() const { return anchor_; }
    UIElement* GetParent() const { return parent_; }

    KamataEngine::Vector2 GetWorldPosition() const;
    float GetWorldScale() const;
    float GetWorldAlpha() const;
    KamataEngine::Vector2 GetScaledSize() const;
    KamataEngine::Vector2 GetAnchorOffset() const;
    KamataEngine::Vector2 GetLocalAnchorOffset() const;
    void DrawChildren() const;

protected:
    virtual void OnTransformChanged() {}
    virtual void OnVisualChanged() {}

    KamataEngine::Vector2 position_{ 0.0f, 0.0f };
    KamataEngine::Vector2 size_{ 0.0f, 0.0f };
    bool visible_ = true;
    float scale_ = 1.0f;
    float alpha_ = 1.0f;
    Anchor anchor_ = Anchor::TopLeft;
    UIElement* parent_ = nullptr;
    std::vector<UIElement*> children_;

private:
    void NotifyChildrenTransformChanged();
    void NotifyChildrenVisualChanged();

public:
    virtual void Draw() const = 0;
};

} // namespace DirectXGame
