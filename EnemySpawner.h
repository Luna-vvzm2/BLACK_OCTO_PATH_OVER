#pragma once
#include "PlayScene.h"
#include "EnemyEntity.h"
#include "EnemyHPBar.h"
#include <type_traits>

class EnemySpawner
{
public:
	template <typename EnemyType>
	static EnemyType* SpawnEnemy(PlayScene* scene, const Vector2d& pos, const std::string& hpBarPath = "assets/images/uies/HP_enemy_black.png", const Vector2d& size = Vector2d(192, 192))
	{
		if (!scene) return nullptr;

		EnemyType* enemy = nullptr;
		if constexpr (std::is_constructible_v<EnemyType, Scene*, Vector2d, Vector2d>)
		{
			enemy = new EnemyType(scene, pos, size);
		}
		else
		{
			enemy = new EnemyType(scene, pos);
		}
		scene->AddActor(enemy);

		EnemyHPBar* hpBar = new EnemyHPBar(scene, enemy->GetHP(), hpBarPath);
		hpBar->SetPosIsCenter(false);
		hpBar->SetFrameOffset(80.0f, 50.0f);
		hpBar->SetGaugeScale(0.8f, 0.05f);
		hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
		hpBar->SetGaugeOffset(0.0f, 0.0f);
		scene->AddUIActorFromExternal(hpBar);

		scene->RegisterEnemyHPBar(enemy, hpBar);

		enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
			if (hpBar && newHP < oldHP) {
				hpBar->ShowFor(0.0f);
			}
			};

		enemy->GetHP()->OnDeath = [hpBar]() {
			if (hpBar) hpBar->SetState(Actor::State::Dead);
			};

		return enemy;
	}
};

