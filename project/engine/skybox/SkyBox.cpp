#include "SkyBox.h"
#include "TextureManager.h"
#include "CameraManager.h"
#include <imgui.h>

SkyBox::~SkyBox()
{

}

void SkyBox::Initialize(SkyBoxCommon* skyBoxCommon, std::string textureFilePath)
{
	skyBoxCommon_ = skyBoxCommon;
	dxCommon_ = skyBoxCommon_->GetDxCommon();// DirectXの共通処理クラス
	srvManager_ = skyBoxCommon_->GetSrvManager();// SRV管理クラス
	textureFilePath_ = textureFilePath;// テクスチャファイルパス


	vertices.resize(24);
	//頂点データを初期化
	// 右面。描画インデックスは[0,1,2][2,1,3]で内側を向く
	vertices[0].position = { 1.0f,  1.0f, -1.0f, 1.0f };
	vertices[1].position = { 1.0f,  1.0f,  1.0f, 1.0f };
	vertices[2].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertices[3].position = { 1.0f, -1.0f,  1.0f, 1.0f };

	// 左面。描画インデックスは[4,5,6][6,5,7]
	vertices[4].position = { -1.0f,  1.0f,  1.0f, 1.0f };
	vertices[5].position = { -1.0f,  1.0f, -1.0f, 1.0f };
	vertices[6].position = { -1.0f, -1.0f,  1.0f, 1.0f };
	vertices[7].position = { -1.0f, -1.0f, -1.0f, 1.0f };

	// 前面。描画インデックスは[8,9,10][10,9,11]
	vertices[8].position = { -1.0f,  1.0f,  1.0f, 1.0f };
	vertices[9].position = { 1.0f,  1.0f,  1.0f, 1.0f };
	vertices[10].position = { -1.0f, -1.0f,  1.0f, 1.0f };
	vertices[11].position = { 1.0f, -1.0f,  1.0f, 1.0f };

	// 後面。描画インデックスは[12,13,14][14,13,15]
	vertices[12].position = { 1.0f,  1.0f, -1.0f, 1.0f };
	vertices[13].position = { -1.0f,  1.0f, -1.0f, 1.0f };
	vertices[14].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertices[15].position = { -1.0f, -1.0f, -1.0f, 1.0f };

	// 上面。描画インデックスは[16,17,18][18,17,19]
	vertices[16].position = { -1.0f,  1.0f, -1.0f, 1.0f };
	vertices[17].position = { 1.0f,  1.0f, -1.0f, 1.0f };
	vertices[18].position = { -1.0f,  1.0f,  1.0f, 1.0f };
	vertices[19].position = { 1.0f,  1.0f,  1.0f, 1.0f };

	// 下面。描画インデックスは[20,21,22][22,21,23]
	vertices[20].position = { -1.0f, -1.0f,  1.0f, 1.0f };
	vertices[21].position = { 1.0f, -1.0f,  1.0f, 1.0f };
	vertices[22].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertices[23].position = { 1.0f, -1.0f, -1.0f, 1.0f };


	indices = {
		// 右面
		0, 1, 2, 2, 1, 3,
		// 左面
		4, 5, 6, 6, 5, 7,
		// 前面
		8, 9,10,10, 9,11,
		// 後面
	   12,13,14,14,13,15,
	   // 上面
	  16,17,18,18,17,19,
	  // 下面
	 20,21,22,22,21,23,
	};


	vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertices.size());
	// リソースアドレスを設定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	// GPUにデータ転送
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices.data(), sizeof(VertexData) * vertices.size());
	vertexResource->Unmap(0, nullptr);
	//インデックスバッファビューを設定
	indexResource = dxCommon_->CreateBufferResource(sizeof(uint16_t) * indices.size());
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = static_cast<UINT>(sizeof(uint16_t) * indices.size());
	// GPU転送
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices.data(), sizeof(uint16_t) * indices.size());
	indexResource->Unmap(0, nullptr);

	//テクスチャ
	//テクスチャファイルを読み込んでSRVを取得
	skyBoxCommon_->GetTextureManager()->LoadTexture(textureFilePath_);//テクスチャファイルの読み込み
	//SRVのインデックスを取得
	textureIndex_ = skyBoxCommon_->GetTextureManager()->GetTextureIndexByFilePath(textureFilePath_);	//テクスチャ番号の取得

	//パーティクルグループのマテリアルデータを初期化
	//マテリアル
	materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
	// マテリアルデータをマップ
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// マテリアルデータを初期化
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;//有効にするか否か
	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();

	//transform
	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = skyBoxCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込む

	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();
	transformaitionMatrixData->worldInberseTranspose = transformaitionMatrixData->worldInberseTranspose.MakeIdentity4x4();


	//Trandform変数
	transform = { {30.0f,30.0f,30.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

}

void SkyBox::Update()
{
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Camera* activeCamera = skyBoxCommon_->GetCameraManager()->GetActiveCamera();

	if (activeCamera) {

		const Matrix4x4& viewProjectionMatrix = activeCamera->GetViewprojectionMatrix();
		worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;
		transformaitionMatrixData->WVP = worldViewProjectionMatrix;
		transformaitionMatrixData->World = worldMatrix;

	} else {
		worldViewProjectionMatrix = worldMatrix;
		transformaitionMatrixData->WVP = worldViewProjectionMatrix;
		transformaitionMatrixData->World = worldMatrix;
	}
}

void SkyBox::Draw()
{
	skyBoxCommon_->commonDraw();//共通描画処理を呼び出す
	//頂点バッファビューをセット
	skyBoxCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	////Materialをセット
	skyBoxCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//トランスフォームをセット
	skyBoxCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//テクスチャをセット
	skyBoxCommon_->GetSrvManager()->SetGraficsRootDescriptorTable(2, skyBoxCommon_->GetTextureManager()->GetTextureIndexByFilePath(textureFilePath_));
	//インデックスバッファビューをセット
	skyBoxCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	//描画
	skyBoxCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
}

void SkyBox::imguidebug()
{
#ifdef _DEBUG

	// ImGuiのデバッグウィンドウを表示
	if (ImGui::Begin("SkyBox Debug")) {

		//transformの調整
		ImGui::Text("SkyBox Transform");
		ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f, 0.0f, 10.0f);




		ImGui::End();
	}




#endif // _DEBUG
}
