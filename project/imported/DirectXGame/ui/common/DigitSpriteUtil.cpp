#include "DigitSpriteUtil.h"

using namespace KamataEngine;

namespace DirectXGame::DigitSpriteUtil {

Vector2 CalculateDigitPosition(const Vector2& basePosition, const Vector2& size, float scale, int32_t index) {
    return { basePosition.x + (size.x * scale * index), basePosition.y };
}

void UpdateDigitLayout(Sprite& sprite, const Vector2& basePosition, const Vector2& size, float scale, int32_t index) {
    sprite.SetSize({ size.x * scale, size.y * scale });
    sprite.SetPosition(CalculateDigitPosition(basePosition, size, scale, index));
}

void SetDigitSprite(Sprite& sprite, float digitWidth, const Vector2& size, int32_t number) {
    sprite.SetTextureRect({ digitWidth * number, 0.0f }, size);
}

} // namespace DirectXGame::DigitSpriteUtil
