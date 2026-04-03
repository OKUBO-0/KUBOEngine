#include "GameStartController.h"
#include "../../core/InputBindings.h"
#include "../../core/ScreenUtil.h"

using namespace KamataEngine;

namespace DirectXGame {

void GameStartController::Initialize() {
    uint32_t textureHandle = TextureManager::Load("ui/game/start.png");
    overlaySprite_ = std::unique_ptr<Sprite>(Sprite::Create(textureHandle, {0, 0}));
    overlaySprite_->SetSize(ScreenUtil::GetClientSize());
    waiting_ = true;
}

bool GameStartController::Update(Input* input, Audio* audio, uint32_t startSEHandle) {
    if (!waiting_) {
        return false;
    }

    if (InputBindings::IsConfirmTriggered(input)) {
        waiting_ = false;
        if (startSEHandle != 0) {
            audio->PlayWave(startSEHandle, false, 1.0f);
        }
    }

    return waiting_;
}

void GameStartController::Draw() const {
    if (waiting_ && overlaySprite_) {
        overlaySprite_->Draw();
    }
}

void GameStartController::Reset() {
    waiting_ = true;
}

} // namespace DirectXGame
