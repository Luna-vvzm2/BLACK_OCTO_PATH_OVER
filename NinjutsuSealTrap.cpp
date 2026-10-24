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
	,m_duration(NINJUTSU_SEAL_DURATION)
{
}

bool NinjutsuSealTrap::Init()
{
	if (!EntityActor::Init()) return false;

	if(m_collision)
	{
		m_collision->SetRect(m_initialSize.x, m_initialSize.y);
	}

	return true;
}

void NinjutsuSealTrap::Update(float deltaTime)
{
	EntityActor::Update(deltaTime);

	PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
	if (!playScene) return;

	PlayerEntity* player = playScene->GetPlayer();
	if (!player) return;

	CollisionComponent* playerCol = player->GetCollision();
	if (!m_collision || !playerCol) return;

	//ÚG‚µ‚½‚çÁ–Å
	if (m_collision->CheckCollision(playerCol))
	{
		std::cout << "[NinjutsuSealTrap] ‹N“® " << m_duration << " sec\n";   
		player->ApplyNinjutsuSeal(m_duration);
		Destroy();
	}
}