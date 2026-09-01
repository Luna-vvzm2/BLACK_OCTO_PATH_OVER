#pragma once
#include "EnemyEntity.h"

class ArrowEnemyEntity : public EnemyEntity
{
public:
    ArrowEnemyEntity(Scene* scene, const Vector2d& pos);

    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    std::string GetTexturePath() const override;
    void TakeDamage(int damage, const Vector2d& knockback) override;

private:


    enum class AttackState
    {
        Idle,
        Warning,
        Attack,
        Cooldown,
        Hit,
        Dead
    };

    AttackState m_attackState;
    float m_hpTestTimer;
    float m_deathTimer;
    float m_attackTimer;
    float m_attackInterval;
    float GetGroundY(float x);
    Vector2d m_targetPos;
    bool m_attackExecuted;
    float m_damageTimer = 0.0f;
    int m_damagePerSecond = 10;
    bool m_isHit = false;
    float m_hitTimer;
    AnimationComponent* m_animation = nullptr;
    float m_detectRange = 1000.0f;   // 感知距離
    float m_attackRange = 500.0f;   // 攻撃開始距離（必要なら）
    bool m_targetLocked = false;
};

