#include "NinjutsuSealTrap.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "CollisionComponent.h"

namespace
{
	constexpr float	NINJUTSU_SEAL_DURATION = 10.0f;
}

NinjutsuSealTrap::NinjutsuSealTrap(Scene* scene, const Vector2d& pos, const Vector2d& size)
	:EntityActor(scene, pos, size)
	, m_triggered(false)
	,m_duration(NINJUTSU_SEAL_DURATION)
{
}

bool NinjutsuSealTrap::Init()
{
	if (!EntityActor::Init()) return false;

	if (m_collision) {
		m_collision->SetRect(m_initialSize.x, m_initialSize.y);
	}

	m_effect.Init(m_sprite, GetTransform());   // 起動前のエフェクト(ループ)を開始

	return true;
}

void NinjutsuSealTrap::Update(float deltaTime)
{
	EntityActor::Update(deltaTime);
	m_effect.Update(deltaTime);

	// 起動後は、起動エフェクトの再生が終わったら消える
	if (m_triggered) {
		if (m_effect.IsActivateFinished()) {
			Destroy();
		}
		return;
	}

	PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
	if (!playScene) return;

	PlayerEntity* player = playScene->GetPlayer();
	if (!player) return;

	CollisionComponent* playerCol = player->GetCollision();
	if (!m_collision || !playerCol) return;

	//接触したら消滅
	if (m_collision->CheckCollision(playerCol))
	{
		std::cout << "[NinjutsuSealTrap] 起動 " << m_duration << " sec\n";   
		player->ApplyNinjutsuSeal(m_duration);
		m_triggered = true;
		m_effect.PlayActivate();   // 起動時のエフェクトを再生(終わったら消える)
	}
}