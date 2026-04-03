#include "ResultScene.h"
#include "../../core/InputBindings.h"
#include "../../ui/common/UILayoutIO.h"
#include <algorithm>
using namespace KamataEngine;

namespace DirectXGame {

namespace {

const char* kResultLayoutPath = "Resources/data/ui_layout_result.csv";

}

void ResultScene::Initialize() {
    // --- 各種シングルトン取得（描画・入力・音声） ---
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    audio_ = Audio::GetInstance();

    selectSEHandle_ = audio_->LoadWave("audio/se/se_pause.wav");
    countupSEHandle_ = audio_->LoadWave("audio/se/se_pause.wav");

    // --- 背景スプライト生成（黒背景） ---
    uint32_t blackTex = TextureManager::Load("textures/color/black.png");
    backgroundSprite_ = std::unique_ptr<Sprite>(Sprite::Create(blackTex, { 0,0 }));

    // --- リザルト画面用スプライト生成 ---
    uint32_t resultTex = TextureManager::Load("ui/result/Result.png");
    resultSprite_ = std::unique_ptr<Sprite>(Sprite::Create(resultTex, { 0,0 }));

    // --- 終了UIスプライト生成 ---
    uint32_t uiTex = TextureManager::Load("ui/result/finish_ui.png");
    resultUI_ = std::unique_ptr<Sprite>(Sprite::Create(uiTex, { 0,0 }));

    // 経験値
    expUI_ = std::make_unique<Score>();
    expUI_->Initialize();
    expUI_->SetNumber(0);

    // レベル
    levelUI_ = std::make_unique<Score>();
    levelUI_->Initialize();
    levelUI_->SetNumber(sessionContext_ ? sessionContext_->resultData.finalLevel : 0);

    // キル数
    killUI_ = std::make_unique<Score>();
    killUI_->Initialize();
    killUI_->SetNumber(sessionContext_ ? sessionContext_->resultData.totalKillCount : 0);

    // --- スコア演出用変数 ---
    currentExp_ = 0;
    targetExp_ = sessionContext_ ? sessionContext_->resultData.totalExp : 0;

    currentLevel_ = 0;
    targetLevel_ = sessionContext_ ? sessionContext_->resultData.finalLevel : 0;

    currentKill_ = 0;
    targetKill_ = sessionContext_ ? sessionContext_->resultData.totalKillCount : 0;

    expUI_->SetNumber(0);
    levelUI_->SetNumber(0);
    killUI_->SetNumber(0);

    // --- カーテン初期化 ---
    curtain_.Initialize();
    curtain_.StartOpen(20.0f);
    curtainOpening_ = true;
    {
        const auto layout = UILayoutIO::Load(kResultLayoutPath);
        if (const auto it = layout.find("backgroundPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.backgroundPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("backgroundSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.backgroundSize = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("resultPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.resultPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("resultSize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.resultSize = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("resultUIPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.resultUIPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("resultUISize"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.resultUISize = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("expPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.expPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("levelPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.levelPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("killPosition"); it != layout.end() && it->second.size() >= 2) {
            layoutSettings_.killPosition = { it->second[0], it->second[1] };
        }
        if (const auto it = layout.find("scoreScale"); it != layout.end() && !it->second.empty()) {
            layoutSettings_.scoreScale = it->second[0];
        }
    }
    ApplyLayout();
}

void ResultScene::Update(float deltaTime) {
    // --- カーテン更新（常に先頭で処理） ---
    curtain_.Update(deltaTime);
    if (countupSECooldown_ > 0) {
        --countupSECooldown_;
    }

    if (curtainOutStarted_) {
        if (curtain_.IsFinished()) {
            finished_ = true;
        }
        DrawDebugUI();
        return;
    }

    if (curtainOpening_) {
        if (curtain_.GetState() == CurtainTransition::State::kNone) {
            curtainOpening_ = false;
        }
        DrawDebugUI();
        return;
    }

    // --- スコア加算演出（徐々に最終スコアまで増加させる） ---
    if (currentExp_ < targetExp_) {
        currentExp_ += (std::max<int32_t>)(1, (targetExp_ - currentExp_) / 12);
        currentExp_ = (std::min)(currentExp_, targetExp_);
        expUI_->SetNumber(currentExp_);
        if (countupSEHandle_ != 0 && countupSECooldown_ == 0) {
            audio_->PlayWave(countupSEHandle_, false, 0.25f);
            countupSECooldown_ = 6;
        }
    }

    // --- レベル加算演出 ---
    if (currentLevel_ < targetLevel_) {
        currentLevel_ += (std::max<int32_t>)(1, (targetLevel_ - currentLevel_) / 8);
        currentLevel_ = (std::min)(currentLevel_, targetLevel_);
        levelUI_->SetNumber(currentLevel_);
        if (countupSEHandle_ != 0 && countupSECooldown_ == 0) {
            audio_->PlayWave(countupSEHandle_, false, 0.25f);
            countupSECooldown_ = 6;
        }
    }

    // --- キル数加算演出 ---
    if (currentKill_ < targetKill_) {
        currentKill_ += (std::max<int32_t>)(1, (targetKill_ - currentKill_) / 8);
        currentKill_ = (std::min)(currentKill_, targetKill_);
        killUI_->SetNumber(currentKill_);
        if (countupSEHandle_ != 0 && countupSECooldown_ == 0) {
            audio_->PlayWave(countupSEHandle_, false, 0.25f);
            countupSECooldown_ = 6;
        }
    }

    expUI_->Update();
    levelUI_->Update();
    killUI_->Update();

    // --- SPACE / ENTER で演出スキップ・タイトル復帰 ---
    const bool canSkipCountUp = currentExp_ < targetExp_ || currentLevel_ < targetLevel_ || currentKill_ < targetKill_;
    if (InputBindings::IsConfirmTriggered(input_) && canSkipCountUp) {
        currentExp_ = targetExp_;
        currentLevel_ = targetLevel_;
        currentKill_ = targetKill_;
        expUI_->SetNumber(currentExp_);
        levelUI_->SetNumber(currentLevel_);
        killUI_->SetNumber(currentKill_);
        DrawDebugUI();
        return;
    }

    if (InputBindings::IsConfirmTriggered(input_) &&
        curtain_.GetState() == CurtainTransition::State::kNone) {
        if (selectSEHandle_ != 0) {
            audio_->PlayWave(selectSEHandle_, false, 1.0f);
        }
        curtain_.StartClose();
        curtainOutStarted_ = true;
        SetSceneNo(Scene::Title);
    }

    DrawDebugUI();
}

void ResultScene::Draw() {
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    // --- スプライト描画開始 ---
    Sprite::PreDraw(dxCommon->GetCommandList());

    // --- 背景・リザルト画面・UIを描画 ---
    backgroundSprite_->Draw();
    resultSprite_->Draw();
    resultUI_->Draw();

    // --- スコア描画（加算演出で更新された値を表示） ---
    if (expUI_) {
        expUI_->Draw();
    }
    if (levelUI_) {
        levelUI_->Draw();
	}
    if (killUI_) {
		killUI_->Draw();
    }

    // --- カーテン描画（シーン遷移演出） ---
    curtain_.Draw();

    // --- スプライト描画終了 ---
    Sprite::PostDraw();
}

void ResultScene::Finalize() {
    // --- 特別な終了処理は不要（リソース解放はデストラクタで対応） ---
}

void ResultScene::ApplyLayout() {
    backgroundSprite_->SetPosition(layoutSettings_.backgroundPosition);
    backgroundSprite_->SetSize(layoutSettings_.backgroundSize);
    resultSprite_->SetPosition(layoutSettings_.resultPosition);
    resultSprite_->SetSize(layoutSettings_.resultSize);
    resultUI_->SetPosition(layoutSettings_.resultUIPosition);
    resultUI_->SetSize(layoutSettings_.resultUISize);

    expUI_->SetPosition(layoutSettings_.expPosition);
    expUI_->SetScale(layoutSettings_.scoreScale);
    levelUI_->SetPosition(layoutSettings_.levelPosition);
    levelUI_->SetScale(layoutSettings_.scoreScale);
    killUI_->SetPosition(layoutSettings_.killPosition);
    killUI_->SetScale(layoutSettings_.scoreScale);
}

void ResultScene::DrawDebugUI() {
#ifdef _DEBUG
    if (!ImGui::Begin("UI Debug")) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Result", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable Result Debug", &layoutSettings_.debugEnabled);
        if (layoutSettings_.debugEnabled) {
            float resultPosition[2]{ layoutSettings_.resultPosition.x, layoutSettings_.resultPosition.y };
            if (ImGui::DragFloat2("Result Position", resultPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.resultPosition = { resultPosition[0], resultPosition[1] };
                ApplyLayout();
            }

            float resultUIPosition[2]{ layoutSettings_.resultUIPosition.x, layoutSettings_.resultUIPosition.y };
            if (ImGui::DragFloat2("Result UI Position", resultUIPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.resultUIPosition = { resultUIPosition[0], resultUIPosition[1] };
                ApplyLayout();
            }

            float expPosition[2]{ layoutSettings_.expPosition.x, layoutSettings_.expPosition.y };
            if (ImGui::DragFloat2("EXP Position", expPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.expPosition = { expPosition[0], expPosition[1] };
                ApplyLayout();
            }

            float levelPosition[2]{ layoutSettings_.levelPosition.x, layoutSettings_.levelPosition.y };
            if (ImGui::DragFloat2("Level Position", levelPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.levelPosition = { levelPosition[0], levelPosition[1] };
                ApplyLayout();
            }

            float killPosition[2]{ layoutSettings_.killPosition.x, layoutSettings_.killPosition.y };
            if (ImGui::DragFloat2("Kill Position", killPosition, 1.0f, -400.0f, 1280.0f)) {
                layoutSettings_.killPosition = { killPosition[0], killPosition[1] };
                ApplyLayout();
            }

            if (ImGui::DragFloat("Score Scale", &layoutSettings_.scoreScale, 0.05f, 0.5f, 6.0f)) {
                ApplyLayout();
            }

            if (ImGui::Button("Save Result Layout")) {
                UILayoutIO::Save(kResultLayoutPath, {
                    { "backgroundPosition", { layoutSettings_.backgroundPosition.x, layoutSettings_.backgroundPosition.y } },
                    { "backgroundSize", { layoutSettings_.backgroundSize.x, layoutSettings_.backgroundSize.y } },
                    { "resultPosition", { layoutSettings_.resultPosition.x, layoutSettings_.resultPosition.y } },
                    { "resultSize", { layoutSettings_.resultSize.x, layoutSettings_.resultSize.y } },
                    { "resultUIPosition", { layoutSettings_.resultUIPosition.x, layoutSettings_.resultUIPosition.y } },
                    { "resultUISize", { layoutSettings_.resultUISize.x, layoutSettings_.resultUISize.y } },
                    { "expPosition", { layoutSettings_.expPosition.x, layoutSettings_.expPosition.y } },
                    { "levelPosition", { layoutSettings_.levelPosition.x, layoutSettings_.levelPosition.y } },
                    { "killPosition", { layoutSettings_.killPosition.x, layoutSettings_.killPosition.y } },
                    { "scoreScale", { layoutSettings_.scoreScale } },
                });
            }
        }
    }

    ImGui::End();
#endif
}

} // namespace DirectXGame
