#include "StageBackActor.h"
#include "CollisionComponent.h"

StageBackActor::StageBackActor(
    Scene* scene,
    const Vector2d& pos,
    int prevStage,
    int spawnIndex)
    : BlockActor(scene, pos)
    , m_prevStage(prevStage)
    , m_spawnIndex(spawnIndex)
{
}

bool StageBackActor::Init()
{
    if (!BlockActor::Init()) return false;
    m_collision->SetRect(104.0f, 520.0f);
    return true;
}