#pragma once
#include <KamataEngine.h>
#include "../../enemy/EnemyManager.h"
#include "../../player/core/Player.h"

namespace DirectXGame {

class MiniMap {
public:
    void Initialize();
    void Update(const Player* player, const EnemyManager& enemyManager);
    void Draw();

private:
    uint32_t playerTex_ = 0;
    uint32_t enemyTex_ = 0;
    uint32_t orbTex_ = 0;
    uint32_t bgTex_ = 0;

    // ミニマップの中心（アイコン計算用）
    KamataEngine::Vector2 mapCenter_ = { 320, 360 };

    KamataEngine::Vector2 bgPos_ = { 0, 0 };

    // 円形ミニマップの半径（背景画像に合わせる）
    float radius_ = 180.0f;

    // ワールド→ミニマップ縮尺
    float scale_ = 3.0f;

    struct Icon {
        KamataEngine::Vector2 pos;
        bool isEnemy;
    };

    std::vector<Icon> icons_;
};

} // namespace DirectXGame
