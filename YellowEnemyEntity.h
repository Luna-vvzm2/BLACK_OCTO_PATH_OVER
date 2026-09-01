
#pragma once
#include "EnemyEntity.h"
#include "CollisionComponent.h"

class AnimationComponent;

struct AttackRect
{
	int x;
	int y;
	int w;
	int h;
};
class YellowEnemyEntity : public EnemyEntity
{
public:
	YellowEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;
	std::string GetTexturePath() const override;
	void TakeDamage(int damage, const Vector2d& knockback);
private:
	enum State {
		Idle,
		Dead,
		Chase,
		AttackReady,
		Attack1,
		Attack2,
		Recovery,
		BlowHit

	};

	State m_state = Idle;

	float m_attackTimer = 0.0f;
	float m_detectRange = 400.0f;
	float m_attackRange = 200.0f;
	float m_deadTimer = 0.0f;
	float m_hitTimer = 0.0f;
	CollisionComponent* m_attackCollision;

	bool m_attackHit = false;

	float m_damageTimer = 0.0f;
	int m_damagePerSecond = 10;
	bool m_faceRight = true;
	bool m_isHit = false;
	bool m_isDying = false;      // éÄñSèàóùíÜÇ©

	AttackRect m_attackRect = { 0,0,0,0 };
	AnimationComponent* m_animation = nullptr;

};

