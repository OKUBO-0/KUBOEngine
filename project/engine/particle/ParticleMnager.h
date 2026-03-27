#pragma once
#include <vector>
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include "Model.h"
#include "GraphicsPipeline.h"
#include "IParticleBehavior.h"
#include <memory>
class CameraManager;
class ModelManager;
class TextureManager;



enum class VerticesType
{
	Ring,
	Cylinder,
	Quad,

};


struct Particle {

	EulerTransform transform;
	Vector3 Velocity;
	float lifetime;
	float currentTime;
	
	Vector4 color;

	
};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;

};

class ParticleMnager
{
	struct ParticleGroup
	{
		MaterialData materialdata;
		//particleのリスト
		std::list<Particle> particles;
		std::unique_ptr<IParticleBehavior> behavior;

		//insutansing用のsrvインデックス
		uint32_t srvIndex;
		//insutansing用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
		//insutance数
		uint32_t instanceCount;
		//insutanceのデータ
		ParticleForGPU* instanceData = nullptr;
		//頂点
		uint32_t vertexCount = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		//VBV
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		//マテリアルにデータを書き込む	
		Material* materialData = nullptr;
		//std::string textureFilePath_;

	};
public:
	// コンストラクタをプライベートにする
public:
	ParticleMnager() = default;
	~ParticleMnager() = default;
	// コピーコンストラクタと代入演算子を削除する
	ParticleMnager(const ParticleMnager&) = delete;
	ParticleMnager& operator=(const ParticleMnager&) = delete;


public:

	//初期化
	void Initialize(DirectXCommon* dxcommn,SrvManager*srvmaneger, CameraManager* cameraManager, ModelManager* modelManager, TextureManager* textureManager);



	/// <summary>
	/// 終了処理
	///< / summary>
	void Finalize();
	void Update();
	void Draw();

	void CreateParticleGroup(const std::string name,const std::string textureFilePath, VerticesType verticesType = VerticesType::Quad, std::unique_ptr<IParticleBehavior> behavior=nullptr);

	void Emit(const std::string& name, const Vector3 position, uint32_t count);

	void SetModel(const std::string& filepath);

	
	//リングの頂点情報を作成
	std::vector<VertexData> MakeRingVertices(uint32_t RingDivide = 128, float outerRadius = 1.0f, float innerRadius = 0.2f);
	//シリンダーの頂点情報を作成
	std::vector<VertexData> MakeCylinderVertices(uint32_t cylinderDivide = 32, float topRadius = 1.0f, float bottomRadius = 1.0f, float height = 2.0f);
	//クワッドの頂点情報を作成
	std::vector<VertexData> MakeQuadVertices();

	// Behavior設定（明示的に設定する用）
	void SetBehavior(const std::string& groupName, std::unique_ptr<IParticleBehavior> behavior);


private:


	DirectXCommon* dxCommon_=nullptr;
	SrvManager* srvManager_ = nullptr;
	CameraManager* cameraManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;


	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	Model* model_ = nullptr;

	std::mt19937 randomEngine;

	
	//SRT
	EulerTransform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	std::unordered_map<std::string, ParticleGroup> particleGroups;

	//ビルボード行列
	Matrix4x4 backToFrontMatrix;
	
	float scrollX = 0.0f; // グローバル or メンバ変数として定義しておく

	// 頂点の種類
	VerticesType verticesType = VerticesType::Quad;

};


