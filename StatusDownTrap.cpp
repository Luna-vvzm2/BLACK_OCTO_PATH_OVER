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
    , m_triggered(false)
    , m_duration(STATUS_DOWN_DURATION)
{
}

bool StatusDownTrap::Init() {
    if (!EntityActor::Init()) return false;

    if (m_collision) {
        m_collision->SetRect(m_initialSize.x, m_initialSize.y);
    }

    m_effect.Init(m_sprite, GetTransform());   // 起動前のエフェクト(ループ)を開始

    return true;
}

void StatusDownTrap::Update(float deltaTime) {
    EntityActor::Update(deltaTime);
    m_effect.Update(deltaTime);

    if (m_triggered) {
        if (m_effect.IsActivateFinished()) {
            Destroy();
        }
        return;
    }

    // PlayScene経由でプレイヤーを取得
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (!playScene) return;

    PlayerEntity* player = playScene->GetPlayer();
    if (!player) return;

    CollisionComponent* playerCol = player->GetCollision();
    if (!m_collision || !playerCol) return;

    // 踏んだ(重なった)らステータスダウンを付与して、罠は消える
    if (m_collision->CheckCollision(playerCol)) {
        std::cout << "[StatusDownTrap] 起動 ";
        player->ApplyStatusDown(m_duration);
        m_triggered = true;
        m_effect.PlayActivate();   // 起動時のエフェクトを再生(終わったら消える)
    }
}