#pragma once
#include "EnemyEntity.h"
#include <vector>

class PlayerEntity;

class WhiteEnemyEntity : public EnemyEntity
{
public:
	WhiteEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void TakeDamage(int damage, const Vector2d& knockback) override;

	std::string GetTexturePath() const override;

private:
	bool TryGetPlayerInfo(Vector2d& playerPos, PlayerEntity*& player, bool& playerOnGround) const;
	void DefineAnimationClips();
	void PlaySheetMotion(const std::string& motionName);
	void StartShurikenAttack();
	void StartSwordAttack();
	void StartWeakHit();
	void StartBlowHit(const Vector2d& knockback);
	void StartLargeBlowHit(const Vector2d& knockback);
	void StartDeadHit();
	void PlayDamageMotion(
		const std::string& motionName,
		const std::string& texturePath,
		int xNum,
		int yNum
	);
	void UpdateDamageMotion(float deltaTime);
	void CancelAttackForDamage();

	float GetDirSign() const;
	void SetFacing(bool flipH);
	void PrepareMoveTracking(float wantedMoveX);

private:
	int m_bulletCount;
	bool m_attackOnce;
	int m_whiteState;
	std::string m_currentTexturePath;
	std::string m_currentMotionName;
	bool m_chasePlayer;
	bool m_hasLastMove;
	float m_lastWantedMoveX;
	float m_wallStopTimer;
	Vector2d m_lastMoveStartPos;
	int m_damageState;
	float m_damageFrameTimer;
	bool m_damageHoldGroundFrame;
};
