#include "BossEntity.h"
#include "HPComponent.h"
#include "GravityComponent.h"

BossEntity::BossEntity(Scene* scene,const Vector2d& pos, const Vector2d& size)
    :EnemyEntity(scene, pos, size)
{
}

bool BossEntity::Init()
{
    if (!EnemyEntity::Init())
        return false;

    m_gravity = AddComponent<GravityComponent>(2800.0f);

    return true;
}

void BossEntity::Update(float deltaTime)
{
    EnemyEntity::Update(deltaTime);

    UpdateAI(deltaTime);
    UpdateAttack(deltaTime);
}

