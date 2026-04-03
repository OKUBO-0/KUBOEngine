#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "SceneServices.h"
#include <memory>

class SceneManager
{
public:
	//シーンの設定
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

	//現在のシーンを取得
	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { this->nextScene = std::move(nextScene); };
	//シーンの更新
	void Update();
	//シーンの描画
	void Draw();
	void DrawEditorImGui();
	//シーンの終了
	void Finalize();

	//sceneFactoryの設定
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { this->sceneFactory = sceneFactory; }
	void SetServices(SceneServices services) { services_ = services; }

	void ChangeScene(const std::string &sceneName);
	const BaseScene* GetCurrentScene() const { return currentScene.get(); }
	BaseScene* GetCurrentScene() { return currentScene.get(); }
	const std::string& GetCurrentSceneName() const { return currentSceneName_; }
	
private:
	std::unique_ptr<BaseScene> currentScene;
	std::unique_ptr<BaseScene> nextScene;
	AbstractSceneFactory* sceneFactory = nullptr;
	SceneServices services_{};
	std::string currentSceneName_;

};

