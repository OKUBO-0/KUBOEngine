#pragma once

#include <KamataEngine.h>
#include <cstdint>
#include <memory>

namespace DirectXGame {

/// <summary>
/// ゲーム開始待ちUIを管理するクラス。
/// 開始オーバーレイの表示と開始入力の受付を担当する。
/// </summary>
class GameStartController {
public:
    /// <summary>
    /// 初期化処理
    /// 目的: 開始案内用のオーバーレイスプライトを生成する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// 目的: SPACE / ENTER の開始入力を監視し、待機状態を解除する。
    /// 引数: input - 入力取得に使う入力管理
    /// 引数: audio - 効果音再生に使う音声管理
    /// 引数: startSEHandle - 開始時に鳴らすSEハンドル
    /// 戻り値: true ならまだ開始待ち中
    /// </summary>
    bool Update(KamataEngine::Input* input, KamataEngine::Audio* audio, uint32_t startSEHandle);

    /// <summary>
    /// 描画処理
    /// 目的: 開始待ちオーバーレイを表示する。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Draw() const;

    /// <summary>
    /// 状態リセット
    /// 目的: 待機フラグを初期状態へ戻す。
    /// 引数: なし
    /// 戻り値: なし
    /// </summary>
    void Reset();

    /// <summary>
    /// 待機状態判定
    /// 目的: ゲーム開始前の待機中かどうかを返す。
    /// 引数: なし
    /// 戻り値: true なら開始待ち中
    /// </summary>
    bool IsWaiting() const { return waiting_; }

private:
    bool waiting_ = true;
    std::unique_ptr<KamataEngine::Sprite> overlaySprite_;
};

} // namespace DirectXGame
