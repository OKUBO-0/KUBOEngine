#pragma once
#include <cstdint>
#include <memory>

#include "GameSessionContext.h"

namespace DirectXGame {

/// <summary>
/// シーンの種類を表す列挙型。
/// Title: タイトルシーン
/// Game: ゲームプレイシーン
/// Result: リザルトシーン
/// </summary>
enum class Scene {
    Title,
    Game,
    Result,
};

/// <summary>
/// シーンの基本インターフェースを定義する抽象クラス。
/// 各シーンはこのクラスを継承し、初期化・更新・描画・終了処理を実装する。
/// </summary>
class IScene {
protected:
    explicit IScene(std::shared_ptr<GameSessionContext> sessionContext)
        : sessionContext_(std::move(sessionContext)) {}

public:
    /// <summary>初期化処理（シーン開始時に必要な準備を行う）</summary>
    virtual void Initialize() = 0;

    /// <summary>毎フレーム更新処理（シーン内の状態を更新する）</summary>
    virtual void Update(float deltaTime) = 0;

    /// <summary>描画処理（シーン内のオブジェクトを画面に描画する）</summary>
    virtual void Draw() = 0;

    /// <summary>終了処理（シーン終了時に必要な後処理を行う）</summary>
    virtual void Finalize() = 0;

    /// <summary>シーンが終了状態かどうかを判定する</summary>
    virtual bool IsFinished() const = 0;

    /// <summary>デストラクタ（派生クラスのリソース解放を保証する）</summary>
    virtual ~IScene();

    /// <summary>次のシーン番号を設定する</summary>
    void SetSceneNo(Scene next) { nextScene_ = next; }

    /// <summary>現在設定されている遷移先シーン番号を取得する</summary>
    Scene GetNextSceneNo() const { return nextScene_; }

protected:
    std::shared_ptr<GameSessionContext> sessionContext_;

private:
    Scene nextScene_ = Scene::Title;
};

} // namespace DirectXGame
