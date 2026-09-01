#pragma once
#include "EntityActor.h"
#include <string>

class HPComponent;

class EnemyBullet : public EntityActor
{
public:
	EnemyBullet(
		Scene* scene,
		const Vector2d& pos,
		const Vector2d& velocity,
		float deleteRange,
		const std::string& texturePath,
		int damage = 10,
		const Vector2d& drawSize = Vector2d::Zero(),
		bool rotate = false,
		float rotateInterval = 0.0f,
		float rotateStep = 0.0f
	);

	bool Init() override;
	void Update(float deltaTime) override;

	ActorType GetType() const override { return ActorType::Ball; }

private:
	bool TryDamagePlayer();
	std::string GetTexturePath() const override;

private:
	Vector2d m_startPos;
	Vector2d m_bulletVelocity;
	float m_deleteRange;
	std::string m_texturePath;
	int m_damage;
	Vector2d m_drawSize;
	bool m_rotate;
	float m_rotateInterval;
	float m_rotateStep;
	float m_rotationAngle;
	float m_rotationTimer;
};