#include "Model.h"
#include <fstream>
#include <sstream>
#include <assert.h>
#include "TextureManager.h"
#include "SrvManager.h"

void Model::Initialize(ModelCommon* modeleCommon, const std::string& directorypath, const std::string& filename)
{
	modelCommon_ = modeleCommon;

	// モデルデータ・アニメーション・スケルトン・スキンクラスターを読み込み/生成
	modelData = LoadModelFile(directorypath, filename);
	animation = LoadAnimationFile(directorypath, filename);
	skeleton = CreateSkeleton(modelData.rootNode);
	skinCluster = CreateSkinCluster();

	// 頂点バッファ生成とデータ転送
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// インデックスバッファ生成とデータ転送
	indexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * modelData.indices.size());
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * modelData.indices.size());
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
	std::memcpy(mappedIndex, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());

	// マテリアルバッファ生成と初期化
	materialResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルト白
	materialData->enableLighting = true;                        // ライティング有効
	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();
	materialData->shiniess = 60.0f;                       // 光沢度

	// テクスチャ読み込みとインデックス取得
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);
}

void Model::Draw()
{
	// 頂点バッファビュー（通常頂点 + スキンクラスター影響情報）
	D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
		vertexBufferView,
		skinCluster.influenceBufferView
	};

	// 頂点バッファ設定
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 2, vbvs);

	// インデックスバッファ設定
	modelCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	// マテリアルCBV設定
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// テクスチャSRV設定
	modelCommon_->GetSRVManager()->SetGraficsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath));

	// インデックス付き描画（インスタンス数 = 1）
	modelCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(
		static_cast<UINT>(modelData.indices.size()), // インデックス数
		1,  // インスタンス数
		0,  // 開始インデックス
		0,  // 基準頂点
		0   // 開始インスタンス
	);
}

Node Model::ReadNode(aiNode* node)
{
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotation;

	// ノード変換を分解（スケール・回転・平行移動）
	node->mTransformation.Decompose(scale, rotation, translate);
	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotation.x, -rotation.y, -rotation.z, rotation.w };
	result.transform.translate = { translate.x, translate.y, translate.z };

	// ローカル行列生成
	result.localMatrix = MyMath::MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	// ノード名と子ノード読み込み
	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

Skeleton Model::CreateSkeleton(const Node& rootNode)
{
	Skeleton skeleton;

	// ルートノードからジョイントツリー構築
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とインデックスのマッピング
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}
	return skeleton;
}

int32_t Model::CreateJoint(const Node& node, std::optional<int32_t> parent, std::vector<Joint>& joints)
{
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = joint.skeletonSpaceMatrix.MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());
	joint.parent = parent;

	joints.push_back(joint);

	// 子ジョイントを再帰的に生成
	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	return joint.index;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename)
{
    MaterialData materialData; // 構築するマテリアルデータ
    std::string line;          // ファイルから読み込む1行
    std::ifstream file(directorypath + "/" + filename); // ファイルを開く
    assert(file.is_open());    // 開けなかった場合は停止

    while (std::getline(file, line)) {
        std::string identifile;
        std::stringstream s(line);
        s >> identifile;

        // マテリアルファイルの識別子に応じた処理
        if (identifile == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // テクスチャファイルパスを連結
            materialData.textureFilePath = directorypath + "/" + textureFilename;
        }
    }
    return materialData;
}

ModelData Model::LoadModelFile(const std::string& ditrectoryPath, const std::string& filename)
{
    ModelData modelData; // 構築するモデルデータ
    Assimp::Importer importer;
    std::string path = ditrectoryPath + "/" + "models" + "/" + filename;

    // Assimpでモデルファイル読み込み
    const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    assert(scene->HasMeshes()); // メッシュが存在しない場合は停止

    // メッシュごとの処理
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        assert(mesh->HasNormals());        // 法線情報必須
        assert(mesh->HasTextureCoords(0)); // テクスチャ座標必須

        modelData.vertices.resize(mesh->mNumVertices); // 頂点数分確保

        // 頂点情報の読み込み
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

            // 右手系 → 左手系変換
            modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
        }

        // インデックス情報の読み込み（三角形のみ対応）
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3);
            for (uint32_t element = 0; element < face.mNumIndices; ++element) {
                uint32_t vertexIndex = face.mIndices[element];
                modelData.indices.push_back(vertexIndex);
            }
        }

        // ボーン情報の読み込み
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            aiBone* bone = mesh->mBones[boneIndex];
            std::string jointName = bone->mName.C_Str();
            JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

            // バインドポーズ行列を分解
            aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
            aiVector3D scale, translate;
            aiQuaternion rotation;
            bindPoseMatrixAssimp.Decompose(scale, rotation, translate);

            Matrix4x4 bindposeMatrix = MyMath::MakeAffineMatrix(
                { scale.x, scale.y, scale.z },
                { rotation.x, -rotation.y, -rotation.z, rotation.w },
                { -translate.x, translate.y, translate.z }
            );

            // 逆バインドポーズ行列を格納
            jointWeightData.inverseBindPoseMatrix = bindposeMatrix.Inverse();

            // 頂点ごとの重みを格納
            for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                jointWeightData.vertexWeights.push_back({
                    bone->mWeights[weightIndex].mWeight,
                    bone->mWeights[weightIndex].mVertexId
                    });
            }
        }
    }

    // マテリアル解析
    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
            aiString texturePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
            modelData.material.textureFilePath = ditrectoryPath + "/" + texturePath.C_Str();
        }
    }

    // ルートノード読み込み
    modelData.rootNode = ReadNode(scene->mRootNode);

    return modelData;
}

Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
    Animation animation;
    Assimp::Importer importer;
    std::string filepath = directoryPath + "/" + "models" + "/" + filename;

    const aiScene* scene = importer.ReadFile(filepath.c_str(), 0);

    // アニメーションが存在しない場合は空を返す
    if (scene->mNumAnimations == 0) {
        return animation;
    }

    aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初のアニメーションのみ採用
    animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

    // チャンネルごとのアニメーション読み込み
    for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
        aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

        // 位置キー
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
            KeyframeVector3 Keyframe;
            Keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            Keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
            nodeAnimation.translate.push_back(Keyframe);
        }

        // 回転キー
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
            aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion Keyframe;
            Keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            Keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
            nodeAnimation.rotate.push_back(Keyframe);
        }

        // スケールキー
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            KeyframeVector3 Keyframe;
            Keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            Keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
            nodeAnimation.scale.push_back(Keyframe);
        }
    }
    return animation;
}

SkinCluster Model::CreateSkinCluster()
{
    SkinCluster skinCluster;

    // パレット用リソース生成（ジョイント行列）
    skinCluster.paletteResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
    WellForGPU* mappedPalette = nullptr;
    skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster.mappedPalette = { mappedPalette, skeleton.joints.size() };

    // SRV割り当て（パレット用）
    uint32_t srvIndex = modelCommon_->GetSRVManager()->Allocate();
    modelCommon_->GetSRVManager()->CreateSRVforStructuredBuffer(
        srvIndex,
        skinCluster.paletteResource.Get(),
        (UINT)skeleton.joints.size(),
        sizeof(WellForGPU)
    );
    skinCluster.paletteSrvHandle.first = modelCommon_->GetSRVManager()->GetCPUDescriptorHandle(srvIndex);
    skinCluster.paletteSrvHandle.second = modelCommon_->GetSRVManager()->GetGPUDescriptorHandle(srvIndex);

    // Influence用リソース生成（頂点ごとのボーン影響情報）
    skinCluster.influenceResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
    VertexInfluence* mappedInfluence = nullptr;
    skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
    skinCluster.mappedInfluence = { mappedInfluence, modelData.vertices.size() };

    // Influence用VBV設定
    skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
    skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
    skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

    // InverseBindPoseMatrixを初期化（単位行列で埋める）
    skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
    std::generate(
        skinCluster.inverseBindPoseMatrices.begin(),
        skinCluster.inverseBindPoseMatrices.end(),
        [] { return MyMath::MakeIdentity4x4(); }
    );

    // SkinClusterデータをスケルトンに適用
    for (const auto& jointWeight : modelData.skinClusterData) {
        auto it = skeleton.jointMap.find(jointWeight.first); // ジョイント名からインデックスを検索
        if (it == skeleton.jointMap.end()) {
            continue; // 該当ジョイントがなければスキップ
        }

        // InverseBindPoseMatrixを設定
        skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;

        // 頂点ごとのInfluence情報を設定
        for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
            auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vectorIndex];

            // 最大影響数（kNumMaxInfluence）まで格納
            for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
                if (currentInfluence.weights[index] == 0.0f) {
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = (*it).second;
                    break;
                }
            }
        }
    }

    return skinCluster;
}