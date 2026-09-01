#pragma once
#include "Component.h"
#include <functional>

class HPComponent : public Component
{
public:
    HPComponent(Actor* owner, int maxHP);

    void Update(float deltaTime) override;

    // --- HP操作 ---
    void Damage(int amount);
    void Heal(int amount);

    int GetHP() const { return m_hp; }
    int GetMaxHP() const { return m_maxHP; }
    bool IsDead() const { return m_hp <= 0; }
    bool IsInvincible() const { return m_invincibleTimer > 0.0f; }

    // 無敵時間をセット（ダメージ後 0.3s など）
    void SetInvincible(float time) { m_invincibleTimer = time; }

    // --- コールバック ---
    std::function<void(int newHP, int oldHP)> OnHPChanged;
    std::function<void()> OnDeath;

private:
    Actor* m_owner = nullptr;
    int m_maxHP;
    int m_hp;

    float m_invincibleTimer;   // 無敵時間（秒）
};

