#include "GamePauseController.h"
#include "../../core/InputBindings.h"
#include "../../core/ScreenUtil.h"
#include "EnemyManager.h"
#include "../../player/core/PlayerManager.h"
#include "../../ui/common/UILayoutIO.h"
#include "Player.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <KamataEngine.h>

using namespace KamataEngine;

namespace DirectXGame {

namespace {

const char* kPauseLayoutPath = "Resources/data/ui_layout_pause.csv";

}

void GamePauseController::Initialize() {
    audio_ = Audio::GetInstance();

    uint32_t pauseTexture = TextureManager::Load("ui/game/pause.png");
    pauseOverlay_ = std::unique_ptr<Sprite>(Sprite::Create(pauseTexture, {0, 0}));
    pauseOverlay_->SetSize(ScreenUtil::GetClientSize());

    uint32_t guideTexture = TextureManager::Load("ui/title/guideUI.png");
    guideSprite_ = std::unique_ptr<Sprite>(Sprite::Create(guideTexture, {0, 0}));
    guideSprite_->SetSize(ScreenUtil::GetClientSize());
    guideSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});

    uint32_t cursorTexture = TextureManager::Load("ui/game/pause_arrow.png");
    cursorSprite_ = std::unique_ptr<Sprite>(Sprite::Create(cursorTexture, {0, 0}));
    cursorSprite_->SetSize({1280, 720});

    miniMap_ = std::make_unique<MiniMap>();
    miniMap_->Initialize();

    statsPanel_ = std::make_unique<UIPanel>();
    statsPanel_->Initialize();
    statsPanel_->SetColor({0.0f, 0.0f, 0.0f, 0.72f});

    buildNormalLabel_ = std::make_unique<UILabel>();
    buildNormalLabel_->Initialize(TextureManager::Load("ui/game/normal/icon.png"), layoutSettings_.buildRowLocalPosition);
    buildNormalLabel_->SetParent(statsPanel_.get());
    buildNormalLabel_->SetSize(layoutSettings_.buildIconSize);

    buildOrbitLabel_ = std::make_unique<UILabel>();
    buildOrbitLabel_->Initialize(TextureManager::Load("ui/game/orbit/icon.png"),
                                 {layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX, layoutSettings_.buildRowLocalPosition.y});
    buildOrbitLabel_->SetParent(statsPanel_.get());
    buildOrbitLabel_->SetSize(layoutSettings_.buildIconSize);

    buildDroneLabel_ = std::make_unique<UILabel>();
    buildDroneLabel_->Initialize(TextureManager::Load("ui/game/drone/icon.png"),
                                 {layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX * 2.0f, layoutSettings_.buildRowLocalPosition.y});
    buildDroneLabel_->SetParent(statsPanel_.get());
    buildDroneLabel_->SetSize(layoutSettings_.buildIconSize);

    buildLightningLabel_ = std::make_unique<UILabel>();
    buildLightningLabel_->Initialize(TextureManager::Load("ui/game/lightning/icon.png"),
                                     {layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX * 3.0f, layoutSettings_.buildRowLocalPosition.y});
    buildLightningLabel_->SetParent(statsPanel_.get());
    buildLightningLabel_->SetSize(layoutSettings_.buildIconSize);

    buildAttackLabel_ = std::make_unique<UILabel>();
    buildAttackLabel_->Initialize(TextureManager::Load("ui/game/lvup_attack_icon.png"),
                                  {layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX * 4.0f, layoutSettings_.buildRowLocalPosition.y});
    buildAttackLabel_->SetParent(statsPanel_.get());
    buildAttackLabel_->SetSize(layoutSettings_.buildIconSize);

    selectSEHandle_ = audio_->LoadWave("audio/se/se_pause.wav");
    decideSEHandle_ = audio_->LoadWave("audio/se/se_exp.wav");

    active_ = false;
    guideActive_ = false;
    guideTransitionState_ = GuideTransitionState::None;
    guideAlpha_ = 0.0f;
    goResult_ = false;
    menuIndex_ = 0;
    navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    {
        const auto layout = UILayoutIO::Load(kPauseLayoutPath);
        if (const auto it = layout.find("buildRowLocalPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.buildRowLocalPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("buildStepX"); it != layout.end() && !it->second.empty()) {
            layoutSettings_.buildStepX = it->second[0];
        }
        if (const auto it = layout.find("buildIconSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.buildIconSize = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("menuHitbox0"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.menuHitboxPositions[0] = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("menuHitbox1"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.menuHitboxPositions[1] = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("menuHitboxSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.menuHitboxSize = { it->second[0], it->second[1] };
        }
    }
    ApplyLayout();
    UpdateVisibility();
}

bool GamePauseController::Update(Player* player, const EnemyManager& enemyManager, const PlayerManager& playerManager,
                                 Input* input, Audio* audio, uint32_t toggleSEHandle) {
    if (!guideActive_ && InputBindings::IsPauseTriggered(input)) {
        if (InputBindings::HasMouseNavigationInput(input)) {
            navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
        } else if (InputBindings::IsGamepadPauseTriggered(input)) {
            navigationInputDevice_ = InputBindings::NavigationInputDevice::Gamepad;
        } else if (InputBindings::IsKeyboardPauseTriggered(input)) {
            navigationInputDevice_ = InputBindings::NavigationInputDevice::Keyboard;
        }
        active_ = !active_;
        guideActive_ = false;
        goResult_ = false;
        if (toggleSEHandle != 0) {
            audio->PlayWave(toggleSEHandle, false, 0.5f);
        }

        UpdateVisibility();
        return active_;
    }

    if (!active_) {
        UpdateVisibility();
        return false;
    }

    int32_t hoveredMenuIndex = -1;
    {
        const Vector2 mousePosition = input->GetMousePosition();
        for (int32_t i = 0; i < 2; ++i) {
            const Vector2& rectPosition = layoutSettings_.menuHitboxPositions[i];
            if (mousePosition.x >= rectPosition.x && mousePosition.x <= rectPosition.x + layoutSettings_.menuHitboxSize.x &&
                mousePosition.y >= rectPosition.y && mousePosition.y <= rectPosition.y + layoutSettings_.menuHitboxSize.y) {
                hoveredMenuIndex = i;
                break;
            }
        }
    }

    // ホバー中でもキー/パッド入力を優先し、選択肢上での実マウス操作時だけマウスへ戻す。
    const bool mouseNavigationTriggered = hoveredMenuIndex >= 0 && InputBindings::HasMouseNavigationInput(input);

    if (InputBindings::IsGamepadMenuUpTriggered(input) || InputBindings::IsGamepadMenuDownTriggered(input) ||
               InputBindings::IsGamepadConfirmTriggered(input) || InputBindings::IsGamepadCancelTriggered(input) ||
               InputBindings::IsGamepadPauseTriggered(input)) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Gamepad;
    } else if (InputBindings::IsKeyboardMenuUpTriggered(input) || InputBindings::IsKeyboardMenuDownTriggered(input) ||
               InputBindings::IsKeyboardConfirmTriggered(input) || InputBindings::IsKeyboardCancelTriggered(input) ||
               InputBindings::IsKeyboardPauseTriggered(input)) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Keyboard;
    } else if (mouseNavigationTriggered) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    }

    if (guideTransitionState_ != GuideTransitionState::None) {
        constexpr float kGuideFadeStep = 0.12f;
        if (guideTransitionState_ == GuideTransitionState::FadeIn) {
            guideAlpha_ += kGuideFadeStep;
            if (guideAlpha_ >= 1.0f) {
                guideAlpha_ = 1.0f;
                guideTransitionState_ = GuideTransitionState::None;
                guideActive_ = true;
            }
        } else {
            guideAlpha_ -= kGuideFadeStep;
            if (guideAlpha_ <= 0.0f) {
                guideAlpha_ = 0.0f;
                guideTransitionState_ = GuideTransitionState::None;
                guideActive_ = false;
            }
        }
        guideSprite_->SetColor({1.0f, 1.0f, 1.0f, guideAlpha_});
        UpdateVisibility();
        return true;
    }

    if (guideActive_) {
        bool closeGuide = false;
        switch (navigationInputDevice_) {
        case InputBindings::NavigationInputDevice::Mouse:
            closeGuide = input->IsTriggerMouse(0);
            break;
        case InputBindings::NavigationInputDevice::Gamepad:
            closeGuide = InputBindings::IsGamepadCancelTriggered(input);
            break;
        case InputBindings::NavigationInputDevice::Keyboard:
            closeGuide = InputBindings::IsKeyboardCancelTriggered(input);
            break;
        case InputBindings::NavigationInputDevice::None:
            break;
        }

        if (closeGuide) {
            if (decideSEHandle_ != 0) {
                audio_->PlayWave(decideSEHandle_, false, 1.0f);
            }
            guideTransitionState_ = GuideTransitionState::FadeOut;
        }
        UpdateVisibility();
        return true;
    }

    miniMap_->Update(player, enemyManager);
    UpdateStats(enemyManager, playerManager);
    UpdateVisibility();

    int32_t previousMenuIndex = menuIndex_;
    if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Mouse) {
        if (hoveredMenuIndex >= 0) {
            menuIndex_ = hoveredMenuIndex;
        }
    } else if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Gamepad) {
        if (InputBindings::IsGamepadMenuUpTriggered(input)) {
            menuIndex_ = std::max<int32_t>(0, menuIndex_ - 1);
        }
        if (InputBindings::IsGamepadMenuDownTriggered(input)) {
            menuIndex_ = std::min<int32_t>(1, menuIndex_ + 1);
        }
    } else if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Keyboard) {
        if (InputBindings::IsKeyboardMenuUpTriggered(input)) {
            menuIndex_ = std::max<int32_t>(0, menuIndex_ - 1);
        }
        if (InputBindings::IsKeyboardMenuDownTriggered(input)) {
            menuIndex_ = std::min<int32_t>(1, menuIndex_ + 1);
        }
    }
    if (menuIndex_ != previousMenuIndex && selectSEHandle_ != 0) {
        audio_->PlayWave(selectSEHandle_, false, 1.0f);
    }

    cursorSprite_->SetPosition({0, menuIndex_ == 0 ? 0.0f : 170.0f});

    bool confirmTriggered = false;
    switch (navigationInputDevice_) {
    case InputBindings::NavigationInputDevice::Mouse:
        confirmTriggered = input->IsTriggerMouse(0);
        break;
    case InputBindings::NavigationInputDevice::Gamepad:
        confirmTriggered = InputBindings::IsGamepadConfirmTriggered(input);
        break;
    case InputBindings::NavigationInputDevice::Keyboard:
        confirmTriggered = InputBindings::IsKeyboardConfirmTriggered(input);
        break;
    case InputBindings::NavigationInputDevice::None:
        break;
    }

    if (confirmTriggered) {
        if (decideSEHandle_ != 0) {
            audio_->PlayWave(decideSEHandle_, false, 1.0f);
        }
        if (menuIndex_ == 0) {
            guideAlpha_ = 0.0f;
            guideSprite_->SetColor({1.0f, 1.0f, 1.0f, guideAlpha_});
            guideTransitionState_ = GuideTransitionState::FadeIn;
        } else if (menuIndex_ == 1) {
            goResult_ = true;
        }
    }

    return true;
}

void GamePauseController::Draw() const {
    if (!active_) {
        return;
    }

    pauseOverlay_->Draw();
    miniMap_->Draw();
    if (guideActive_ || guideTransitionState_ != GuideTransitionState::None) {
        guideSprite_->Draw();
    } else {
        buildNormalLabel_->Draw();
        buildOrbitLabel_->Draw();
        buildDroneLabel_->Draw();
        buildLightningLabel_->Draw();
        buildAttackLabel_->Draw();
        cursorSprite_->Draw();
    }
}

void GamePauseController::Reset() {
    active_ = false;
    guideActive_ = false;
    guideTransitionState_ = GuideTransitionState::None;
    guideAlpha_ = 0.0f;
    goResult_ = false;
    menuIndex_ = 0;
    navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    guideSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    UpdateVisibility();
}

bool GamePauseController::IsActive() const {
    return active_;
}

bool GamePauseController::IsGuideActive() const {
    return guideActive_;
}

bool GamePauseController::ShouldGoResult() const {
    return goResult_;
}

void GamePauseController::DebugDrawImGui() {
#ifdef _DEBUG
    if (!ImGui::Begin("UI Debug")) {
        ImGui::End();
        return;
    }

    ImGui::Checkbox("Pause Layout", &debugLayoutEnabled_);
    if (debugLayoutEnabled_ && ImGui::CollapsingHeader("Pause Menu", ImGuiTreeNodeFlags_DefaultOpen)) {
        float buildRowPosition[2]{ layoutSettings_.buildRowLocalPosition.x, layoutSettings_.buildRowLocalPosition.y };
        if (ImGui::DragFloat2("Build Row", buildRowPosition, 1.0f, -400.0f, 400.0f)) {
            layoutSettings_.buildRowLocalPosition = { buildRowPosition[0], buildRowPosition[1] };
            ApplyLayout();
        }

        if (ImGui::DragFloat("Build Spacing", &layoutSettings_.buildStepX, 1.0f, 32.0f, 240.0f)) {
            ApplyLayout();
        }

        float buildIconSize[2]{ layoutSettings_.buildIconSize.x, layoutSettings_.buildIconSize.y };
        if (ImGui::DragFloat2("Build Size", buildIconSize, 1.0f, 16.0f, 256.0f)) {
            layoutSettings_.buildIconSize = { buildIconSize[0], buildIconSize[1] };
            ApplyLayout();
        }

        if (ImGui::Button("Save Pause Layout")) {
            UILayoutIO::Save(kPauseLayoutPath, {
                { "buildRowLocalPosition", { layoutSettings_.buildRowLocalPosition.x, layoutSettings_.buildRowLocalPosition.y } },
                { "buildStepX", { layoutSettings_.buildStepX } },
                { "buildIconSize", { layoutSettings_.buildIconSize.x, layoutSettings_.buildIconSize.y } },
                { "menuHitbox0", { layoutSettings_.menuHitboxPositions[0].x, layoutSettings_.menuHitboxPositions[0].y } },
                { "menuHitbox1", { layoutSettings_.menuHitboxPositions[1].x, layoutSettings_.menuHitboxPositions[1].y } },
                { "menuHitboxSize", { layoutSettings_.menuHitboxSize.x, layoutSettings_.menuHitboxSize.y } },
            });
        }
    }

    ImGui::End();
#endif
}

void GamePauseController::ApplyLayout() {
    buildNormalLabel_->SetPosition(layoutSettings_.buildRowLocalPosition);
    buildNormalLabel_->SetSize(layoutSettings_.buildIconSize);

    buildOrbitLabel_->SetPosition({ layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX,
                                    layoutSettings_.buildRowLocalPosition.y });
    buildOrbitLabel_->SetSize(layoutSettings_.buildIconSize);

    buildDroneLabel_->SetPosition({ layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX * 2.0f,
                                    layoutSettings_.buildRowLocalPosition.y });
    buildDroneLabel_->SetSize(layoutSettings_.buildIconSize);

    buildLightningLabel_->SetPosition({ layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX * 3.0f,
                                        layoutSettings_.buildRowLocalPosition.y });
    buildLightningLabel_->SetSize(layoutSettings_.buildIconSize);

    buildAttackLabel_->SetPosition({ layoutSettings_.buildRowLocalPosition.x + layoutSettings_.buildStepX * 4.0f,
                                     layoutSettings_.buildRowLocalPosition.y });
    buildAttackLabel_->SetSize(layoutSettings_.buildIconSize);
}

void GamePauseController::UpdateStats(const EnemyManager& enemyManager, const PlayerManager& playerManager) {
    (void)enemyManager;

    buildNormalLabel_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    buildOrbitLabel_->SetColor({1.0f, 1.0f, 1.0f, playerManager.HasOrbitBullets() ? 1.0f : 0.25f});
    buildDroneLabel_->SetColor({1.0f, 1.0f, 1.0f, playerManager.HasDrone() ? 1.0f : 0.25f});
    buildLightningLabel_->SetColor({1.0f, 1.0f, 1.0f, playerManager.HasLightning() ? 1.0f : 0.25f});
    buildAttackLabel_->SetColor({1.0f, 1.0f, 1.0f, playerManager.GetAttackPower() > 1 ? 1.0f : 0.4f});
}

void GamePauseController::UpdateVisibility() {
    const bool showBuilds = active_ && !guideActive_ && guideTransitionState_ == GuideTransitionState::None;
    statsPanel_->SetVisible(false);
    buildNormalLabel_->SetVisible(showBuilds);
    buildOrbitLabel_->SetVisible(showBuilds);
    buildDroneLabel_->SetVisible(showBuilds);
    buildLightningLabel_->SetVisible(showBuilds);
    buildAttackLabel_->SetVisible(showBuilds);
}

} // namespace DirectXGame
