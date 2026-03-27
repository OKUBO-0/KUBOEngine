#include "GameClearScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "SceneManager.h"
#include "SceneServices.h"
#include "CameraManager.h"

void GameClearScene::Initialize()
{
}

void GameClearScene::Finalize()
{
}

void GameClearScene::Update()
{
	GetServices().cameraManager->GetActiveCamera()->Update();

#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gameClearScene");
		if (ImGui::Button("TitleScene"))
		{
			GetSceneManager()->ChangeScene("TITLE");
		}
	}
#endif // _DEBUG
}

void GameClearScene::Draw()
{
#pragma region 3Dオブジェクト描画
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	GetServices().object3DCommon->CommonDraw();
#pragma endregion

#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	GetServices().spriteCommon->CommonDraw();
#pragma endregion
}
