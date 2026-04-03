#pragma once

#include "Input.h"
#include "Vector2.h"
#include <algorithm>
#include <cmath>

namespace DirectXGamePort::InputBindings {

namespace Detail {

inline float ClampAxis(float value)
{
    return std::clamp(value, -1.0f, 1.0f);
}

inline bool IsGamepadPressed(Input* input, WORD button)
{
    return input != nullptr && input->PushGamePadButton(button);
}

inline bool IsGamepadTriggered(Input* input, WORD button)
{
    return input != nullptr && input->TriggerGamePadButton(button);
}

} // namespace Detail

inline Vector2 GetMoveVector(Input* input)
{
    Vector2 move{ 0.0f, 0.0f };
    if (input == nullptr) {
        return move;
    }

    if (input->PushKey(DIK_W)) { move.y += 1.0f; }
    if (input->PushKey(DIK_S)) { move.y -= 1.0f; }
    if (input->PushKey(DIK_A)) { move.x -= 1.0f; }
    if (input->PushKey(DIK_D)) { move.x += 1.0f; }

    move.x += Detail::ClampAxis(input->GetGamePadStickX());
    move.y += Detail::ClampAxis(input->GetGamePadStickY());

    if (Detail::IsGamepadPressed(input, XINPUT_GAMEPAD_DPAD_UP)) { move.y += 1.0f; }
    if (Detail::IsGamepadPressed(input, XINPUT_GAMEPAD_DPAD_DOWN)) { move.y -= 1.0f; }
    if (Detail::IsGamepadPressed(input, XINPUT_GAMEPAD_DPAD_LEFT)) { move.x -= 1.0f; }
    if (Detail::IsGamepadPressed(input, XINPUT_GAMEPAD_DPAD_RIGHT)) { move.x += 1.0f; }

    const float lengthSq = move.x * move.x + move.y * move.y;
    if (lengthSq > 1.0f) {
        const float length = std::sqrt(lengthSq);
        move.x /= length;
        move.y /= length;
    }

    return move;
}

inline bool GetAimVector(Input* input, Vector2& outAim)
{
    if (input == nullptr) {
        return false;
    }

    const float x = Detail::ClampAxis(input->GetGamePadStickX(true));
    const float y = Detail::ClampAxis(input->GetGamePadStickY(true));
    constexpr float kAimDeadZone = 0.25f;
    if (std::abs(x) >= kAimDeadZone || std::abs(y) >= kAimDeadZone) {
        outAim = { x, y };
        return true;
    }

    return false;
}

inline bool IsMenuUpTriggered(Input* input)
{
    return input != nullptr &&
        (input->TriggerKey(DIK_W) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_DPAD_UP));
}

inline bool IsKeyboardMenuUpTriggered(Input* input)
{
    return input != nullptr && input->TriggerKey(DIK_W);
}

inline bool IsMenuDownTriggered(Input* input)
{
    return input != nullptr &&
        (input->TriggerKey(DIK_S) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_DPAD_DOWN));
}

inline bool IsKeyboardMenuDownTriggered(Input* input)
{
    return input != nullptr && input->TriggerKey(DIK_S);
}

inline bool IsConfirmTriggered(Input* input)
{
    return input != nullptr &&
        (input->TriggerMouse(0) ||
         input->TriggerKey(DIK_SPACE) ||
         input->TriggerKey(DIK_RETURN) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_A));
}

inline bool IsKeyboardConfirmTriggered(Input* input)
{
    return input != nullptr && (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN));
}

inline bool IsCancelTriggered(Input* input)
{
    return input != nullptr &&
        (input->TriggerKey(DIK_ESCAPE) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_B) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_BACK));
}

inline bool IsKeyboardCancelTriggered(Input* input)
{
    return input != nullptr && input->TriggerKey(DIK_ESCAPE);
}

inline bool IsPauseTriggered(Input* input)
{
    return input != nullptr &&
        (input->TriggerKey(DIK_ESCAPE) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_START));
}

inline bool IsKeyboardPauseTriggered(Input* input)
{
    return input != nullptr && input->TriggerKey(DIK_ESCAPE);
}

inline bool IsGamepadMenuUpTriggered(Input* input)
{
    return input != nullptr && Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_DPAD_UP);
}

inline bool IsGamepadMenuDownTriggered(Input* input)
{
    return input != nullptr && Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_DPAD_DOWN);
}

inline bool IsGamepadConfirmTriggered(Input* input)
{
    return input != nullptr && Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_A);
}

inline bool IsGamepadCancelTriggered(Input* input)
{
    return input != nullptr &&
        (Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_B) ||
         Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_BACK));
}

inline bool IsGamepadPauseTriggered(Input* input)
{
    return input != nullptr && Detail::IsGamepadTriggered(input, XINPUT_GAMEPAD_START);
}

inline bool HasMouseNavigationInput(Input* input)
{
    if (input == nullptr) {
        return false;
    }

    const Input::MouseMove move = input->GetMouseMove();
    return move.lX != 0 || move.lY != 0 || input->PushMouse(0);
}

inline bool IsAnyMovePressed(Input* input)
{
    const Vector2 move = GetMoveVector(input);
    return std::abs(move.x) > 0.0f || std::abs(move.y) > 0.0f;
}

} // namespace DirectXGamePort::InputBindings
