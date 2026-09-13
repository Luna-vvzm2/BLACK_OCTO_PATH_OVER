#pragma once

#include "EnemyEntity.h"

#include <string>

class PlayerEntity;
class SoundComponent;

class KooniEnemyEntity : public EnemyEntity
{
public:
    enum class Status
    {
        Idle,
        Move,
        Attack,
        Defeated
    };

    enum class Awareness
    {
        Undetected,
        Detected
    };

    KooniEnemyEntity(Scene* scene, const Vector2d& pos);

    bool Init() override;
    void Update(float deltaTime) override;
    void TakeDamage(int damage, const Vector2d& knockback) override;

    int GetMaxHP() const override { return 150; }
    Status GetStatus() const { return m_status; }
    Awareness GetAwareness() const { return m_awareness; }

private:
    PlayerEntity* FindPlayer() const;
    bool CanSeePlayer(const PlayerEntity* player) const;
    void ChangeStatus(Status nextStatus);
    void PlayMotion(const std::string& name, const char* texturePath, int frameCount);
    void UpdateIdle(PlayerEntity* player);
    void UpdateMove(PlayerEntity* player);
    void UpdateAttack(float deltaTime, PlayerEntity* player);
    void UpdateDead();
    void TryHitPlayer(PlayerEntity* player);

    std::string GetTexturePath() const override;

    Status m_status;
    Awareness m_awareness;
    float m_stateTimer;
    bool m_faceRight;
    bool m_attackHit;
    bool m_attackRecoveryStarted;
    std::string m_currentTexturePath;
    SoundComponent* m_hitSound;
};

