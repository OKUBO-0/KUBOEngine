#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include "SceneServices.h"
#include "ImGuiManager.h"
#include <imgui.h>


void TitleScene::Initialize()
{
	
	
	
}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{
	if (GetServices().input->TriggerKey(DIK_SPACE)) {
		GetSceneManager()->ChangeScene("GAMEPLAY");
		
	}

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("titleScene %d");
		if (ImGui::Button("gamePlayScene"))
		{
			GetSceneManager()->ChangeScene("GAMEPLAY");
		}
		


	}

}

void TitleScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	GetServices().object3DCommon->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	GetServices().spriteCommon->CommonDraw();

}
