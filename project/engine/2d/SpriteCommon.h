#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
class CameraManager;
class TextureManager;

class SpriteCommon
{
public:
    SpriteCommon() = default;
    ~SpriteCommon() = default;
    SpriteCommon(const SpriteCommon&) = delete;
    SpriteCommon& operator=(const SpriteCommon&) = delete;

    /// 初期化（DX共通とパイプライン生成）
    void Initialize(DirectXCommon* dxCommon);

    /// 終了処理（インスタンス破棄）
    void Finalize();

    /// 共通描画設定（ルートシグネチャ・パイプライン・トポロジ）
    void CommonDraw();

    DirectXCommon* GetDxCommon() const { return dxCommon_; }
    CameraManager* GetCameraManager() const { return cameraManager_; }
    TextureManager* GetTextureManager() const { return textureManager_; }
    void SetCameraManager(CameraManager* cameraManager) { cameraManager_ = cameraManager; }
    void SetTextureManager(TextureManager* textureManager) { textureManager_ = textureManager; }

private:
    DirectXCommon* dxCommon_ = nullptr; // DX共通クラス参照
    CameraManager* cameraManager_ = nullptr;
    TextureManager* textureManager_ = nullptr;
    std::unique_ptr<GraphicsPipeline> graphicsPipeline_; // グラフィックスパイプライン
};
