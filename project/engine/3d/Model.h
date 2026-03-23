#pragma once
#include "ModelCommon.h"
#include "RenderingData.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// 1頂点あたりの最大ボーン影響数
const uint32_t kNumMaxInfluence = 4;

struct VertexInfluence {
    std::array<float, kNumMaxInfluence> weights;      // 各ボーンの重み
    std::array<int32_t, kNumMaxInfluence> jointIndices; // 各ボーンのインデックス
};

struct WellForGPU {
    Matrix4x4 skeletonSpaceMatrix;              // スケルトン空間行列（位置用）
    Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用行列
};

struct SkinCluster {
    // 各ジョイントの逆バインドポーズ行列
    std::vector<Matrix4x4> inverseBindPoseMatrices;

    // 頂点影響情報（ボーンと重み）
    Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
    D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
    std::span<VertexInfluence> mappedInfluence;

    // ボーン行列（palette）
    Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
    std::span<WellForGPU> mappedPalette;
    std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
};

class Model
{
public:
    /// 初期化（モデルデータ・アニメーション・スケルトン・スキンクラスター生成）
    void Initialize(ModelCommon* modeleCommon, const std::string& directorypath, const std::string& filename);

    /// 描画処理
    void Draw();

    // ノード読み込み
    Node ReadNode(aiNode* node);
    Skeleton CreateSkeleton(const Node& rootNode);
    int32_t CreateJoint(const Node& node, std::optional<int32_t> parent, std::vector<Joint>& joints);

    // 各種データ取得
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return vertexBufferView; }
    ModelData GetModelData() { return modelData; }
    Animation& GetAnimation() { return animation; }
    Skeleton& GetSkeleton() { return skeleton; }
    SkinCluster& GetSkinCluster() { return skinCluster; }

    // マテリアル設定
    void SetEnableLighting(bool enable) { materialData->enableLighting = enable; }
    void SetColor(const Vector4& color) { materialData->color = color; }

    // ファイル読み込み
    MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename);
    ModelData LoadModelFile(const std::string& ditrectoryPath, const std::string& filename);
    Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

    // スキンクラスター生成
    SkinCluster CreateSkinCluster();

private:
    ModelCommon* modelCommon_ = nullptr; // 共通部

    ModelData modelData;   // モデルデータ
    Animation animation;   // アニメーションデータ
    Skeleton skeleton;     // スケルトン
    SkinCluster skinCluster; // スキンクラスター

    // GPUリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // 頂点バッファ
    VertexData* vertexData = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource; // マテリアルバッファ
    Material* materialData = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource; // インデックスバッファ
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    uint32_t* mappedIndex = nullptr;
};