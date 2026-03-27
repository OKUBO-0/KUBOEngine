#include "Game.h"
#include "SceneFactory.h"

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
	GetEngineContext().ofscreenRenderManager->DrawImGui();
	GetEngineContext().imGuiManager->End();
#endif // _DEBUG
}

void Game::Draw()
{
	// DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む
	GetEngineContext().ofscreenRenderManager->Begin();
	GetEngineContext().srvManager->PreDraw();
	GetEngineContext().sceneManager->Draw();
	GetEngineContext().ofscreenRenderManager->End();

	GetEngineContext().dxCommon->Begin();
	// 描画
	GetEngineContext().ofscreenRenderManager->Draw();
#ifdef _DEBUG
	GetEngineContext().imGuiManager->Draw();
#endif // _DEBUG
	GetEngineContext().dxCommon->End();
}
