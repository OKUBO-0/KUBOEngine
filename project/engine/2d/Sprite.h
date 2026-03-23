#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <string>
#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>

class SpriteCommon;
class Sprite
{
public:
    /// 初期化処理（共通設定とテクスチャ読み込み）
    void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);

    /// 毎フレーム更新（座標・回転・UVなど）
    void Update();

    /// 描画処理（バッファ設定と描画コマンド発行）
    void Draw();

    // サイズ
    const Vector2& GetSize() const { return size; }
    void SetSize(const Vector2& size) { this->size = size; }

    // 位置
    const Vector2& GetPosition() const { return position; }
    void SetPosition(const Vector2& position) { this->position = position; }

    // 回転角度
    const float& GetRotation() const { return rotation; }
    void SetRotation(const float& rotation) { this->rotation = rotation; }

    // 色（マテリアルカラー）
    const Vector4& GetColor() const { return materialData->color; }
    void setColor(const Vector4& color) { materialData->color = color; }

    // アンカーポイント（基準位置）
    const Vector2& GetAnchorPoint() const { return anchorPoint_; }
    void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

    // 左右反転
    const bool& GetIsFlipX() const { return isFlipX_; }
    void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }

    // 上下反転
    const bool& GetIsFlipY() const { return isFlipY_; }
    void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }

    // テクスチャ左上座標
    const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
    void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

    // テクスチャ切り出しサイズ
    const Vector2& GetTextureSize() const { return textureSize_; }
    void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:
    std::string textureFilePath_;

    /// テクスチャサイズを画像に合わせる
    void AdjustTextureSize();

    SpriteCommon* spriteCommon_ = nullptr;

    // GPUリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vetexResource;                // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;                // インデックスバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;             // マテリアル用バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource; // 行列用バッファ

    // バッファデータへのポインタ
    VertexData* vertexData = nullptr;
    uint32_t* indexData = nullptr;
    MaterialSprite* materialData = nullptr;
    TransformationMatrixsprite* transformaitionMatrixData = nullptr;

    // バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    // 変換情報（スケール・回転・平行移動）
    EulerTransform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

    // 設定用パラメータ
    Vector2 size = { 640.0f,360.0f };
    Vector2 position = { 0.0f,0.0f };
    float rotation = 0.0f;
    uint32_t textureIndex = 0;

    Vector2 anchorPoint_ = { 0.0f,0.0f }; // アンカーポイント
    bool isFlipX_ = false;                // 左右反転
    bool isFlipY_ = false;                // 上下反転

    Vector2 textureLeftTop_ = { 0.0f,0.0f };   // テクスチャ左上座標
    Vector2 textureSize_ = { 512.0f,512.0f };  // テクスチャ切り出しサイズ

    // 行列
    Matrix4x4 worldMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Matrix4x4 worldViewProjectionMatrix;

    // カメラ関連
    Camera* camera = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource; // GPU送信用カメラリソース
    CaMeraForGpu* cameraForGpu = nullptr;                  // GPU送信用カメラ構造体
};