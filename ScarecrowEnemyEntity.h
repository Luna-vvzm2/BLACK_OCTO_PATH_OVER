#pragma once

#include "EnemyEntity.h"

class HPComponent;

class ScarecrowEnemyEntity : public EnemyEntity
{
public:
    ScarecrowEnemyEntity(Scene* scene, const Vector2d& pos);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    void TakeDamage(int damage, const Vector2d& knockback) override;

private:
    enum  ScarecrowState
    {
        Idle,       // ‘Ò‹@
        BlowHit,
    };
    ScarecrowState m_state = Idle;
    AnimationComponent* m_anim = nullptr;
    HPComponent* m_hp = nullptr;
    VelocityComponent* m_velocity = nullptr;
    void UpdateIdle(float dt);
    bool m_faceRight = true;

    float m_hitTimer = 0.0f;
};