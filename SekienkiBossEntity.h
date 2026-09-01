#pragma once
#include "BossEntity.h"
#include "EnemyEntity.h"

class SekienkiBossEntity : public BossEntity
{
public:

    SekienkiBossEntity(Scene* scene, const Vector2d& pos, const Vector2d& size);

    bool Init() override;
    int GetMaxHP() const override { return 5600; }
    void StartJumpAttack();
    void PhaseChange();
    void TakeDamage(int damage, const Vector2d& knockback) override;
    void Update(float deltaTime) override;

protected:

    void UpdateAI(float deltaTime) override;
    void UpdateAttack(float deltaTime) override;
    bool TryDamagePlayer();

private:
    int m_attackAnimFrame;
    float m_attackAnimTimer;

    int m_attackStep;
    int m_phase;

    float m_attackTimer;
    float m_jumpAttackCooldown;

    float m_rollDistanceLeft;
    float m_rollWaitTimer;
    float m_rollDir;

    bool m_fire;
    bool m_bulletActive;
    float m_fallStartY;
    float m_hoverY;
    bool m_jumpOffsetApplied ;
    bool m_secondJump;
    bool m_darkAttackFire;
    float m_tornadoDistanceLeft;
    bool  m_dead;

    Vector2d m_bulletPos;
    Vector2d m_bulletVel;
};

