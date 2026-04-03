#include "MiniMap.h"
#include <algorithm>
#include <cmath>
using namespace KamataEngine;

namespace DirectXGame {

void MiniMap::Initialize() {
    playerTex_ = TextureManager::Load("ui/game/minimap_player.png");
    enemyTex_ = TextureManager::Load("ui/game/minimap_enemy.png");
    orbTex_ = TextureManager::Load("ui/game/minimap_orb.png");
    bgTex_ = TextureManager::Load("ui/game/minimap_bg.png");
}

void MiniMap::Update(const Player* player, const EnemyManager& enemyManager) {
    icons_.clear();

    Vector3 pPos = player->GetWorldPosition();

    auto addIcon = [&](const Vector3& objPos, bool isEnemy) {
        Vector3 rel = { objPos.x - pPos.x, 0, objPos.z - pPos.z };

        float mx = rel.x * scale_;
        float my = -rel.z * scale_;

        // ミニマップ上の座標
        Vector2 pos = { mapCenter_.x + mx, mapCenter_.y + my };

        // --- 円周クランプ処理 ---
        float dx = pos.x - mapCenter_.x;
        float dy = pos.y - mapCenter_.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > radius_) {
            float ratio = radius_ / dist;
            pos.x = mapCenter_.x + dx * ratio;
            pos.y = mapCenter_.y + dy * ratio;
        }

        icons_.push_back({ pos, isEnemy });
        };

    // 敵
    for (auto& e : enemyManager.GetEnemies()) {
        if (e->IsActive()) addIcon(e->GetPosition(), true);
    }

    // オーブ
    for (auto& orb : enemyManager.GetExpOrbs()) {
        if (orb->IsActive()) addIcon(orb->GetPosition(), false);
    }
}

void MiniMap::Draw() {
    // --- 背景  ---
    {
        auto bg = Sprite::Create(bgTex_, bgPos_);
        bg->Draw();
    }

    // プレイヤーアイコン（常にミニマップ中心）
    {
        auto p = Sprite::Create(playerTex_, mapCenter_);
        p->SetAnchorPoint({ 0.5f, 0.5f });
        p->SetSize({ 20, 20 });
        p->Draw();
    }

    // --- アイコン ---
    for (auto& icon : icons_) {
        uint32_t tex = icon.isEnemy ? enemyTex_ : orbTex_;
        auto sprite = Sprite::Create(tex, icon.pos);
        sprite->SetAnchorPoint({ 0.5f, 0.5f });
        sprite->SetSize(icon.isEnemy ? Vector2{ 20,20 } : Vector2{ 16,16 });
        sprite->Draw();
    }
}

} // namespace DirectXGame
