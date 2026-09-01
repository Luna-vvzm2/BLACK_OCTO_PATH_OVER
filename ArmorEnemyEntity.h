#pragma once
#include "EnemyEntity.h"

class PlayerEntity;

class ArmorEnemyEntity : public EnemyEntity
{
public:
	ArmorEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void TakeDamage(int damage, const Vector2d& knockback) override;

	std::string GetTexturePath() const override;
	int GetGuard() const { return m_guard; }
	int GetGuardMax() const { return m_guardMax; }
	bool IsGuardBroken() const { return m_guard <= 0; }

private:
	bool TryGetPlayerInfo(Vector2d& playerPos, PlayerEntity*& player) const;
	void UpdateGuardRecover(float deltaTime);
	void PlayMotion(
		const std::string& motionName,
		bool reset = false
	);
	void DefineAnimationClips();
	void StartArmorsAttack();
	void TriggerExplosion(const Vector2d& playerPos, PlayerEntity* player);
	void OnHPChanged(int newHP, int oldHP);
	void StartDamageMotion(const Vector2d& knockback);
	void StartDeadMotion();
	void StartTurnMotion();
	void UpdateDeadMotion(float deltaTime);
	void UpdateTurnMotion(float deltaTime);
	void BreakGuard(const Vector2d& knockback);
	void RecoverGuard();
	float GetDirSign() const;
	void SetFacing();

private:
	bool m_attackOnce;
	int m_armorState;
	float m_guardRecoverTimer;
	bool m_ignoreHPChange;
	bool m_guardRecovering;
	float m_guardRecoverMotionTimer;
	float m_knockbackTimer;
	bool m_deadMotion;
	float m_deadMotionTimer;
	bool m_turning;
	float m_turnTimer;
	std::string m_currentMotionName;
};
