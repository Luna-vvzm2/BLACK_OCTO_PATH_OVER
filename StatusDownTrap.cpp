#include "StatusDownTrap.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "CollisionComponent.h"

namespace
{
    constexpr float STATUS_DOWN_DURATION = 15.0f;   // 効果時間(秒)
}

StatusDownTrap::StatusDownTrap(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_duration(STATUS_DOWN_DURATION)
{
}

bool StatusDownTrap::Init() {
    if (!EntityActor::Init()) return false;

    if (m_collision) {
        m_collision->SetRect(m_initialSize.x, m_initialSize.y); // 当たり判定のサイズ
    }

    return true;
}

void StatusDownTrap::Update(float deltaTime) {
    EntityActor::Update(deltaTime);

    // PlayScene経由でプレイヤーを取得
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (!playScene) return;

    PlayerEntity* player = playScene->GetPlayer();
    if (!player) return;

    CollisionComponent* playerCol = player->GetCollision();
    if (!m_collision || !playerCol) return;

    // 踏んだ(重なった)らステータスダウンを付与して、罠は消える
    if (m_collision->CheckCollision(playerCol)) {
        player->ApplyStatusDown(m_duration);
        Destroy();
    }
}