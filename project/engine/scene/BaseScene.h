#pragma once

class SceneManager;
struct SceneServices;
class BaseScene
{
public:
	//すべて純粋仮想関数として宣言する


	//ゲームの初期化
	virtual void Initialize() = 0;
	//終了
	virtual void Finalize() = 0;
	//更新
	virtual void Update() = 0;
	//描画
	virtual void Draw() = 0;
	virtual const char* GetSceneName() const = 0;
	virtual void DrawEditorImGui() {}
	//デストラクタ
	virtual ~BaseScene() = default;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }
	virtual void SetServices(const SceneServices* services) { services_ = services; }

protected:
	SceneManager* GetSceneManager() const { return sceneManager_; }
	const SceneServices& GetServices() const { return *services_; }

private:
	SceneManager* sceneManager_ = nullptr;
	const SceneServices* services_ = nullptr;

};

