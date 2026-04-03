#include "Fade.h"
using namespace KamataEngine;

namespace DirectXGame {

// 調整値の初期化（cpp側で定義）
const float Fade::kDefaultFadeSpeed = 0.02f;

void Fade::Initialize() {
    // 黒色テクスチャを読み込み、画面全体を覆うスプライトを生成
    uint32_t blackTex = TextureManager::Load(kBlackTexturePath);

    // Sprite::Create が生ポインタを返すため、unique_ptr にラップ
    fadeSprite_.reset(Sprite::Create(blackTex, { 0, 0 }));
    fadeSprite_->SetSize(Vector2(kScreenWidth, kScreenHeight));

    // 初期状態は「フェードイン中」から開始
    alpha_ = 1.0f;
    speed_ = kDefaultFadeSpeed;
    state_ = State::kFadeIn;
    finished_ = false;
}

void Fade::StartFadeIn(float speed) {
    state_ = State::kFadeIn;
    speed_ = speed;
    alpha_ = 1.0f;
    finished_ = false;
}

void Fade::StartFadeOut(float speed) {
    state_ = State::kFadeOut;
    speed_ = speed;
    alpha_ = 0.0f;
    finished_ = false;
}

void Fade::Update(float deltaTime) {
    /// <summary>
    /// 状態に応じてアルファ値を更新する処理
    /// 意図: フェードイン/アウトの進行度を管理し、完了時に状態を切り替える
    /// </summary>
    switch (state_) {
    case State::kFadeIn:
        alpha_ -= speed_ * (deltaTime / 0.016f);
        if (alpha_ <= 0.0f) {
            alpha_ = 0.0f;
            state_ = State::kStay;   // 完全透明後は「表示維持」状態
            finished_ = true;
        }
        break;

    case State::kFadeOut:
        alpha_ += speed_ * (deltaTime / 0.016f);
        if (alpha_ >= 1.0f) {
            alpha_ = 1.0f;
            state_ = State::kNone;   // 完全黒後は「非表示」状態
            finished_ = true;
        }
        break;

    default:
        // kStay や kNone の場合は処理なし
        break;
    }

    // スプライトの色（アルファ値）を更新
    if (fadeSprite_) {
        fadeSprite_->SetColor(Vector4(1, 1, 1, alpha_));
    }
}

void Fade::Draw() {
    /// <summary>
    /// フェードが有効な状態のみ描画する処理
    /// 意図: フェード演出が不要な場合は描画負荷を避ける
    /// </summary>
    if (state_ != State::kNone && fadeSprite_) {
        fadeSprite_->Draw();
    }
}

} // namespace DirectXGame
