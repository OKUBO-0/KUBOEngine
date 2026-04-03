#pragma once

#include <KamataEngine.h>
#include <base/WinApp.h>
#include <Windows.h>

namespace DirectXGame::ScreenUtil {

inline KamataEngine::Vector2 GetClientSize() {
    RECT clientRect{};
    if (const KamataEngine::WinApp* winApp = KamataEngine::WinApp::GetInstance();
        winApp && GetClientRect(winApp->GetHwnd(), &clientRect)) {
        return {
            static_cast<float>(clientRect.right - clientRect.left),
            static_cast<float>(clientRect.bottom - clientRect.top),
        };
    }

    return {
        static_cast<float>(KamataEngine::WinApp::kWindowWidth),
        static_cast<float>(KamataEngine::WinApp::kWindowHeight),
    };
}

} // namespace DirectXGame::ScreenUtil
