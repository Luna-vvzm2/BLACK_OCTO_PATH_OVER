#pragma once

#include "Vector2d.h"

class AddEnemySpawner
{
public:
	AddEnemySpawner(class Scene* Scene);
	~AddEnemySpawner();

	void Update(float deltaTime);

private:
	enum class EnemyKind
	{
		Kooni,
		Spirit,
		KasaTengu,
		Count
	};

	float m_interval;

	Scene* m_scene;
	Vector2d m_spawnPos;

	Vector2d ChooseSpawnPos(const class PlayerEntity* player) const;
	bool IsSpawnPosFree(const Vector2d& pos) const;

};