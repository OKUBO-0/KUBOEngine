#include "ParticleMnager.h"
#include <ModelManager.h>
#include <TextureManager.h>
#include "CameraManager.h"
#include <MyMath.h>
#include <numbers>
#include <imgui.h>

void ParticleMnager::Initialize(DirectXCommon* dxcommn, SrvManager* srvmaneger, CameraManager* cameraManager, ModelManager* modelManager, TextureManager* textureManager)
{


	//引数で受け取ったポインタをメンバ変数に代入
	dxCommon_ = dxcommn;
	srvManager_ = srvmaneger;
	cameraManager_ = cameraManager;
	modelManager_ = modelManager;
	textureManager_ = textureManager;
	//乱数エンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateParticle();


	//カメラとモデルのTrandform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

	




}




void ParticleMnager::Finalize()
{
	particleGroups.clear();
	graphicsPipeline_.reset();
	model_ = nullptr;
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
	cameraManager_ = nullptr;
	modelManager_ = nullptr;
	textureManager_ = nullptr;

}


void ParticleMnager::Update()
{
	//カメラからビュープロジェクション行列を取得
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * cameraManager_->GetActiveCamera()->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//ビルボード行列を使ってビルボード行列を計算
	Matrix4x4 viewMatrix = cameraManager_->GetActiveCamera()->GetViewMatrix();
	Matrix4x4 projectionMatrix = cameraManager_->GetActiveCamera()->GetProjextionMatrix();






	//全パーティクル	グループ内の全パーティクルについて二重処理する
	for (auto& [name, particleGroup] : particleGroups) {
		auto& behavior = particleGroup.behavior;
		uint32_t counter = 0;
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {


			//パーティクルの寿命が尽きたらグループから外す

			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}

			behavior->Update((*particleIterator), 1.0f / 60.0f, particleGroup.materialData);
			
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifetime);
			/*float alpha = 1.0f;*/
			//ローテート
			Matrix4x4 rotateMatrix = MyMath::MakeRotateMatrix((*particleIterator).transform.rotate);
			//ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * rotateMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;


			if (counter < particleGroup.instanceCount) {
				particleGroup.instanceData[counter].WVP = worldViewProjetionMatrix;
				particleGroup.instanceData[counter].World = worldMatrix;
				particleGroup.instanceData[counter].color = particleIterator->color;
				particleGroup.instanceData[counter].color.w = alpha;
				++counter;
			}




			//次のパーティクルに進む
			++particleIterator;

		}

		// ここでインスタンス数を更新
		particleGroup.instanceCount = counter;

	}




}

void ParticleMnager::Draw()
{


	// パーティクルグループが設定されていない場合は描画しない
	if (particleGroups.empty()) {
		return;
	}

	//ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureParticle());
	//psoを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateParticle());
	//purimitetopologyを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パーティクルグループごとに描画
	for (const auto& [name, particleGroup] : particleGroups) {


		// インスタンス数が 0 の場合は描画しない
		if (particleGroup.instanceCount == 0) {
			continue;
		}


		dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &particleGroup.vertexBufferView);
		//マテリアルのCBufferの場所を設定
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, particleGroup.materialResource->GetGPUVirtualAddress());
		// インスタンシングデータの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(2, particleGroup.materialdata.textureIndex);
		// テクスチャの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(1, particleGroup.srvIndex);
		//描画！
		dxCommon_->GetCommandList()->DrawInstanced(UINT(particleGroup.vertexCount), particleGroup.instanceCount, 0, 0);

	}

}

void ParticleMnager::CreateParticleGroup(const std::string name, const std::string textureFilePath, VerticesType verticesType, std::unique_ptr<IParticleBehavior> behavior)
{



	//登録済みなら早期リターン
	if (particleGroups.contains(name)) {
		return;
	}

	//パーティクルグループを作成コンテナに登録
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録

	//パーティクルグループのマテリアルデータを初期化
	//マテリアル
	particleGroups.at(name).materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	particleGroups.at(name).materialData = nullptr;
	particleGroups.at(name).materialResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).materialData));
	//色
	particleGroups.at(name).materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	particleGroups.at(name).materialData->enableLighting = false;//有効にするか否か
	particleGroups.at(name).materialData->uvTransform = particleGroups.at(name).materialData->uvTransform.MakeIdentity4x4();

	//頂点データを作成
	std::vector<VertexData>vertices = MakeCylinderVertices();
	switch (verticesType) {
	case VerticesType::Quad:
		vertices = MakeQuadVertices();
		break;
	case VerticesType::Ring:
		vertices = MakeRingVertices();
		break;
	case VerticesType::Cylinder:
		vertices = MakeCylinderVertices();
		break;
	}
	particleGroups.at(name).vertexCount = static_cast<uint32_t>(vertices.size());
	// GPUリソース作成
	particleGroups.at(name).vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertices.size());
	// リソースアドレスを設定
	particleGroups.at(name).vertexBufferView.BufferLocation = particleGroups.at(name).vertexResource->GetGPUVirtualAddress();
	particleGroups.at(name).vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices.size());
	particleGroups.at(name).vertexBufferView.StrideInBytes = sizeof(VertexData);
	// GPUにデータ転送
	VertexData* vertexData = nullptr;
	particleGroups.at(name).vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices.data(), sizeof(VertexData) * vertices.size());
	particleGroups.at(name).vertexResource->Unmap(0, nullptr);

	//テクスチャファイルパスを登録
	particleGroups.at(name).materialdata.textureFilePath = textureFilePath;
	//テクスチャファイルを読み込んでSRVを取得
	textureManager_->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	//SRVのインデックスを取得
	particleGroups.at(name).materialdata.textureIndex = textureManager_->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	//最大インスタンスカウント
	uint32_t MaxInstanceCount = 1000;
	//インスタンス数を初期化
	particleGroups.at(name).instanceCount = 0;
	//インスタンス用のリソースを作成
	particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * MaxInstanceCount);
	//インスタンス用のリソースをマップ
	particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//インスタンスのデータを初期化
	ParticleForGPU particleForGPU;
	particleForGPU.WVP = particleForGPU.WVP.MakeIdentity4x4();
	particleForGPU.World = particleForGPU.World.MakeIdentity4x4();
	particleForGPU.color = { 1.0f,1.0f,1.0f,0.0f };
	//インスタンスのデータを登録
	for (uint32_t index = 0; index < MaxInstanceCount; ++index) {
		particleGroups.at(name).instanceData[index] = particleForGPU;
	}

	//insutansing用のsrvインデックス
	particleGroups.at(name).srvIndex = srvManager_->Allocate();
	//srv生成
	srvManager_->CreateSRVforStructuredBuffer(particleGroups.at(name).srvIndex, particleGroups.at(name).instanceResource.Get(), MaxInstanceCount, sizeof(ParticleForGPU));

	// Behavior 登録
	particleGroups.at(name).behavior = std::move(behavior);




}

void ParticleMnager::Emit(const std::string& name, const Vector3 position, uint32_t count)
{


	//パーティクルグループが存在するかチェックしてassert
	assert(particleGroups.contains(name));
	//パーティクルグループのパーティクルリストにパーティクルを追加
	for (uint32_t i = 0; i < count; ++i) {


		
		particleGroups.at(name).particles.push_back(particleGroups.at(name).behavior->Create(randomEngine, position));

	}

	//パーティクルグループのインスタンス数を更新
	particleGroups.at(name).instanceCount = count;
	////インスタンス用のリソースを作成
	//particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroups.at(name).instanceCount);
	////インスタンス用のリソースをマップ
	//particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//


}

void ParticleMnager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = modelManager_->FindModel(filepath);
}

std::vector<VertexData> ParticleMnager::MakeRingVertices(uint32_t  RingDivide, float outerRadius, float innerRadius)
{

	std::vector<VertexData> ringVertices;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(RingDivide);

	for (uint32_t index = 0; index < RingDivide; ++index) {
		// 現在と次の角度
		float angle = index * radianPerDivide;
		float nextAngle = ((index + 1) % RingDivide) * radianPerDivide;

		// sin, cos
		float sin = std::sinf(angle);
		float cos = std::cosf(angle);
		float sinnext = std::sinf(nextAngle);
		float cosnext = std::cosf(nextAngle);

		// UV (ここもwrapを考慮)
		float u = (static_cast<float>(index) / RingDivide);
		float unext = (static_cast<float>(index + 1) / RingDivide);

		VertexData v[] = {
			{ {-sin * outerRadius,  cos * outerRadius,  0.0f, 1.0f},     {u,     0.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sin * innerRadius,  cos * innerRadius,  0.0f, 1.0f},     {u,     1.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sinnext * outerRadius, cosnext * outerRadius, 0.0f, 1.0f}, {unext, 0.0f}, {0.0f, 0.0f, 1.0f} },

			{ {-sinnext * outerRadius, cosnext * outerRadius, 0.0f, 1.0f}, {unext, 0.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sin * innerRadius,  cos * innerRadius,  0.0f, 1.0f},     {u,     1.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sinnext * innerRadius, cosnext * innerRadius, 0.0f, 1.0f}, {unext, 1.0f}, {0.0f, 0.0f, 1.0f} }
		};

		for (const auto& vert : v) {
			ringVertices.push_back(vert);
		}
	}

	return ringVertices;

}

std::vector<VertexData> ParticleMnager::MakeCylinderVertices(uint32_t cylinderDivide, float topRadius, float bottomRadius, float height)
{
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(cylinderDivide);

	std::vector<VertexData> cylinderVertices;

	for (uint32_t index = 0; index < cylinderDivide; ++index) {

		float sin = std::sinf(index * radianPerDivide);
		float cos = std::cosf(index * radianPerDivide);
		float sinnext = std::sinf((index + 1) * radianPerDivide);
		float cosnext = std::cosf((index + 1) * radianPerDivide);
		float u = float(index) / float(cylinderDivide);
		float unext = float(index + 1) / float(cylinderDivide);

		VertexData v[] = {
			{{-sin * topRadius,height,cos * topRadius,1.0f},				{u,0.0f} ,		{-sin,0.0f,cos}},
			{{-sinnext * topRadius,height,cosnext * topRadius,1.0f},		{unext,0.0f},	{-sinnext,0.0f,cosnext}},
			{{-sin * bottomRadius,0.0f,cos * bottomRadius,1.0f},			{u,1.0f} ,		{-sin,0.0f,cos}},
			{{-sinnext * topRadius,height,cosnext * topRadius,1.0f},		{unext,0.0f},	{-sinnext,0.0f,cosnext}},
			{{-sinnext * bottomRadius,0.0f,cosnext * bottomRadius,1.0f},	{unext,1.0f},	{-sinnext,0.0f,cosnext}},
			{{-sin * bottomRadius,0.0f,cos * bottomRadius,1.0f},{u,1.0f} ,	{-sin,0.0f,cos}}

		};
		for (const auto& vert : v) {
			cylinderVertices.push_back(vert);
		}

	}
	return cylinderVertices;
}

std::vector<VertexData> ParticleMnager::MakeQuadVertices()
{
	//クワッドの頂点情報を作成
	std::vector<VertexData> vertices;
	vertices = {
			{{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	};
	return vertices;
}

void ParticleMnager::SetBehavior(const std::string& groupName, std::unique_ptr<IParticleBehavior> behavior)
{
	//グループが存在するかチェック
	assert(particleGroups.contains(groupName) && "ParticleGroup does not exist!");

	// ムーブ代入でユニークポインタを保持させる
	particleGroups.at(groupName).behavior = std::move(behavior);
}

