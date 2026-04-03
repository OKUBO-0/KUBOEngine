#pragma once
#include <memory>
#include <unordered_map>
#include <functional>
#include "IScene.h"

namespace DirectXGame {

/// <summary>
/// シーンの生成・管理を行うクラス。
/// シーンの登録、切り替え、更新、描画を統括する。
/// </summary>
class SceneManager {
public:
    /// <summary>コンストラクタ（シーン管理用の初期値を設定する）</summary>
    SceneManager();

    /// <summary>デストラクタ（unique_ptrによりリソースは自動解放される）</summary>
    ~SceneManager();

    /// <summary>毎フレーム更新処理（現在のシーンの更新を呼び出す）</summary>
    void Update(float deltaTime);

    /// <summary>描画処理（現在のシーンの描画を呼び出す）</summary>
    void Draw();

    /// <summary>
    /// シーンを登録する
    /// </summary>
    /// <param name="scene">登録するシーン番号</param>
    /// <param name="createFunc">シーン生成用関数（unique_ptrを返す）</param>
    void RegisterScene(
        Scene scene,
        const std::function<std::unique_ptr<IScene>(const std::shared_ptr<GameSessionContext>&)>& createFunc);

    /// <summary>
    /// シーンを切り替える
    /// </summary>
    /// <param name="scene">切り替えるシーン番号</param>
    void ChangeScene(Scene scene);

private:
    std::unordered_map<Scene, std::function<std::unique_ptr<IScene>(const std::shared_ptr<GameSessionContext>&)>> sceneFactory_; ///< シーン生成関数のマップ
    std::unique_ptr<IScene> currentScene_; ///< 現在のシーン（unique_ptrで管理）
    Scene currentSceneNo_ = Scene::Title;  ///< 現在のシーン番号（初期値はタイトル）
    std::shared_ptr<GameSessionContext> sessionContext_;
};

} // namespace DirectXGame
