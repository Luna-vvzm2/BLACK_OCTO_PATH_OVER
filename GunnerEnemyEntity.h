#pragma once
#include "EnemyEntity.h"

class GunnerEnemyEntity : public EnemyEntity
{
public:
	GunnerEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void TakeDamage(int damage, const Vector2d& knockback) override;

	std::string GetTexturePath() const override;

private:
	bool TryGetPlayerInfo(Vector2d& playerPos) const;
	void PlayMotion(const std::string& motionName, bool reset = false);
	void DefineAnimationClips();
	void StartGunAttack(const Vector2d& playerPos);
	void StartTurnMotion();
	void StartDamageMotion(const Vector2d& knockback, int damage, bool dead);
	void UpdateTurnMotion(float deltaTime);
	void UpdateDamageMotion(float deltaTime);
	float GetDirSign() const;
	void SetFacing();
	void SetReverseFacing();

private:
	int m_bulletCount;
	bool m_attackOnce[3];
	int m_attackDir;
	int m_gunnerState;
	int m_damageType;
	bool m_damageDead;
	float m_damageTimer;
	bool m_turning;
	float m_turnTimer;
	std::string m_currentMotionName;
};
