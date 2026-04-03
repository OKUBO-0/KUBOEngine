#include "GameLevelUpController.h"
#include "../../core/InputBindings.h"
#include "../../core/ScreenUtil.h"
#include "../../player/core/PlayerManager.h"
#include "../../ui/common/UILayoutIO.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <random>

using namespace KamataEngine;

namespace DirectXGame {

namespace {

const char* kLevelUpLayoutPath = "Resources/data/ui_layout_levelup.csv";
constexpr float kLevelUpSlideSpeed = 4200.0f;
constexpr float kConfettiGravity = 980.0f;
uint32_t LoadTextureWithFallback(const std::string& preferredPath, const char* fallbackPath) {
    if (std::filesystem::exists(preferredPath)) {
        return TextureManager::Load(preferredPath.c_str());
    }
    return TextureManager::Load(fallbackPath);
}

uint32_t GetNormalLevelUpTexture(PlayerManager* playerManager) {
    const int32_t nextLevel =
        (std::min)(playerManager->GetNormalBulletLevel() + 1, PlayerManager::kNormalBulletMaxLevel);
    const std::string levelPath = "Resources/ui/game/normal/lv" + std::to_string(nextLevel) + ".png";
    return LoadTextureWithFallback(levelPath, "ui/game/normal/choice.png");
}

uint32_t GetOrbitLevelUpTexture(PlayerManager* playerManager) {
    if (!playerManager->HasOrbitBullets()) {
        return TextureManager::Load("ui/game/orbit/add.png");
    }

    const int32_t nextLevel =
        (std::min)(playerManager->GetOrbitBulletLevel() + 1, PlayerManager::kOrbitBulletMaxLevel);
    const std::string levelPath = "Resources/ui/game/orbit/lv" + std::to_string(nextLevel) + ".png";
    return LoadTextureWithFallback(levelPath, "ui/game/orbit/upgrade.png");
}

uint32_t GetDroneLevelUpTexture(PlayerManager* playerManager) {
    if (!playerManager->HasDrone()) {
        return TextureManager::Load("ui/game/drone/add.png");
    }

    const int32_t nextLevel = (std::min)(playerManager->GetDroneLevel() + 1, PlayerManager::kDroneMaxLevel);
    const std::string levelPath = "Resources/ui/game/drone/lv" + std::to_string(nextLevel) + ".png";
    return LoadTextureWithFallback(levelPath, "ui/game/drone/upgrade.png");
}

uint32_t GetLightningLevelUpTexture(PlayerManager* playerManager) {
    if (!playerManager->HasLightning()) {
        return TextureManager::Load("ui/game/lightning/add.png");
    }

    const int32_t nextLevel =
        (std::min)(playerManager->GetLightningLevel() + 1, PlayerManager::kLightningMaxLevel);
    const std::string levelPath = "Resources/ui/game/lightning/lv" + std::to_string(nextLevel) + ".png";
    return LoadTextureWithFallback(levelPath, "ui/game/lightning/upgrade.png");
}

std::vector<LevelUpOption> BuildCandidateOptions(const std::vector<LevelUpOption>& baseOptions, PlayerManager* playerManager) {
    std::vector<LevelUpOption> candidates;
    candidates.reserve(baseOptions.size());

    for (const auto& option : baseOptions) {
        LevelUpOption adjusted = option;

        if (option.name == "周囲弾強化") {
            if (playerManager->IsOrbitBulletMaxLevel()) {
                continue;
            }
            if (!playerManager->HasOrbitBullets()) {
                adjusted.weight = 2.2f;
            }
        } else if (option.name == "通常弾強化") {
            if (playerManager->IsNormalBulletMaxLevel()) {
                continue;
            }
            const float interval = playerManager->GetNormalBulletInterval();
            if (playerManager->GetNormalBulletLevel() >= 7) {
                adjusted.weight = 0.35f;
            } else if (interval <= 0.24f) {
                adjusted.weight = 0.5f;
            } else if (interval <= 0.32f) {
                adjusted.weight = 0.7f;
            } else if (interval <= 0.45f) {
                adjusted.weight = 1.1f;
            }
        } else if (option.name == "ドローン") {
            if (playerManager->IsDroneMaxLevel()) {
                continue;
            }
            if (!playerManager->HasDrone()) {
                adjusted.weight = 1.8f;
            }
        } else if (option.name == "ライトニング") {
            if (playerManager->IsLightningMaxLevel()) {
                continue;
            }
            if (!playerManager->HasLightning()) {
                adjusted.weight = 1.4f;
            }
        } else if (option.name == "攻撃力 +1") {
            const int attackPower = playerManager->GetAttackPower();
            adjusted.weight = attackPower >= 6 ? 0.75f : (attackPower >= 4 ? 1.1f : 1.5f);
        } else if (option.name == "最大HP増加") {
            if (playerManager->GetMaxHP() >= 6) {
                continue;
            }

            adjusted.weight = playerManager->GetMaxHP() <= 4 ? 1.5f : 0.85f;
        } else if (option.name == "移動速度アップ") {
            if (playerManager->GetMoveSpeedLevel() >= 5) {
                continue;
            }

            adjusted.weight = playerManager->GetMoveSpeedLevel() == 0 ? 1.5f : 1.0f;
        } else if (option.name == "HP回復") {
            if (playerManager->GetHP() >= playerManager->GetMaxHP()) {
                continue;
            }

            const float hpRatio =
                static_cast<float>(playerManager->GetHP()) / static_cast<float>((std::max)(1, playerManager->GetMaxHP()));
            adjusted.weight = hpRatio <= 0.34f ? 2.5f : (hpRatio <= 0.67f ? 1.4f : 0.7f);
        }

        candidates.push_back(std::move(adjusted));
    }

    return candidates;
}

int32_t GetHoveredChoiceIndex(Input* input,
                              const Vector2* choicePositions,
                              const Vector2& hitboxOffset,
                              const Vector2& hitboxSize,
                              float slideOffsetX,
                              int32_t choiceCount) {
    if (!input || choiceCount <= 0) {
        return -1;
    }

    const Vector2 mousePosition = input->GetMousePosition();
    for (int32_t i = 0; i < choiceCount; ++i) {
        const Vector2 rectPosition{
            choicePositions[i].x + hitboxOffset.x + slideOffsetX,
            choicePositions[i].y + hitboxOffset.y,
        };
        if (mousePosition.x >= rectPosition.x && mousePosition.x <= rectPosition.x + hitboxSize.x &&
            mousePosition.y >= rectPosition.y && mousePosition.y <= rectPosition.y + hitboxSize.y) {
            return i;
        }
    }

    return -1;
}

} // namespace

void GameLevelUpController::Initialize() {
    uint32_t overlayTexture = TextureManager::Load("ui/game/levelup.png");
    overlaySprite_ = std::unique_ptr<Sprite>(Sprite::Create(overlayTexture, {0, 0}));
    overlaySprite_->SetSize(ScreenUtil::GetClientSize());

    uint32_t arrowTexture = TextureManager::Load("ui/game/arrow.png");
    arrowSprite_ = std::unique_ptr<Sprite>(Sprite::Create(arrowTexture, {0, 0}));
    {
        const auto layout = UILayoutIO::Load(kLevelUpLayoutPath);
        for (int i = 0; i < 3; ++i) {
            const std::string key = "choicePosition" + std::to_string(i);
            if (const auto it = layout.find(key); it != layout.end() && it->second.size() >= 2) {
                layoutSettings_.choicePositions[i] = { it->second[0], it->second[1] };
            }
        }
        if (const auto it = layout.find("choiceSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.choiceSize = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("arrowBasePosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.arrowBasePosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("choiceSpacingY"); it != layout.end() && !it->second.empty()) {
            layoutSettings_.choiceSpacingY = it->second[0];
        }
        if (const auto it = layout.find("choiceHitboxOffset"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.choiceHitboxOffset = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("choiceHitboxSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.choiceHitboxSize = { it->second[0], it->second[1] };
        }
    }
    ApplyLayout();
}

void GameLevelUpController::RegisterDefaultOptions() {
    options_.clear();

    options_.push_back({
        "通常弾強化",
        [](PlayerManager* pm) { pm->UpgradeNormalBullets(); },
        [](PlayerManager* pm) { return GetNormalLevelUpTexture(pm); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/normal/icon.png"); },
        2.0f
    });

    options_.push_back({
        "周囲弾強化",
        [](PlayerManager* pm) {
            if (!pm->HasOrbitBullets()) {
                pm->AddOrbitBullets();
            } else {
                pm->UpgradeOrbitBullets();
            }
        },
        [](PlayerManager* pm) { return GetOrbitLevelUpTexture(pm); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/orbit/icon.png"); },
        1.0f
    });

    options_.push_back({
        "ドローン",
        [](PlayerManager* pm) {
            if (!pm->HasDrone()) {
                pm->AddDrone();
            } else {
                pm->UpgradeDrone();
            }
        },
        [](PlayerManager* pm) { return GetDroneLevelUpTexture(pm); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/drone/icon.png"); },
        0.5f
    });

    options_.push_back({
        "ライトニング",
        [](PlayerManager* pm) {
            if (!pm->HasLightning()) {
                pm->AddLightning();
            } else {
                pm->UpgradeLightning();
            }
        },
        [](PlayerManager* pm) { return GetLightningLevelUpTexture(pm); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lightning/icon.png"); },
        0.7f
    });

    options_.push_back({
        "攻撃力 +1",
        [](PlayerManager* pm) { pm->UpgradeAttackPower(); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_attack.png"); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_attack_icon.png"); },
        1.5f
    });

    options_.push_back({
        "移動速度アップ",
        [](PlayerManager* pm) { pm->UpgradeMoveSpeed(); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_speed.png"); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_speed_icon.png"); },
        1.1f
    });

    options_.push_back({
        "HP回復",
        [](PlayerManager* pm) { pm->RecoverHP(); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_heal.png"); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_heal_icon.png"); },
        1.0f
    });

    options_.push_back({
        "最大HP増加",
        [](PlayerManager* pm) { pm->IncreaseMaxHP(); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_maxhp.png"); },
        [](PlayerManager*) { return TextureManager::Load("ui/game/lvup_maxhp_icon.png"); },
        0.8f
    });
}

bool GameLevelUpController::TryStart(PlayerManager* playerManager, Audio* audio, uint32_t levelUpSEHandle) {
    if (!playerManager->IsLevelUpRequested()) {
        return false;
    }

    if (levelUpSEHandle != 0) {
        audio->PlayWave(levelUpSEHandle, false, 1.0f);
    }

    currentChoices_.clear();
    const std::vector<LevelUpOption> candidateOptions = BuildCandidateOptions(options_, playerManager);
    if (candidateOptions.empty()) {
        return false;
    }

    std::vector<int32_t> pickedIndices;
    const int32_t choiceCount = (std::min)(3, static_cast<int32_t>(candidateOptions.size()));
    for (int32_t i = 0; i < choiceCount; ++i) {
        int32_t pickedIndex = 0;
        while (true) {
            pickedIndex = PickWeightedOptionIndex(candidateOptions);
            if (std::find(pickedIndices.begin(), pickedIndices.end(), pickedIndex) == pickedIndices.end()) {
                break;
            }
        }

        pickedIndices.push_back(pickedIndex);
        currentChoices_.push_back(candidateOptions[pickedIndex]);
    }

    for (int32_t i = 0; i < choiceCount; ++i) {
        uint32_t textureHandle = currentChoices_[i].getTexture(playerManager);
        choiceSprites_[i] = std::unique_ptr<Sprite>(
            Sprite::Create(textureHandle, layoutSettings_.choicePositions[i])
        );
        choiceSprites_[i]->SetSize(layoutSettings_.choiceSize);

        uint32_t iconTextureHandle = currentChoices_[i].getIconTexture(playerManager);
        iconSprites_[i] = std::unique_ptr<Sprite>(
            Sprite::Create(iconTextureHandle, layoutSettings_.choicePositions[i])
        );
        iconSprites_[i]->SetSize(layoutSettings_.choiceSize);
    }
    for (int32_t i = choiceCount; i < 3; ++i) {
        choiceSprites_[i].reset();
        iconSprites_[i].reset();
    }

    active_ = true;
    selection_ = 0;
    animationState_ = AnimationState::Entering;
    slideOffsetX_ = ScreenUtil::GetClientSize().x;
    pendingAction_ = nullptr;
    navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    SpawnConfetti();
    ApplyLayout();
    playerManager->ClearLevelUpRequest();
    return true;
}

bool GameLevelUpController::Update(PlayerManager* playerManager, Input* input, Audio* audio,
                                   uint32_t moveSEHandle, uint32_t decideSEHandle, float deltaTime) {
    if (!active_) {
        return false;
    }

    if (currentChoices_.empty()) {
        active_ = false;
        selection_ = 0;
        return false;
    }

    UpdateConfetti(deltaTime);
    UpdateSlideAnimation(deltaTime);
    if (animationState_ == AnimationState::Exiting) {
        if (slideOffsetX_ <= -ScreenUtil::GetClientSize().x) {
            if (pendingAction_) {
                pendingAction_(playerManager);
                pendingAction_ = nullptr;
            }
            active_ = false;
            animationState_ = AnimationState::Hidden;
            selection_ = 0;
            ApplyLayout();
            return false;
        }

        return true;
    }

    if (animationState_ != AnimationState::Idle) {
        return true;
    }

    const int32_t maxSelectionIndex = static_cast<int32_t>(currentChoices_.size()) - 1;
    selection_ = (std::clamp)(selection_, 0, maxSelectionIndex);

    const int32_t hoveredChoiceIndex = GetHoveredChoiceIndex(
        input, layoutSettings_.choicePositions, layoutSettings_.choiceHitboxOffset, layoutSettings_.choiceHitboxSize, slideOffsetX_,
        static_cast<int32_t>(currentChoices_.size()));

    // 入力元の競合を防ぐため、マウスは選択肢上で操作したときだけ有効化する。
    const bool mouseNavigationTriggered = hoveredChoiceIndex >= 0 && InputBindings::HasMouseNavigationInput(input);

    if (InputBindings::IsGamepadMenuUpTriggered(input) || InputBindings::IsGamepadMenuDownTriggered(input) ||
               InputBindings::IsGamepadConfirmTriggered(input) || InputBindings::IsGamepadCancelTriggered(input)) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Gamepad;
    } else if (InputBindings::IsKeyboardMenuUpTriggered(input) || InputBindings::IsKeyboardMenuDownTriggered(input) ||
               InputBindings::IsKeyboardConfirmTriggered(input) || InputBindings::IsKeyboardCancelTriggered(input)) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Keyboard;
    } else if (mouseNavigationTriggered) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    }

    int32_t previousSelection = selection_;
    if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Mouse) {
        if (hoveredChoiceIndex >= 0) {
            selection_ = hoveredChoiceIndex;
        }
    } else if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Gamepad) {
        if (InputBindings::IsGamepadMenuUpTriggered(input)) {
            selection_ = std::max<int32_t>(0, selection_ - 1);
        } else if (InputBindings::IsGamepadMenuDownTriggered(input)) {
            selection_ = (std::min)(maxSelectionIndex, selection_ + 1);
        }
    } else if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Keyboard) {
        if (InputBindings::IsKeyboardMenuUpTriggered(input)) {
            selection_ = std::max<int32_t>(0, selection_ - 1);
        } else if (InputBindings::IsKeyboardMenuDownTriggered(input)) {
            selection_ = (std::min)(maxSelectionIndex, selection_ + 1);
        }
    }

    if (selection_ != previousSelection && moveSEHandle != 0) {
        audio->PlayWave(moveSEHandle, false, 0.5f);
    }

    arrowSprite_->SetPosition({layoutSettings_.arrowBasePosition.x,
                               layoutSettings_.arrowBasePosition.y + static_cast<float>(selection_) * layoutSettings_.choiceSpacingY});

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
        if (decideSEHandle != 0) {
            audio->PlayWave(decideSEHandle, false, 1.0f);
        }
        pendingAction_ = currentChoices_[selection_].action;
        animationState_ = AnimationState::Exiting;
    }

    return true;
}

void GameLevelUpController::Draw() const {
    if (!active_) {
        return;
    }

    overlaySprite_->Draw();
    for (const auto& particle : confettiParticles_) {
        if (particle.sprite) {
            particle.sprite->Draw();
        }
    }
    for (const auto& sprite : choiceSprites_) {
        if (sprite) {
            sprite->Draw();
        }
    }
    for (const auto& sprite : iconSprites_) {
        if (sprite) {
            sprite->Draw();
        }
    }
    arrowSprite_->Draw();
}

void GameLevelUpController::DebugDrawImGui() {
#ifdef _DEBUG
    if (!ImGui::Begin("UI Debug")) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Level Up", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable LevelUp Debug", &layoutSettings_.debugEnabled);
        if (layoutSettings_.debugEnabled) {
            float choiceSize[2]{ layoutSettings_.choiceSize.x, layoutSettings_.choiceSize.y };
            if (ImGui::DragFloat2("Choice Size", choiceSize, 1.0f, 64.0f, 1280.0f)) {
                layoutSettings_.choiceSize = { choiceSize[0], choiceSize[1] };
                ApplyLayout();
            }

            for (int i = 0; i < 3; ++i) {
                float choicePosition[2]{ layoutSettings_.choicePositions[i].x, layoutSettings_.choicePositions[i].y };
                const std::string label = "Choice " + std::to_string(i + 1);
                if (ImGui::DragFloat2(label.c_str(), choicePosition, 1.0f, -400.0f, 1280.0f)) {
                    layoutSettings_.choicePositions[i] = { choicePosition[0], choicePosition[1] };
                    ApplyLayout();
                }
            }

            float arrowBase[2]{ layoutSettings_.arrowBasePosition.x, layoutSettings_.arrowBasePosition.y };
            if (ImGui::DragFloat2("Arrow Base", arrowBase, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.arrowBasePosition = { arrowBase[0], arrowBase[1] };
                ApplyLayout();
            }

            if (ImGui::DragFloat("Choice Spacing", &layoutSettings_.choiceSpacingY, 1.0f, 16.0f, 320.0f)) {
                ApplyLayout();
            }

            float hitboxOffset[2]{ layoutSettings_.choiceHitboxOffset.x, layoutSettings_.choiceHitboxOffset.y };
            if (ImGui::DragFloat2("Choice Hitbox Offset", hitboxOffset, 1.0f, 0.0f, 1280.0f)) {
                layoutSettings_.choiceHitboxOffset = { hitboxOffset[0], hitboxOffset[1] };
            }

            float hitboxSize[2]{ layoutSettings_.choiceHitboxSize.x, layoutSettings_.choiceHitboxSize.y };
            if (ImGui::DragFloat2("Choice Hitbox Size", hitboxSize, 1.0f, 16.0f, 1280.0f)) {
                layoutSettings_.choiceHitboxSize = { hitboxSize[0], hitboxSize[1] };
            }

            if (ImGui::Button("Save LevelUp Layout")) {
                UILayoutIO::Save(kLevelUpLayoutPath, {
                    { "choicePosition0", { layoutSettings_.choicePositions[0].x, layoutSettings_.choicePositions[0].y } },
                    { "choicePosition1", { layoutSettings_.choicePositions[1].x, layoutSettings_.choicePositions[1].y } },
                    { "choicePosition2", { layoutSettings_.choicePositions[2].x, layoutSettings_.choicePositions[2].y } },
                    { "choiceSize", { layoutSettings_.choiceSize.x, layoutSettings_.choiceSize.y } },
                    { "arrowBasePosition", { layoutSettings_.arrowBasePosition.x, layoutSettings_.arrowBasePosition.y } },
                    { "choiceSpacingY", { layoutSettings_.choiceSpacingY } },
                    { "choiceHitboxOffset", { layoutSettings_.choiceHitboxOffset.x, layoutSettings_.choiceHitboxOffset.y } },
                    { "choiceHitboxSize", { layoutSettings_.choiceHitboxSize.x, layoutSettings_.choiceHitboxSize.y } },
                });
            }
        }
    }

    ImGui::End();
#endif
}

void GameLevelUpController::Reset() {
    active_ = false;
    selection_ = 0;
    animationState_ = AnimationState::Hidden;
    slideOffsetX_ = ScreenUtil::GetClientSize().x;
    pendingAction_ = nullptr;
    navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    currentChoices_.clear();
    confettiParticles_.clear();
    for (auto& sprite : choiceSprites_) {
        sprite.reset();
    }
    for (auto& sprite : iconSprites_) {
        sprite.reset();
    }
}

int32_t GameLevelUpController::PickWeightedOptionIndex(const std::vector<LevelUpOption>& candidateOptions) const {
    float totalWeight = 0.0f;
    for (const auto& option : candidateOptions) {
        totalWeight += option.weight;
    }

    static std::mt19937 mt(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, totalWeight);

    float randomValue = dist(mt);
    float accumulatedWeight = 0.0f;
    for (size_t i = 0; i < candidateOptions.size(); ++i) {
        accumulatedWeight += candidateOptions[i].weight;
        if (randomValue <= accumulatedWeight) {
            return static_cast<int32_t>(i);
        }
    }

    return static_cast<int32_t>(candidateOptions.size() - 1);
}

void GameLevelUpController::ApplyLayout() {
    selection_ = (std::clamp)(selection_, 0, (std::max)(0, static_cast<int32_t>(currentChoices_.size()) - 1));

    if (overlaySprite_) {
        overlaySprite_->SetPosition({ slideOffsetX_, 0.0f });
    }

    if (arrowSprite_) {
        arrowSprite_->SetPosition({layoutSettings_.arrowBasePosition.x + slideOffsetX_,
                                   layoutSettings_.arrowBasePosition.y + static_cast<float>(selection_) * layoutSettings_.choiceSpacingY});
    }

    for (int i = 0; i < 3; ++i) {
        if (!choiceSprites_[i]) {
        } else {
            choiceSprites_[i]->SetPosition({ layoutSettings_.choicePositions[i].x + slideOffsetX_, layoutSettings_.choicePositions[i].y });
            choiceSprites_[i]->SetSize(layoutSettings_.choiceSize);
        }

        if (!iconSprites_[i]) {
            continue;
        }

        iconSprites_[i]->SetPosition({ layoutSettings_.choicePositions[i].x + slideOffsetX_, layoutSettings_.choicePositions[i].y });
        iconSprites_[i]->SetSize(layoutSettings_.choiceSize);
    }
}

void GameLevelUpController::UpdateSlideAnimation(float deltaTime) {
    switch (animationState_) {
    case AnimationState::Entering:
        slideOffsetX_ = (std::max)(0.0f, slideOffsetX_ - kLevelUpSlideSpeed * deltaTime);
        if (slideOffsetX_ <= 0.0f) {
            slideOffsetX_ = 0.0f;
            animationState_ = AnimationState::Idle;
        }
        ApplyLayout();
        break;

    case AnimationState::Exiting:
        slideOffsetX_ -= kLevelUpSlideSpeed * deltaTime;
        ApplyLayout();
        break;

    case AnimationState::Idle:
    case AnimationState::Hidden:
        break;
    }
}

void GameLevelUpController::SpawnConfetti() {
    confettiParticles_.clear();

    static std::mt19937 mt(std::random_device{}());
    const Vector2 clientSize = ScreenUtil::GetClientSize();
    const float screenWidth = (std::max)(clientSize.x, 240.0f);
    const float screenHeight = (std::max)(clientSize.y, 240.0f);
    std::uniform_real_distribution<float> spawnXDist(120.0f, screenWidth - 120.0f);
    std::uniform_real_distribution<float> velocityXDist(-180.0f, 180.0f);
    std::uniform_real_distribution<float> velocityYDist(-980.0f, -520.0f);
    std::uniform_real_distribution<float> sizeXDist(8.0f, 18.0f);
    std::uniform_real_distribution<float> sizeYDist(14.0f, 30.0f);
    std::uniform_real_distribution<float> rotationDist(0.0f, 6.28318f);
    std::uniform_real_distribution<float> angularVelocityDist(-7.0f, 7.0f);
    std::uniform_real_distribution<float> lifetimeDist(0.8f, 1.5f);
    std::uniform_int_distribution<int> colorIndexDist(0, 5);

    constexpr KamataEngine::Vector4 kColors[] = {
        {1.0f, 0.35f, 0.35f, 1.0f},
        {1.0f, 0.82f, 0.22f, 1.0f},
        {0.35f, 0.86f, 0.52f, 1.0f},
        {0.30f, 0.72f, 1.0f, 1.0f},
        {0.98f, 0.52f, 0.88f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
    };

    const uint32_t whiteTexture = TextureManager::Load("white1x1.png");
    constexpr int kParticleCount = 28;
    confettiParticles_.reserve(kParticleCount);
    for (int i = 0; i < kParticleCount; ++i) {
        ConfettiParticle particle;
        particle.position = { spawnXDist(mt), screenHeight + 24.0f };
        particle.velocity = { velocityXDist(mt), velocityYDist(mt) };
        particle.size = { sizeXDist(mt), sizeYDist(mt) };
        particle.rotation = rotationDist(mt);
        particle.angularVelocity = angularVelocityDist(mt);
        particle.lifetime = lifetimeDist(mt);
        particle.sprite.reset(Sprite::Create(whiteTexture, particle.position));
        particle.sprite->SetSize(particle.size);
        particle.sprite->SetAnchorPoint({ 0.5f, 0.5f });
        particle.sprite->SetRotation(particle.rotation);
        particle.sprite->SetColor(kColors[colorIndexDist(mt)]);
        confettiParticles_.push_back(std::move(particle));
    }
}

void GameLevelUpController::UpdateConfetti(float deltaTime) {
    for (auto it = confettiParticles_.begin(); it != confettiParticles_.end();) {
        it->age += deltaTime;
        if (it->age >= it->lifetime) {
            it = confettiParticles_.erase(it);
            continue;
        }

        it->velocity.y += kConfettiGravity * deltaTime;
        it->position.x += it->velocity.x * deltaTime;
        it->position.y += it->velocity.y * deltaTime;
        it->rotation += it->angularVelocity * deltaTime;

        if (it->sprite) {
            const float alpha = 1.0f - (it->age / it->lifetime);
            KamataEngine::Vector4 color = it->sprite->GetColor();
            color.w = alpha;
            it->sprite->SetColor(color);
            it->sprite->SetPosition(it->position);
            it->sprite->SetRotation(it->rotation);
        }

        ++it;
    }
}

} // namespace DirectXGame
