#pragma once

#include "EnemyEntity.h"


class AnimationComponent;
class SoundComponent;

class SpiritEnemyEntity : public EnemyEntity
{
public:
    static bool PreloadTextures();
	SpiritEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;
	void TakeDamage(
		int damage,
		const Vector2d& attackerPos,
		bool attackDir = false
	) override;
	
	void OnDead() override;

protected:
	std::string GetTexturePath() const override;
	void UpdateAI() override;
private:
	enum State
	{
		Idle,
		Move,
		CombatMove,
		Attack,
		Recovery,
		Hit,
		Dead
	};
	State m_state = Idle;

	// アニメーション
	AnimationComponent* m_animation;


	// アニメーション切り替え
	void UpdateAnimation();
	SoundComponent* m_attackSound = nullptr;

	float m_floatTimer = 0.0f;
	float m_attackTimer = 0.0f;
	float m_deadTimer = 0.0f;

	float m_detectRange = 700.0f;
	float m_keepDistance = 500.0f;
	float m_moveSpeed = 500.0f;
	float m_groundY;
	bool m_faceRight = true;
	bool m_isDying = false;
	bool m_isBlockedByWall = false;
	Vector2d m_blockedPlayerPosition = Vector2d::Zero();
	Vector2d m_targetPosition = Vector2d::Zero();
	bool m_hasTargetPosition = false;
	bool IsPlayerInSearchRange() const override;
	
	bool m_wallBlocked = false;
	bool m_isAvoidingWall = false;

	float m_wallBlockedTimer = 0.0f;
	float m_wallAvoidTimer = 0.0f;

	float m_wallAvoidDirection = -1.0f;
};