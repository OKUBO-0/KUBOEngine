#include "RippleEffect.h"
#include "ModelCache.h"
using namespace KamataEngine;

namespace DirectXGame {

void RippleEffect::Initialize(const Vector3& pos) {
    // モデル生成（波紋の見た目を表現するモデルを読み込み）
    model_ = ModelCache::Get("ripples");

    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();
    objectColor_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    // ワールド変換初期化（位置・回転・スケールの基準を設定）
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos; // 波紋の生成位置を設定

    // 初期スケール設定（小さな状態から始まり、時間経過で拡大する演出）
    worldTransform_.scale_ = { kStartScale, kStartScale, kStartScale };

    // 初期状態の設定
    age_ = 0.0f;    // 経過時間リセット
    alpha_ = 1.0f;  // 完全不透明から開始
    active_ = true; // 有効状態に設定
}

void RippleEffect::Update(float deltaTime) {
    age_ += deltaTime;

    // 寿命判定
    // 意図: 一定時間経過後に波紋を非アクティブ化する
    if (age_ >= kLifetime) {
        active_ = false;
        return;
    }

    // 寿命に対する進行度（0.0 → 1.0）
    float t = age_ / kLifetime;

    // スケール拡大処理
    // 意図: 開始サイズから終了サイズへ線形補間し、波紋が広がる演出を行う
    float s = kStartScale + (kEndScale - kStartScale) * t;
    worldTransform_.scale_ = { s, s, s };

    // 透明度減少処理
    // 意図: 時間経過に応じて透明度を減少させ、波紋が消えていく演出を行う
    alpha_ = 1.0f - t;
    if (objectColor_) {
        objectColor_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
    }

    worldTransform_.UpdateMatrix(); // 行列更新
}

void RippleEffect::Draw(Camera* camera) {
    if (!active_ || !model_) return;
    model_->Draw(worldTransform_, *camera, objectColor_.get());
}

} // namespace DirectXGame
