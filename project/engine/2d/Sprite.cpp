#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Matrix4x4.h"
#include <MyMath.h>
#include "CameraManager.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
    textureFilePath_ = textureFilePath;

    // テクスチャ読み込みとインデックス取得
    spriteCommon_ = spriteCommon;
    spriteCommon_->GetTextureManager()->LoadTexture(textureFilePath);
    textureIndex = spriteCommon_->GetTextureManager()->GetTextureIndexByFilePath(textureFilePath);

    // GPUリソース生成（頂点・インデックス・マテリアル・行列）
    vetexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
    indexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
    materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(MaterialSprite));
    transformationMatrixResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrixsprite));

    // 頂点バッファビュー設定
    vertexBufferView.BufferLocation = vetexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    // インデックスバッファビュー設定
    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // マテリアル初期化
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
    materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();

    // 行列初期化
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
    transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
    transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();

    // カメラ用リソース
    cameraResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(CaMeraForGpu));
    cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGpu));
    cameraForGpu->worldPosition = { 0.0f,0.0f,0.0f };

    // テクスチャサイズを画像に合わせる
    AdjustTextureSize();
}

void Sprite::Update()
{
    // アクティブカメラの位置を取得しGPUへ送信
    Camera* activeCamera = spriteCommon_->GetCameraManager()->GetActiveCamera();
    Vector3 cameraPosition = activeCamera->GetTransform().translate;
    cameraForGpu->worldPosition = cameraPosition;

    // SRT変換設定
    transform.rotate = { 0.0f,0.0f,rotation };
    transform.translate = { position.x,position.y,0.0f };
    transform.scale = { size.x,size.y,1.0f };

    // アンカーポイント計算
    float left = 0.0f - anchorPoint_.x;
    float right = 1.0f - anchorPoint_.x;
    float top = 0.0f - anchorPoint_.y;
    float bottom = 1.0f - anchorPoint_.y;

    // 左右反転
    if (isFlipX_) {
        left = -left;
        right = -right;
    }
    // 上下反転
    if (isFlipY_) {
        top = -top;
        bottom = -bottom;
    }

    // テクスチャ座標計算
    const DirectX::TexMetadata& metadata = spriteCommon_->GetTextureManager()->GetMetaData(textureFilePath_);
    float tex_left = textureLeftTop_.x / metadata.width;
    float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
    float tex_top = textureLeftTop_.y / metadata.height;
    float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;

    // 頂点データ書き込み
    vetexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    vertexData[0].position = { left,  bottom, 0.0f, 1.0f }; // 左下
    vertexData[1].position = { left,  top,    0.0f, 1.0f }; // 左上
    vertexData[2].position = { right, bottom, 0.0f, 1.0f }; // 右下
    vertexData[3].position = { right, top,    0.0f, 1.0f }; // 右上

    vertexData[0].texcoord = { tex_left,  tex_bottom }; // 左下
    vertexData[1].texcoord = { tex_left,  tex_top }; // 左上
    vertexData[2].texcoord = { tex_right, tex_bottom }; // 右下
    vertexData[3].texcoord = { tex_right, tex_top }; // 右上

    vertexData[0].normal = { 0.0f,0.0f,-1.0f };
    vertexData[1].normal = { 0.0f,0.0f,-1.0f };
    vertexData[2].normal = { 0.0f,0.0f,-1.0f };
    vertexData[3].normal = { 0.0f,0.0f,-1.0f };

    // インデックスデータ書き込み
    indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
    indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
    indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

    // 行列計算
    worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    projectionMatrix = MyMath::MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWindth), float(WinApp::kClientHeight), 0.0f, 100.0f);
    worldViewProjectionMatrix = worldMatrix * viewMatrix.MakeIdentity4x4() * projectionMatrix;

    transformaitionMatrixData->WVP = worldViewProjectionMatrix;
    transformaitionMatrixData->World = worldMatrix;
}

void Sprite::Draw()
{
    // 頂点バッファ設定
    spriteCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    spriteCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

    // マテリアルCBV設定 (RootParameter[0])
    spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

    // テクスチャSRV設定 (RootParameter[1])
    spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(1, spriteCommon_->GetTextureManager()->GetSrvHandleGPU(textureFilePath_));

    // 行列CBV設定 (RootParameter[2])
    spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(2, transformationMatrixResource->GetGPUVirtualAddress());

    // インデックス付き描画
    spriteCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::AdjustTextureSize()
{
    // テクスチャメタデータを取得
    const DirectX::TexMetadata& metadata = spriteCommon_->GetTextureManager()->GetMetaData(textureFilePath_);

    // 切り出しサイズをテクスチャ全体に設定
    textureSize_ = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };

    // スプライトサイズをテクスチャサイズに合わせる
    size = textureSize_;
}
