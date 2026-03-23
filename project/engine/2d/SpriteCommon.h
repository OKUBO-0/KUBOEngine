#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"

class SpriteCommon
{
public:
    static SpriteCommon* GetInstance();

    /// 初期化（DX共通とパイプライン生成）
    void Initialize(DirectXCommon* dxCommon);

    /// 終了処理（インスタンス破棄）
    void Finalize();

    /// 共通描画設定（ルートシグネチャ・パイプライン・トポロジ）
    void CommonDraw();

    DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
    static SpriteCommon* instance_;
    DirectXCommon* dxCommon_ = nullptr; // DX共通クラス参照
    std::unique_ptr<GraphicsPipeline> graphicsPipeline_; // グラフィックスパイプライン
};