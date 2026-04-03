#include "SkyDome.h"
using namespace KamataEngine;

namespace DirectXGame {

void SkyDome::Initialize() {
    // カメラ初期化（背景専用カメラを準備）
    camera_.Initialize();

    // モデル生成（天球モデルを読み込み、背景の空を表現）
    skyModel_.reset(Model::CreateFromOBJ("skydome"));

    // ワールド変換初期化（位置・回転・スケールの基準を設定）
    worldTransform_.Initialize();
    
    // スケール拡大（大きさを増加させる）
    worldTransform_.scale_ = { 100.0f, 100.0f, 100.0f };
}

void SkyDome::Update() {
    worldTransform_.UpdateMatrix();
}

void SkyDome::Draw() {
    if (skyModel_) {
        skyModel_->Draw(worldTransform_, camera_);
    }
}

void SkyDome::SetLightGroup(const LightGroup* lightGroup) {
    if (skyModel_) {
        skyModel_->SetLightGroup(lightGroup);
    }
}

} // namespace DirectXGame
