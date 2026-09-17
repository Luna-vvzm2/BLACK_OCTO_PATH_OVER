#include "SpiritBulletEntity.h"
#include "CollisionComponent.h"
SpiritBulletEntity::SpiritBulletEntity(
    Scene* scene,
    const Vector2d& pos,
    const Vector2d& velocity,
    float deleteRange,
    const std::string& texturePath,
    int damage,
    const Vector2d& drawSize,
    bool rotate,
    float rotateInterval,
    float rotateStep
)
    : EnemyBullet(
        scene,
        pos,
        velocity,
        deleteRange,
        texturePath,
        damage,
        drawSize,
        rotate,
        rotateInterval,
        rotateStep
    )
{
}

bool SpiritBulletEntity::Init()
{
    if (!EnemyBullet::Init())
    {
        return false;
    }

    // ‰…—ì‚Ì’eF15 ~ 15 px
    m_collision->SetRect(15.0f, 15.0f);

    return true;
}