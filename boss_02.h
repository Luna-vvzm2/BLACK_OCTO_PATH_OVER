#pragma once
#include "BossEntity.h"
#include <functional>
#include <vector>

// Ginjo: base status, idle, approach and defeat only (60 motion frames/second).
class Boss_02 : public BossEntity
{
public:
    enum class Behavior { Idle, Move, Defeated };
    struct Parameters
    {
        int maxHP = 3000;
        Vector2d hitboxSize = { 120.0f, 180.0f };
        // Jet speed: faster than player ground dash (600) and air dash (800).
        float moveSpeed = 1000.0f;
        // Temporary edge-to-edge distances; editable when attacks are added.
        float approachStopDistance = 80.0f;
        float approachStartDistance = 120.0f;
        // Robot reaction delay after detecting a distant player (temporary value).
        float approachDelaySeconds = 1.2f;
        float knockbackSeconds = 0.4f;
        bool showDebugShapes = true;
    };

    explicit Boss_02(Scene* scene, const Vector2d& pos = Vector2d::Zero());
    Boss_02(Scene* scene, const Vector2d& pos, const Parameters& parameters);
    bool Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    int GetMaxHP() const override { return m_parameters.maxHP; }
    void TakeDamage(int damage, const Vector2d& knockback) override;
    void TakeMetsu(int metsu) override;
    void OnDead() override;
    Behavior GetBehavior() const { return m_behavior; }
    bool IsBattleFinished() const { return m_behavior == Behavior::Defeated; }
    const Parameters& GetParameters() const { return m_parameters; }

    // Call after Init. Borrowed DxLib handles: idle=3, move=3, defeat=7.
    // Keep textures alive until this actor is removed. No automatic asset loading.
    bool SetAnimationFrames(const std::vector<int>& idle, const std::vector<int>& move,
        const std::vector<int>& defeated, bool sourceFacesRight = true);
    // Notification after the full 14-frame defeat animation. Do not delete here;
    // queue scene changes instead. Scene removes this actor on its normal cleanup.
    std::function<void()> OnBattleFinished = {};
    static void SpawnForTest(Scene* scene, const Vector2d& pos);

protected:
    void UpdateAI(float deltaTime) override;
    void UpdateAttack(float) override {} // Reserved for the next task.

private:
    void ChangeBehavior(Behavior behavior);
    void PlayBehaviorAnimation();
    Parameters m_parameters;
    Behavior m_behavior = Behavior::Idle;
    float m_behaviorTime = 0.0f;
    float m_knockbackRemaining = 0.0f;
    float m_approachWaitTime = 0.0f;
    float m_jetTargetX = 0.0f; // Locked at launch; this movement does not deal damage.
    bool m_initialized = false;
    bool m_sourceFacesRight = true;
};
