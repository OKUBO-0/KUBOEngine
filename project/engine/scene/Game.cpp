#include "Game.h"
#include "SceneFactory.h"
#include <imgui.h>

void Game::Initialize()
{
	// 初期化
	Framework::Initialize();
	sceneFactory = std::make_unique<SceneFactory>();
	GetEngineContext().sceneFactory = sceneFactory.get();
	GetEngineContext().sceneManager->SetSceneFactory(GetEngineContext().sceneFactory);
	GetEngineContext().sceneManager->SetServices(GetEngineContext().CreateSceneServices());

	// シーンの変更
	// "TITLE"
	// "GAMEPLAY"
	// "GAMEOVER"
	// "GAMECLEAR"
	GetEngineContext().sceneManager->ChangeScene("GAMEPLAY");
}

void Game::Finalize()
{
	// 終了
	Framework::Finalize();
}

void Game::Update()
{
#ifdef _DEBUG
	GetEngineContext().imGuiManager->Begin();
#endif // _DEBUG
	// 更新
	Framework::Update();

#ifdef _DEBUG
	GetEngineContext().offscreenRenderManager->DrawImGui();
	DrawEngineEditorImGui();
	GetEngineContext().sceneManager->DrawEditorImGui();
	GetEngineContext().imGuiManager->End();
#endif // _DEBUG
}

void Game::Draw()
{
	// DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む
	GetEngineContext().offscreenRenderManager->Begin();
	GetEngineContext().srvManager->PreDraw();
	GetEngineContext().sceneManager->Draw();
	GetEngineContext().offscreenRenderManager->End();

	GetEngineContext().dxCommon->Begin();
	// 描画
	GetEngineContext().offscreenRenderManager->Draw();
#ifdef _DEBUG
	GetEngineContext().imGuiManager->Draw();
#endif // _DEBUG
	GetEngineContext().dxCommon->End();
}

void Game::DrawEngineEditorImGui()
{
	ImGui::Begin("Engine Editor");
	ImGui::Text("Current Scene: %s", GetEngineContext().sceneManager->GetCurrentSceneName().c_str());
	ImGui::Separator();

	if (ImGui::Button("Title")) {
		GetEngineContext().sceneManager->ChangeScene("TITLE");
	}
	ImGui::SameLine();
	if (ImGui::Button("Gameplay")) {
		GetEngineContext().sceneManager->ChangeScene("GAMEPLAY");
	}
	ImGui::SameLine();
	if (ImGui::Button("GameOver")) {
		GetEngineContext().sceneManager->ChangeScene("GAMEOVER");
	}
	ImGui::SameLine();
	if (ImGui::Button("GameClear")) {
		GetEngineContext().sceneManager->ChangeScene("GAMECLEAR");
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Unity-like editor foundation");
	ImGui::BulletText("Hierarchy/Inspector are scene-driven");
	ImGui::BulletText("Post effects are edited in OffscreenRenderManager");
	ImGui::BulletText("Scene switching is centralized here");
	ImGui::Separator();
	ImGui::TextUnformatted("Missing Core Features");
	ImGui::BulletText("Serialized scene save/load");
	ImGui::BulletText("Prefab and reusable object templates");
	ImGui::BulletText("Parent-child transform hierarchy");
	ImGui::BulletText("Component attach/remove workflow");
	ImGui::BulletText("Asset browser and drag-and-drop");
	ImGui::BulletText("Undo / Redo");
	ImGui::BulletText("Physics and collider authoring");
	ImGui::BulletText("Animation controller/state machine");
	ImGui::BulletText("Scene gizmos and viewport tools");
	ImGui::BulletText("Play mode state separation");
	ImGui::Separator();
	ImGui::TextUnformatted("Imported Project");
	ImGui::BulletText("Raw import path: imported/DirectXGame");
	ImGui::BulletText("Status: source and assets copied, not build-integrated");
	ImGui::BulletText("Primary blocker: KamataEngine API dependency");
	ImGui::BulletText("Ported now: GameSessionContext + InputBindings");
	ImGui::BulletText("Stored pending: GameScene / Player / Enemy / UI / World / Effects");
	ImGui::End();
}
