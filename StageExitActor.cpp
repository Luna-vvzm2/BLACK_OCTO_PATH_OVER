#include "StageExitActor.h"
#include "CollisionComponent.h"
#include "TransformComponent.h"

StageExitActor::StageExitActor(Scene* scene,
    const Vector2d& pos,
    int nextStage)
    : BlockActor(scene, pos)
    , m_nextStage(nextStage)
{
    auto transform = AddComponent<TransformComponent>();
    transform->SetPosition(pos);
}

bool StageExitActor::Init()
{
    if (!BlockActor::Init())
        return false;

    m_collision->SetRect(104.0f, 520.0f);

    return true;
}