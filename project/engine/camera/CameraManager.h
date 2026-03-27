#pragma once
#include <Camera.h>
#include <unordered_map>
#include <string>
#include <memory>
class CameraManager
{
public:
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(CameraManager&) = delete;
	CameraManager& operator=(CameraManager&) = delete;

	//終了
	void Finalize();

	//初期化
	void Initialize();



	//カメラの追加
	void AddCamera(const std::string& name, const Camera* camera);

	//カメラの削除
	void RemoveCamera(const std::string& name);

	//カメラの取得
	Camera* GetCamera(const std::string& name);

	// アクティブカメラの取得
	Camera* GetActiveCamera();


	// アクティブカメラの設定
	void SetActiveCamera(const std::string& name);



private:
	//カメラデータ
	std::unordered_map<std::string, Camera> cameras;

	// アクティブカメラ名
	std::string activeCameraName;

	//デフォルトカメラ
	
	//デフォルトカメラ
	Camera* defaultCamera=nullptr;


};

