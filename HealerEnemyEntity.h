#pragma once
#include "EnemyEntity.h"

class HealerEnemyEntity : public EnemyEntity
{
public:
	HealerEnemyEntity(Scene* scene, const Vector2d& pos);

	bool Init() override;
	void Update(float deltaTime) override;

	std::string GetTexturePath() const override;

private:
	int m_bulletCount;
};

