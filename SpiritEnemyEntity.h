#pragma once

#include "EnemyEntity.h"


class AnimationComponent;

class SpiritEnemyEntity : public EnemyEntity
{
public:
	SpiritEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

	std::string GetTexturePath() const override;



protected:
	void UpdateAI() override;

private:
	enum State
	{
		Idle,
		Move,
		Attack,
		Recovery,
		Dead
	};
	State m_state = Idle;

	AnimationComponent* m_animation = nullptr;

	float m_floatTimer = 0.0f;
	float m_attackTimer = 0.0f;
	float m_deadTimer = 0.0f;

	float m_detectRange = 700.0f;
	float m_keepDistance = 500.0f;
	float m_moveSpeed = 500.0f;

	bool m_faceRight = true;
	bool m_isDying = false;
};