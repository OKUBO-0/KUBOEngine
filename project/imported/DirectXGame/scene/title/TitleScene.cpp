#include "TitleScene.h"
#include "../../core/InputBindings.h"
#include "../../ui/common/UILayoutIO.h"
#include <cmath>

using namespace KamataEngine;

namespace DirectXGame {

namespace {

const char* kTitleLayoutPath = "Resources/data/ui_layout_title.csv";

bool IsPointInRect(const Vector2& point, const Vector2& rectPosition, const Vector2& rectSize) {
    return point.x >= rectPosition.x && point.x <= rectPosition.x + rectSize.x &&
           point.y >= rectPosition.y && point.y <= rectPosition.y + rectSize.y;
}

int32_t GetHoveredTitleMenuIndex(Input* input,
                                 const Vector2& hitboxPosition,
                                 const Vector2& hitboxSize,
                                 float hitboxStepY) {
    if (!input) {
        return -1;
    }

    const Vector2 mousePosition = input->GetMousePosition();
    for (int32_t i = 0; i < 3; ++i) {
        const Vector2 rectPosition{
            hitboxPosition.x,
            hitboxPosition.y + hitboxStepY * static_cast<float>(i),
        };
        if (IsPointInRect(mousePosition, rectPosition, hitboxSize)) {
            return i;
        }
    }

    return -1;
}

}

void TitleScene::Initialize() {
    // --- 各種シングルトン取得（描画・入力・音声） ---
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    audio_ = Audio::GetInstance();
    InitializeLighting();

    // --- タイトル画面用BGMと効果音を読み込み ---
    titleBGMHandle_ = audio_->LoadWave("audio/bgm/title.wav");
    selectSEHandle_ = audio_->LoadWave("audio/se/se_pause.wav");
    decideSEHandle_ = audio_->LoadWave("audio/se/se_exp.wav");

    // --- タイトルロゴスプライト生成 ---
    uint32_t titleTex = TextureManager::Load("ui/title/title.png");
    titleSprite_ = std::unique_ptr<Sprite>(Sprite::Create(titleTex, { 0, 0 }));

    // --- カーソルスプライト生成 ---
    uint32_t cursorTex = TextureManager::Load("ui/title/cursor.png");
    cursorSprite_ = std::unique_ptr<Sprite>(Sprite::Create(cursorTex, { 0, 0 }));
    cursorSprite_->SetSize({ 1280, 720 });

    // ガイド画面
    uint32_t guideUITex = TextureManager::Load("ui/title/guideUI.png");
    guideSprite_ = std::unique_ptr<Sprite>(Sprite::Create(guideUITex, { 0, 0 }));
	guideSprite_->SetSize({1280, 720});
    guideSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});

    // モデル生成（天球モデルを読み込み、背景の空を表現）
    playerModel_.reset(Model::CreateFromOBJ("octopus"));
    if (lightGroup_) {
        playerModel_->SetLightGroup(lightGroup_.get());
    }

    // 天球生成
	skyDome_ = std::make_unique<SkyDome>();
	skyDome_->Initialize();
    if (lightGroup_) {
        skyDome_->SetLightGroup(lightGroup_.get());
    }

    // ワールド変換初期化
    worldTransform_.Initialize();
    // モデルの向き（反対方向）
    worldTransform_.rotation_ = { 0.0f, -2.618f, 0.0f };
    // モデルの大きさと位置
    worldTransform_.scale_ = layoutSettings_.modelScale;
    worldTransform_.translation_ = layoutSettings_.modelBasePosition;
    // 行列更新
    worldTransform_.UpdateMatrix();

    // --- 遷移演出用変数の初期化 ---
    curtain_.Initialize();
    curtain_.StartOpen(20.0f);
    curtainOpening_ = true;

    // --- カメラ設定 --- 
    camera_.Initialize();
    camera_.rotation_ = { 0.0f, 0.0f, 0.0f }; 
    camera_.translation_ = { 0.0f, 0.0f, -50.0f }; 
    camera_.UpdateMatrix();
    {
        const auto layout = UILayoutIO::Load(kTitleLayoutPath);
        if (const auto it = layout.find("titlePosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.titlePosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("titleSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.titleSize = { it->second[0], it->second[1] };
        }
    if (const auto it = layout.find("cursorBasePosition"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.cursorBasePosition = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("cursorSize"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.cursorSize = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("cursorStepY"); it != layout.end() && !it->second.empty()) {
        layoutSettings_.cursorStepY = it->second[0];
    }
        if (const auto it = layout.find("menuHitboxPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.menuHitboxPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("menuHitboxSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.menuHitboxSize = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("menuHitboxStepY"); it != layout.end() && !it->second.empty()) {
            layoutSettings_.menuHitboxStepY = it->second[0];
        }
        if (const auto it = layout.find("guidePosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.guidePosition = { it->second[0], it->second[1] };
        }
    if (const auto it = layout.find("guideSize"); it != layout.end() && it->second.size() >= 2) {
        layoutSettings_.guideSize = { it->second[0], it->second[1] };
    }
    if (const auto it = layout.find("modelBasePosition"); it != layout.end() && it->second.size() >= 3) {
        layoutSettings_.modelBasePosition = { it->second[0], it->second[1], it->second[2] };
    }
    if (const auto it = layout.find("modelScale"); it != layout.end() && it->second.size() >= 3) {
        layoutSettings_.modelScale = { it->second[0], it->second[1], it->second[2] };
    }
    }

    ApplyLayout();
}

void TitleScene::InitializeLighting() {
    lightGroup_.reset(LightGroup::Create());
    if (!lightGroup_) {
        return;
    }

    lightGroup_->SetAmbientColor({0.48f, 0.48f, 0.52f});
    lightGroup_->SetDirLightActive(0, true);
    lightGroup_->SetDirLightDir(0, {-0.35f, -1.0f, -0.4f});
    lightGroup_->SetDirLightColor(0, {1.05f, 1.0f, 0.95f});
    lightGroup_->SetDirLightActive(1, false);
    lightGroup_->SetDirLightActive(2, false);
    lightGroup_->Update();
}

void TitleScene::Update(float deltaTime) {
    if (lightGroup_) {
        lightGroup_->Update();
    }

    // --- 遷移演出更新（常に先頭で処理） ---
    curtain_.Update(deltaTime);

    if (curtainStarted_) {
        if (curtain_.IsFinished()) {
            audio_->StopWave(titleBGMHandle_);
            finished_ = true;
        }
        DrawDebugUI();
        return;
    }

    // カーテン開き中は操作禁止
    if (curtainOpening_) {
        if (curtain_.GetState() == CurtainTransition::State::kNone) {
            curtainOpening_ = false;
        }
        DrawDebugUI();
        return;
    }

    // --- BGM再生（ループ再生、未再生なら開始） ---
    if (!audio_->IsPlaying(titleBGMHandle_)) {
        titleBGMHandle_ = audio_->PlayWave(titleBGMHandle_, true, 0.1f);
    }

    // ガイド表示中は ESC で閉じる
    if (guideTransitionState_ != GuideTransitionState::None) {
        const float deltaAlpha = deltaTime * kGuideFadeSpeed_;
        if (guideTransitionState_ == GuideTransitionState::FadeIn) {
            guideAlpha_ += deltaAlpha;
            if (guideAlpha_ >= 1.0f) {
                guideAlpha_ = 1.0f;
                guideTransitionState_ = GuideTransitionState::None;
                guideActive_ = true;
            }
        } else {
            guideAlpha_ -= deltaAlpha;
            if (guideAlpha_ <= 0.0f) {
                guideAlpha_ = 0.0f;
                guideTransitionState_ = GuideTransitionState::None;
                guideActive_ = false;
            }
        }
        guideSprite_->SetColor({1.0f, 1.0f, 1.0f, guideAlpha_});
        DrawDebugUI();
        return;
    }

    const int32_t hoveredMenuIndex = GetHoveredTitleMenuIndex(
        input_, layoutSettings_.menuHitboxPosition, layoutSettings_.menuHitboxSize, layoutSettings_.menuHitboxStepY);

    // 選択中デバイスは固定しつつ、明示入力があったときだけ別デバイスへ切り替える。
    const bool mouseNavigationTriggered = hoveredMenuIndex >= 0 && InputBindings::HasMouseNavigationInput(input_);

    if (InputBindings::IsGamepadMenuUpTriggered(input_) || InputBindings::IsGamepadMenuDownTriggered(input_) ||
               InputBindings::IsGamepadConfirmTriggered(input_) || InputBindings::IsGamepadCancelTriggered(input_)) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Gamepad;
    } else if (InputBindings::IsKeyboardMenuUpTriggered(input_) || InputBindings::IsKeyboardMenuDownTriggered(input_) ||
               InputBindings::IsKeyboardConfirmTriggered(input_) || InputBindings::IsKeyboardCancelTriggered(input_)) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Keyboard;
    } else if (mouseNavigationTriggered) {
        navigationInputDevice_ = InputBindings::NavigationInputDevice::Mouse;
    }

    if (guideActive_) {
        bool closeGuide = false;
        switch (navigationInputDevice_) {
        case InputBindings::NavigationInputDevice::Mouse:
            closeGuide = input_->IsTriggerMouse(0);
            break;
        case InputBindings::NavigationInputDevice::Gamepad:
            closeGuide = InputBindings::IsGamepadCancelTriggered(input_);
            break;
        case InputBindings::NavigationInputDevice::Keyboard:
            closeGuide = InputBindings::IsKeyboardCancelTriggered(input_);
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
        DrawDebugUI();
        return; // メニュー操作は無効
    }

    // --- メニュー選択（W / S） ---
    int32_t previousMenuIndex = menuIndex_;
    if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Mouse) {
        if (hoveredMenuIndex >= 0) {
            menuIndex_ = hoveredMenuIndex;
        }
    } else if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Gamepad) {
        if (InputBindings::IsGamepadMenuUpTriggered(input_)) {
            menuIndex_ = std::max<int32_t>(0, menuIndex_ - 1);
        }
        if (InputBindings::IsGamepadMenuDownTriggered(input_)) {
            menuIndex_ = std::min<int32_t>(2, menuIndex_ + 1);
        }
    } else if (navigationInputDevice_ == InputBindings::NavigationInputDevice::Keyboard) {
        if (InputBindings::IsKeyboardMenuUpTriggered(input_)) {
            menuIndex_ = std::max<int32_t>(0, menuIndex_ - 1);
        }
        if (InputBindings::IsKeyboardMenuDownTriggered(input_)) {
            menuIndex_ = std::min<int32_t>(2, menuIndex_ + 1);
        }
    }
    if (menuIndex_ != previousMenuIndex) {
        if (selectSEHandle_ != 0) {
            audio_->PlayWave(selectSEHandle_, false, 1.0f);
        }
    }

    // --- カーソル位置更新 ---
    cursorSprite_->SetPosition({ layoutSettings_.cursorBasePosition.x,
                                 layoutSettings_.cursorBasePosition.y + layoutSettings_.cursorStepY * static_cast<float>(menuIndex_) });

    // --- 決定（SPACE / ENTER） ---
    bool confirmTriggered = false;
    switch (navigationInputDevice_) {
    case InputBindings::NavigationInputDevice::Mouse:
        confirmTriggered = input_->IsTriggerMouse(0);
        break;
    case InputBindings::NavigationInputDevice::Gamepad:
        confirmTriggered = InputBindings::IsGamepadConfirmTriggered(input_);
        break;
    case InputBindings::NavigationInputDevice::Keyboard:
        confirmTriggered = InputBindings::IsKeyboardConfirmTriggered(input_);
        break;
    case InputBindings::NavigationInputDevice::None:
        break;
    }

    if (confirmTriggered) {
        if (decideSEHandle_ != 0) {
            audio_->PlayWave(decideSEHandle_, false, 1.0f);
        }
        switch (menuIndex_) {
        case 0: // Play
            if (curtain_.GetState() == CurtainTransition::State::kNone) {
                curtain_.StartClose();
                curtainStarted_ = true;
                SetSceneNo(Scene::Game);
            }
            break;

        case 1: // Guide
            guideAlpha_ = 0.0f;
            guideSprite_->SetColor({1.0f, 1.0f, 1.0f, guideAlpha_});
            guideTransitionState_ = GuideTransitionState::FadeIn;
            break;

        case 2: // Quit
            PostQuitMessage(0); // Windowsアプリ終了
            break;
        }
    }

    // --- プレイヤーモデルのアニメーション（回転 + 浮遊 + 左右揺れ） ---

    // 時間経過
    animationTime_ += deltaTime;

    // 回転（Y軸）
    worldTransform_.rotation_.y += deltaTime * 0.625f;

    // 上下浮遊（サイン波）
    float baseY = layoutSettings_.modelBasePosition.y;
    float floatY = sinf(animationTime_ * 1.875f) * 1.5f;
    worldTransform_.translation_.y = baseY + floatY;

    // 左右揺れ（サイン波）
    float baseX = layoutSettings_.modelBasePosition.x;
    float floatX = sinf(animationTime_ * 1.25f) * 1.0f;
    worldTransform_.translation_.x = baseX + floatX;

    worldTransform_.translation_.z = layoutSettings_.modelBasePosition.z;

    // 行列更新
    worldTransform_.UpdateMatrix();
    DrawDebugUI();
}

void TitleScene::Draw() {
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    // --- 背景描画 ---
    Sprite::PreDraw(dxCommon->GetCommandList());
    Sprite::PostDraw();

    // --- モデル描画（タイトル演出用） ---
    dxCommon_->ClearDepthBuffer();
    Model::PreDraw();
    if (playerModel_) {
        playerModel_->Draw(worldTransform_, camera_);
    }
	skyDome_->Draw();
    Model::PostDraw();

    // --- UI・フェード描画 ---
    Sprite::PreDraw(dxCommon->GetCommandList());
    titleSprite_->Draw();
    // ガイド画面表示中
    if (guideActive_ || guideTransitionState_ != GuideTransitionState::None) {
        guideSprite_->Draw();
        curtain_.Draw();
        Sprite::PostDraw();
        return;
    }
    cursorSprite_->Draw();
    curtain_.Draw();
    Sprite::PostDraw();
}

void TitleScene::Finalize() {
    // --- 特別な終了処理は不要（リソース解放はデストラクタで対応） ---
}

void TitleScene::ApplyLayout() {
    titleSprite_->SetPosition(layoutSettings_.titlePosition);
    titleSprite_->SetSize(layoutSettings_.titleSize);
    cursorSprite_->SetSize(layoutSettings_.cursorSize);
    guideSprite_->SetPosition(layoutSettings_.guidePosition);
    guideSprite_->SetSize(layoutSettings_.guideSize);
    cursorSprite_->SetPosition({ layoutSettings_.cursorBasePosition.x,
                                 layoutSettings_.cursorBasePosition.y + layoutSettings_.cursorStepY * static_cast<float>(menuIndex_) });
    worldTransform_.scale_ = layoutSettings_.modelScale;
    worldTransform_.translation_ = layoutSettings_.modelBasePosition;
    worldTransform_.UpdateMatrix();
}

void TitleScene::DrawDebugUI() {
#ifdef _DEBUG
    if (!ImGui::Begin("UI Debug")) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Title", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable Title Debug", &layoutSettings_.debugEnabled);
        if (layoutSettings_.debugEnabled) {
            float titlePosition[2]{ layoutSettings_.titlePosition.x, layoutSettings_.titlePosition.y };
            if (ImGui::DragFloat2("Title Position", titlePosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.titlePosition = { titlePosition[0], titlePosition[1] };
                ApplyLayout();
            }

            float titleSize[2]{ layoutSettings_.titleSize.x, layoutSettings_.titleSize.y };
            if (ImGui::DragFloat2("Title Size", titleSize, 1.0f, 64.0f, 1280.0f)) {
                layoutSettings_.titleSize = { titleSize[0], titleSize[1] };
                ApplyLayout();
            }

            float cursorPosition[2]{ layoutSettings_.cursorBasePosition.x, layoutSettings_.cursorBasePosition.y };
            if (ImGui::DragFloat2("Cursor Base", cursorPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.cursorBasePosition = { cursorPosition[0], cursorPosition[1] };
                ApplyLayout();
            }

            float cursorSize[2]{ layoutSettings_.cursorSize.x, layoutSettings_.cursorSize.y };
            if (ImGui::DragFloat2("Cursor Size", cursorSize, 1.0f, 64.0f, 1280.0f)) {
                layoutSettings_.cursorSize = { cursorSize[0], cursorSize[1] };
                ApplyLayout();
            }

            if (ImGui::DragFloat("Cursor Step", &layoutSettings_.cursorStepY, 1.0f, 16.0f, 320.0f)) {
                ApplyLayout();
            }

            float menuHitboxPosition[2]{ layoutSettings_.menuHitboxPosition.x, layoutSettings_.menuHitboxPosition.y };
            if (ImGui::DragFloat2("Menu Hitbox Pos", menuHitboxPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.menuHitboxPosition = { menuHitboxPosition[0], menuHitboxPosition[1] };
            }

            float menuHitboxSize[2]{ layoutSettings_.menuHitboxSize.x, layoutSettings_.menuHitboxSize.y };
            if (ImGui::DragFloat2("Menu Hitbox Size", menuHitboxSize, 1.0f, 16.0f, 640.0f)) {
                layoutSettings_.menuHitboxSize = { menuHitboxSize[0], menuHitboxSize[1] };
            }

            ImGui::DragFloat("Menu Hitbox Step", &layoutSettings_.menuHitboxStepY, 1.0f, 16.0f, 320.0f);

            float guidePosition[2]{ layoutSettings_.guidePosition.x, layoutSettings_.guidePosition.y };
            if (ImGui::DragFloat2("Guide Position", guidePosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.guidePosition = { guidePosition[0], guidePosition[1] };
                ApplyLayout();
            }

            float guideSize[2]{ layoutSettings_.guideSize.x, layoutSettings_.guideSize.y };
            if (ImGui::DragFloat2("Guide Size", guideSize, 1.0f, 64.0f, 1280.0f)) {
                layoutSettings_.guideSize = { guideSize[0], guideSize[1] };
                ApplyLayout();
            }

            float modelPosition[3]{
                layoutSettings_.modelBasePosition.x,
                layoutSettings_.modelBasePosition.y,
                layoutSettings_.modelBasePosition.z
            };
            if (ImGui::DragFloat3("Model Position", modelPosition, 0.1f, -40.0f, 40.0f)) {
                layoutSettings_.modelBasePosition = { modelPosition[0], modelPosition[1], modelPosition[2] };
                ApplyLayout();
            }

            float modelScale[3]{
                layoutSettings_.modelScale.x,
                layoutSettings_.modelScale.y,
                layoutSettings_.modelScale.z
            };
            if (ImGui::DragFloat3("Model Scale", modelScale, 0.1f, 0.5f, 10.0f)) {
                layoutSettings_.modelScale = { modelScale[0], modelScale[1], modelScale[2] };
                ApplyLayout();
            }

            if (ImGui::Button("Save Title Layout")) {
                UILayoutIO::Save(kTitleLayoutPath, {
                    { "titlePosition", { layoutSettings_.titlePosition.x, layoutSettings_.titlePosition.y } },
                    { "titleSize", { layoutSettings_.titleSize.x, layoutSettings_.titleSize.y } },
                    { "cursorBasePosition", { layoutSettings_.cursorBasePosition.x, layoutSettings_.cursorBasePosition.y } },
                    { "cursorSize", { layoutSettings_.cursorSize.x, layoutSettings_.cursorSize.y } },
                    { "cursorStepY", { layoutSettings_.cursorStepY } },
                    { "menuHitboxPosition", { layoutSettings_.menuHitboxPosition.x, layoutSettings_.menuHitboxPosition.y } },
                    { "menuHitboxSize", { layoutSettings_.menuHitboxSize.x, layoutSettings_.menuHitboxSize.y } },
                    { "menuHitboxStepY", { layoutSettings_.menuHitboxStepY } },
                    { "guidePosition", { layoutSettings_.guidePosition.x, layoutSettings_.guidePosition.y } },
                    { "guideSize", { layoutSettings_.guideSize.x, layoutSettings_.guideSize.y } },
                    { "modelBasePosition", {
                        layoutSettings_.modelBasePosition.x,
                        layoutSettings_.modelBasePosition.y,
                        layoutSettings_.modelBasePosition.z } },
                    { "modelScale", {
                        layoutSettings_.modelScale.x,
                        layoutSettings_.modelScale.y,
                        layoutSettings_.modelScale.z } },
                });
            }
        }
    }

    ImGui::End();
#endif
}

} // namespace DirectXGame
