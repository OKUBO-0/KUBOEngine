#pragma once
#include <map>
#include <string>
#include "Model.h"
class TextureManager;
class ModelManager
{
public:
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	//終了
	void Finalize();

/// <summary>
/// 初期化
/// </summary>
	void Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager, TextureManager* textureManager);
/// <summary>
/// モデルの読み込み
/// </summary>
	void LoadModel(const std::string& filePath);
/// <summary>
///	モデル検索
/// </summary>
	Model* FindModel(const std::string& filePath);

private:
	//モデルデータ
	std::map<std::string, std::unique_ptr < Model>> models;

	std::unique_ptr< ModelCommon> modelCommon = nullptr;
	SrvManager* srvmnager_ = nullptr;
};

