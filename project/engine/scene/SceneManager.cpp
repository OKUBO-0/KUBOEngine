#include "SceneManager.h"
#include "Logger.h"

void SceneManager::Update()
{

	//シーンの切り替え
	if (nextScene) {
		//旧シーンの終了処理
		if (currentScene) {
			currentScene->Finalize();
		}
		//新シーンの初期化
		currentScene = std::move(nextScene);

		currentScene->SetSceneManager(this);
		currentScene->SetServices(&services_);
		currentSceneName_ = currentScene->GetSceneName();

		//新シーンの初期化
		currentScene->Initialize();
	}

	//現在のシーンの更新
	if (currentScene) {
		currentScene->Update();
	}

}

void SceneManager::Draw()
{
	//現在のシーンの描画
	if (currentScene) {
		currentScene->Draw();
	}
}

void SceneManager::DrawEditorImGui()
{
	if (currentScene) {
		currentScene->DrawEditorImGui();
	}
}

void SceneManager::Finalize()
{
	if (currentScene) {
		currentScene->Finalize();
		currentScene.reset();
	}
	nextScene.reset();
	currentSceneName_.clear();

}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	if (sceneFactory == nullptr) {
		Logger::Log("SceneManager::ChangeScene failed. sceneFactory is null.\n");
		return;
	}

	if (nextScene != nullptr) {
		Logger::Log("SceneManager::ChangeScene ignored. A next scene is already queued.\n");
		return;
	}

	nextScene = sceneFactory->CreateScene(sceneName);
	if (nextScene == nullptr) {
		Logger::Log("SceneManager::ChangeScene failed. Scene creation returned null for: " + sceneName + "\n");
	}

}


