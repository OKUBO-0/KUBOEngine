#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <cmath>

namespace DirectXGame::InputBindings {

enum class NavigationInputDevice {
    None,
    Mouse,
    Keyboard,
    Gamepad,
};

namespace Detail {

inline float ClampAxis(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

inline float NormalizeDirectInputAxis(LONG value) {
    constexpr float kCenter = 32767.5f;
    return ClampAxis((static_cast<float>(value) - kCenter) / kCenter);
}

inline float NormalizeXInputAxis(SHORT value) {
    constexpr float kAxisMax = 32767.0f;
    return ClampAxis(static_cast<float>(value) / kAxisMax);
}

inline bool IsDirectInputButtonPressed(const DIJOYSTATE2& state, int buttonIndex) {
    return buttonIndex >= 0 && buttonIndex < 128 && (state.rgbButtons[buttonIndex] & 0x80) != 0;
}

inline bool IsDirectInputPovPressed(const DIJOYSTATE2& state, int angle) {
    if (state.rgdwPOV[0] == 0xFFFFFFFF) {
        return false;
    }

    const DWORD pov = state.rgdwPOV[0];
    const int lower = angle - 4500;
    const int upper = angle + 4500;
    if (lower < 0) {
        return pov >= static_cast<DWORD>(36000 + lower) || pov <= static_cast<DWORD>(upper);
    }
    if (upper >= 36000) {
        return pov >= static_cast<DWORD>(lower) || pov <= static_cast<DWORD>(upper - 36000);
    }
    return pov >= static_cast<DWORD>(lower) && pov <= static_cast<DWORD>(upper);
}

inline bool GetFirstXInputState(KamataEngine::Input* input, XINPUT_STATE& current, XINPUT_STATE& previous) {
    if (!input) {
        return false;
    }

    const size_t count = input->GetNumberOfJoysticks();
    for (size_t i = 0; i < count; ++i) {
        XINPUT_STATE state{};
        XINPUT_STATE prev{};
        if (input->GetJoystickState(static_cast<int32_t>(i), state) &&
            input->GetJoystickStatePrevious(static_cast<int32_t>(i), prev)) {
            current = state;
            previous = prev;
            return true;
        }
    }

    return false;
}

inline bool GetFirstDirectInputState(KamataEngine::Input* input, DIJOYSTATE2& current, DIJOYSTATE2& previous) {
    if (!input) {
        return false;
    }

    const size_t count = input->GetNumberOfJoysticks();
    for (size_t i = 0; i < count; ++i) {
        DIJOYSTATE2 state{};
        DIJOYSTATE2 prev{};
        if (input->GetJoystickState(static_cast<int32_t>(i), state) &&
            input->GetJoystickStatePrevious(static_cast<int32_t>(i), prev)) {
            current = state;
            previous = prev;
            return true;
        }
    }

    return false;
}

} // namespace Detail

inline KamataEngine::Vector2 GetMoveVector(KamataEngine::Input* input) {
    using namespace KamataEngine;

    Vector2 move{0.0f, 0.0f};
    if (!input) {
        return move;
    }

    if (input->PushKey(DIK_W)) { move.y += 1.0f; }
    if (input->PushKey(DIK_S)) { move.y -= 1.0f; }
    if (input->PushKey(DIK_A)) { move.x -= 1.0f; }
    if (input->PushKey(DIK_D)) { move.x += 1.0f; }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        constexpr float kStickDeadZone = 0.22f;
        float stickX = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbLX);
        float stickY = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbLY);
        if (std::abs(stickX) >= kStickDeadZone) {
            move.x += stickX;
        }
        if (std::abs(stickY) >= kStickDeadZone) {
            move.y += stickY;
        }
        if (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) { move.y += 1.0f; }
        if (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) { move.y -= 1.0f; }
        if (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) { move.x -= 1.0f; }
        if (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) { move.x += 1.0f; }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        constexpr float kStickDeadZone = 0.22f;
        float stickX = Detail::NormalizeDirectInputAxis(dState.lX);
        float stickY = -Detail::NormalizeDirectInputAxis(dState.lY);
        if (std::abs(stickX) >= kStickDeadZone) {
            move.x += stickX;
        }
        if (std::abs(stickY) >= kStickDeadZone) {
            move.y += stickY;
        }
        if (Detail::IsDirectInputPovPressed(dState, 0)) { move.y += 1.0f; }
        if (Detail::IsDirectInputPovPressed(dState, 18000)) { move.y -= 1.0f; }
        if (Detail::IsDirectInputPovPressed(dState, 27000)) { move.x -= 1.0f; }
        if (Detail::IsDirectInputPovPressed(dState, 9000)) { move.x += 1.0f; }
    }

    const float lengthSq = move.x * move.x + move.y * move.y;
    if (lengthSq > 1.0f) {
        const float length = std::sqrt(lengthSq);
        move.x /= length;
        move.y /= length;
    }

    return move;
}

inline bool GetAimVector(KamataEngine::Input* input, KamataEngine::Vector2& outAim) {
    if (!input) {
        return false;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        constexpr float kAimDeadZone = 0.25f;
        const float x = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbRX);
        const float y = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbRY);
        if (std::abs(x) >= kAimDeadZone || std::abs(y) >= kAimDeadZone) {
            outAim = {x, y};
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        constexpr float kAimDeadZone = 0.25f;
        const float x = Detail::NormalizeDirectInputAxis(dState.lRx);
        const float y = -Detail::NormalizeDirectInputAxis(dState.lRy);
        if (std::abs(x) >= kAimDeadZone || std::abs(y) >= kAimDeadZone) {
            outAim = {x, y};
            return true;
        }
    }

    return false;
}

inline bool IsMenuUpTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    if (input->TriggerKey(DIK_W)) {
        return true;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const bool currentDpad = (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
        const bool previousDpad = (xPrev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
        const bool currentStick = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbLY) >= 0.6f;
        const bool previousStick = Detail::NormalizeXInputAxis(xPrev.Gamepad.sThumbLY) >= 0.6f;
        if ((currentDpad && !previousDpad) || (currentStick && !previousStick)) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        const bool currentPov = Detail::IsDirectInputPovPressed(dState, 0);
        const bool previousPov = Detail::IsDirectInputPovPressed(dPrev, 0);
        const bool currentStick = -Detail::NormalizeDirectInputAxis(dState.lY) >= 0.6f;
        const bool previousStick = -Detail::NormalizeDirectInputAxis(dPrev.lY) >= 0.6f;
        if ((currentPov && !previousPov) || (currentStick && !previousStick)) {
            return true;
        }
    }

    return false;
}

inline bool IsKeyboardMenuUpTriggered(KamataEngine::Input* input) {
    return input && input->TriggerKey(DIK_W);
}

inline bool IsMenuDownTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    if (input->TriggerKey(DIK_S)) {
        return true;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const bool currentDpad = (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        const bool previousDpad = (xPrev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        const bool currentStick = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbLY) <= -0.6f;
        const bool previousStick = Detail::NormalizeXInputAxis(xPrev.Gamepad.sThumbLY) <= -0.6f;
        if ((currentDpad && !previousDpad) || (currentStick && !previousStick)) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        const bool currentPov = Detail::IsDirectInputPovPressed(dState, 18000);
        const bool previousPov = Detail::IsDirectInputPovPressed(dPrev, 18000);
        const bool currentStick = -Detail::NormalizeDirectInputAxis(dState.lY) <= -0.6f;
        const bool previousStick = -Detail::NormalizeDirectInputAxis(dPrev.lY) <= -0.6f;
        if ((currentPov && !previousPov) || (currentStick && !previousStick)) {
            return true;
        }
    }

    return false;
}

inline bool IsKeyboardMenuDownTriggered(KamataEngine::Input* input) {
    return input && input->TriggerKey(DIK_S);
}

inline bool IsConfirmTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    if (input->IsTriggerMouse(0)) {
        return true;
    }

    if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
        return true;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const WORD current = xState.Gamepad.wButtons;
        const WORD previous = xPrev.Gamepad.wButtons;
        const WORD mask = XINPUT_GAMEPAD_A;
        if ((current & mask) != 0 && (previous & mask) == 0) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        for (int buttonIndex : {0}) {
            if (Detail::IsDirectInputButtonPressed(dState, buttonIndex) &&
                !Detail::IsDirectInputButtonPressed(dPrev, buttonIndex)) {
                return true;
            }
        }
    }

    return false;
}

inline bool IsKeyboardConfirmTriggered(KamataEngine::Input* input) {
    return input && (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN));
}

inline bool IsCancelTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    if (input->TriggerKey(DIK_ESCAPE)) {
        return true;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const WORD current = xState.Gamepad.wButtons;
        const WORD previous = xPrev.Gamepad.wButtons;
        const WORD mask = XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_BACK;
        if ((current & mask) != 0 && (previous & mask) == 0) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        for (int buttonIndex : {1, 6}) {
            if (Detail::IsDirectInputButtonPressed(dState, buttonIndex) &&
                !Detail::IsDirectInputButtonPressed(dPrev, buttonIndex)) {
                return true;
            }
        }
    }

    return false;
}

inline bool IsKeyboardCancelTriggered(KamataEngine::Input* input) {
    return input && input->TriggerKey(DIK_ESCAPE);
}

inline bool IsPauseTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    if (input->TriggerKey(DIK_ESCAPE)) {
        return true;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const bool current = (xState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
        const bool previous = (xPrev.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
        if (current && !previous) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        if (Detail::IsDirectInputButtonPressed(dState, 7) &&
            !Detail::IsDirectInputButtonPressed(dPrev, 7)) {
            return true;
        }
    }

    return false;
}

inline bool IsKeyboardPauseTriggered(KamataEngine::Input* input) {
    return input && input->TriggerKey(DIK_ESCAPE);
}

inline bool IsGamepadMenuUpTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const bool currentDpad = (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
        const bool previousDpad = (xPrev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
        const bool currentStick = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbLY) >= 0.6f;
        const bool previousStick = Detail::NormalizeXInputAxis(xPrev.Gamepad.sThumbLY) >= 0.6f;
        if ((currentDpad && !previousDpad) || (currentStick && !previousStick)) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        const bool currentPov = Detail::IsDirectInputPovPressed(dState, 0);
        const bool previousPov = Detail::IsDirectInputPovPressed(dPrev, 0);
        const bool currentStick = -Detail::NormalizeDirectInputAxis(dState.lY) >= 0.6f;
        const bool previousStick = -Detail::NormalizeDirectInputAxis(dPrev.lY) >= 0.6f;
        if ((currentPov && !previousPov) || (currentStick && !previousStick)) {
            return true;
        }
    }

    return false;
}

inline bool IsGamepadMenuDownTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const bool currentDpad = (xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        const bool previousDpad = (xPrev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        const bool currentStick = Detail::NormalizeXInputAxis(xState.Gamepad.sThumbLY) <= -0.6f;
        const bool previousStick = Detail::NormalizeXInputAxis(xPrev.Gamepad.sThumbLY) <= -0.6f;
        if ((currentDpad && !previousDpad) || (currentStick && !previousStick)) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        const bool currentPov = Detail::IsDirectInputPovPressed(dState, 18000);
        const bool previousPov = Detail::IsDirectInputPovPressed(dPrev, 18000);
        const bool currentStick = -Detail::NormalizeDirectInputAxis(dState.lY) <= -0.6f;
        const bool previousStick = -Detail::NormalizeDirectInputAxis(dPrev.lY) <= -0.6f;
        if ((currentPov && !previousPov) || (currentStick && !previousStick)) {
            return true;
        }
    }

    return false;
}

inline bool IsGamepadConfirmTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const WORD current = xState.Gamepad.wButtons;
        const WORD previous = xPrev.Gamepad.wButtons;
        if ((current & XINPUT_GAMEPAD_A) != 0 && (previous & XINPUT_GAMEPAD_A) == 0) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        return Detail::IsDirectInputButtonPressed(dState, 0) &&
               !Detail::IsDirectInputButtonPressed(dPrev, 0);
    }

    return false;
}

inline bool IsGamepadCancelTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    const bool hasXInput = Detail::GetFirstXInputState(input, xState, xPrev);
    if (hasXInput) {
        const WORD current = xState.Gamepad.wButtons;
        const WORD previous = xPrev.Gamepad.wButtons;
        const WORD mask = XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_BACK;
        if ((current & mask) != 0 && (previous & mask) == 0) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (!hasXInput && Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        for (int buttonIndex : {1, 6}) {
            if (Detail::IsDirectInputButtonPressed(dState, buttonIndex) &&
                !Detail::IsDirectInputButtonPressed(dPrev, buttonIndex)) {
                return true;
            }
        }
    }

    return false;
}

inline bool IsGamepadPauseTriggered(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    XINPUT_STATE xState{};
    XINPUT_STATE xPrev{};
    if (Detail::GetFirstXInputState(input, xState, xPrev)) {
        const bool current = (xState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
        const bool previous = (xPrev.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
        if (current && !previous) {
            return true;
        }
    }

    DIJOYSTATE2 dState{};
    DIJOYSTATE2 dPrev{};
    if (Detail::GetFirstDirectInputState(input, dState, dPrev)) {
        return Detail::IsDirectInputButtonPressed(dState, 7) &&
               !Detail::IsDirectInputButtonPressed(dPrev, 7);
    }

    return false;
}

inline bool HasMouseNavigationInput(KamataEngine::Input* input) {
    if (!input) {
        return false;
    }

    const auto mouseMove = input->GetMouseMove();
    return mouseMove.lX != 0 || mouseMove.lY != 0 || input->IsTriggerMouse(0);
}

inline bool IsAnyMovePressed(KamataEngine::Input* input) {
    const KamataEngine::Vector2 move = GetMoveVector(input);
    return std::abs(move.x) > 0.05f || std::abs(move.y) > 0.05f;
}

} // namespace DirectXGame::InputBindings
