#include "WolfEnemyEntity.h"

#include "Actor.h"
#include "AnimationComponent.h"
#include "CollisionComponent.h"
#include "HPComponent.h"
#include "PlayerEntity.h"
#include "Scene.h"
#include "SoundComponent.h"
#include "SpriteComponent.h"
#include "VelocityComponent.h"
#include "AlertGaugeComponent.h"

#include <cmath>

namespace
{
	constexpr float WOLF_BODY_WIDTH = 300.0f;
	constexpr float WOLF_BODY_HEIGHT = 200.0f;
	constexpr float WOLF_SIGHT_RANGE_X = 800.0f;
	constexpr float WOLF_SIGHT_RANGE_Y = 120.0f;
	constexpr float WOLF_MOVE_SPEED = 600.0f;

	constexpr float WOLF_ATTACK_WIDTH = 3500.0f;
	constexpr float WOLF_ATTACK_HEIGHT = 3500.0f;
	constexpr float WOLF_ATTACK_DAMAGE = 5;
	constexpr float WOLF_ATTACK_HIT_TIME = 0.5f;
	constexpr float WOLF_ROAR_DURATION = 5.0f;
	constexpr float WOLF_ATTACK_RECOVERY_TIME = 2.5f;
	constexpr float WOLF_ATTACK_KNOCKBACK = 250.0f;

	constexpr float WOLF_ALLY_HEAL_AMOUNT = 15.0f;
	constexpr float WOLF_ALLY_HEAL_INTERVAL = 1.0f;
	constexpr float WOLF_ALLY_BUFF_MULTIPLIER = 1.2f;
	constexpr float WOLF_ALLY_BUFF_DURATION = 8.0f;

	constexpr float WOLF_IDLE_FRAME_TIME = 9.0f / 60.0f;
	constexpr float WOLF_MOVE_FRAME_TIME = 6.0f / 60.0f;
	constexpr float WOLF_ATTACK_FRAME_TIME = 60.0f / 60.0f;
	constexpr float WOLF_DEAD_FRAME_TIME = 14.0f / 60.0f;
	constexpr float WOLF_DEAD_MOTION_TIME = WOLF_DEAD_FRAME_TIME * 7.0f;

	const char* WOLF_TEXTURE_IDLE = "";
	const char* WOLF_TEXTURE_MOVE = "";
	const char* WOLF_TEXTURE_ATTACK = "";
	const char* WOLF_TEXTURE_DEAD = "";
}

WolfEnemyEntity::WolfEnemyEntity(Scene* scene, const Vector2d& pos)
	:EnemyEntity(scene, pos, Vector2d(WOLF_BODY_WIDTH, WOLF_BODY_HEIGHT))
	, m_status(Status::Idle)
	, m_awareness(Awareness::Undetected)
	, m_stateTimer(0.0f)
	, m_faceRight(true)
	, m_attackHit(false)
	, m_attackRecoveryStarted(false)
	, m_attackSound(nullptr)
{
}

bool WolfEnemyEntity::Init()
{
	if (!EnemyEntity::Init())
	{
		return false;
	}
	SetEnemyRank(EnemyRank::Elite);
	m_anim = AddComponent<AnimationComponent>();
	if (m_anim == nullptr || m_sprite == nullptr)
	{
		return false;
	}

	m_anim->SetSprite(m_sprite);
	m_attackSound = AddComponent<SoundComponent>();

	AnimationClip idle;
	idle.frames = { 0,1,2 };
	idle.frameDurations = { };
	idle.loop = true;
	m_anim->AddClip("idle", idle);

	AnimationClip move;
	move.frames = { 0, 1, 2 };
	move.frameDurations = { };
	move.loop = true;
	m_anim->AddClip("move", move);

	AnimationClip attack;
	attack.frames = { 0, 1, 2, 3, 4 ,5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
	attack.frameDurations = { };
	attack.loop = false;
	m_anim->AddClip("attack", attack);

	AnimationClip dead;
	dead.frames = { 0, 1, 2, 3, 4, 5, 6 };
	dead.frameDurations = { };
	dead.loop = false;
	m_anim->AddClip("dead", dead);

	PlayMotion("idle", WOLF_TEXTURE_IDLE, 3);
	return true;

}

void WolfEnemyEntity::Update(float deltaTime)
{
	if (m_status == Status::Defeated)
	{
		m_stateTimer += deltaTime;

		if (m_velocity != nullptr)
		{
			Vector2d velocity = m_velocity->GetVelocity();
			velocity.x = 0.0f;
			m_velocity->SetVelocity(velocity);
		}

		EnemyEntity::Update(deltaTime);
		UpdateDead();
		return;
	}

	PlayerEntity* player = FindPlayer();
	m_stateTimer += deltaTime;

	switch (m_status)
	{
	case Status::Idle:
		UpdateIdle(player);
		break;
	case Status::Move:
		UpdateMove(player);
		break;
	case Status::Attack:
		UpdateAttack(deltaTime, player);
		break;
	case Status::Defeated:
		break;
	}

	if (m_sprite != nullptr)
	{
		m_sprite->SetFlipH(!m_faceRight);
	}

	EnemyEntity::Update(deltaTime);
}

PlayerEntity* WolfEnemyEntity::FindPlayer() const
{
	if (m_scene == nullptr)
	{
		return nullptr;
	}

	for (Actor* actor : m_scene->GetActors())
	{
		if (actor != nullptr && actor->GetType() == ActorType::Player && !actor->IsDead())
		{
			return static_cast<PlayerEntity*>(actor);
		}
	}

	return nullptr;
}

bool WolfEnemyEntity::CanSeePlayer(const PlayerEntity* player) const
{
	if (player == nullptr)
	{
		return false;
	}

	if (player->GetIsHide()) { return false; }

	const Vector2d difference = player->GetPos() - GetPos();
	return std::fabs(difference.x) <= WOLF_SIGHT_RANGE_X &&
		std::fabs(difference.y) <= WOLF_SIGHT_RANGE_Y;
}

void WolfEnemyEntity::ChangeStatus(Status nextStatus)
{
	if (m_status == nextStatus)
	{
		return;
	}

	m_status = nextStatus;
	m_stateTimer = 0.0f;

	switch (m_status)
	{
	case Status::Idle:
		PlayMotion("idle", WOLF_TEXTURE_IDLE, 3);
		break;
	case Status::Move:
		PlayMotion("move", WOLF_TEXTURE_MOVE, 3);
		break;
	case Status::Attack:
		m_attackHit = false;
		m_attackRecoveryStarted = false;
		m_roarDamageTimer = 0.0f;
		m_roarHealTimer = 0.0f;
		m_roarBuffApplied = false;

		PlayMotion("attack", WOLF_TEXTURE_ATTACK, 15);
		if (m_attackSound != nullptr)
		{
			m_attackSound->Play("");
		}
		break;
	case Status::Defeated:
		PlayMotion("dead", WOLF_TEXTURE_DEAD, 7);
		break;
	}
}

void WolfEnemyEntity::PlayMotion(
	const std::string& name,
	const char* texturePath,
	int frameCount)
{
	if (m_sprite == nullptr || m_anim == nullptr)
	{
		return;
	}

	if (m_currentTexturePath != texturePath)
	{
		if (!m_sprite->LoadTextureDiv(texturePath, frameCount, 1))
		{
			return;
		}
		m_currentTexturePath = texturePath;
	}
	m_anim->Play(name, true);
}

void WolfEnemyEntity::UpdateIdle(PlayerEntity* player)
{
	if (m_velocity != nullptr)
	{
		Vector2d velocity = m_velocity->GetVelocity();
		velocity.x = 0.0f;
		m_velocity->SetVelocity(velocity);
	}

	if (!CanSeePlayer(player))
	{
		m_awareness = Awareness::Undetected;
		return;
	}

	if (m_alertGauge == nullptr || m_alertGauge->GetGauge() < 0.0f)
	{
		m_awareness = Awareness::Undetected;
		return;
	}
	const Vector2d difference = player->GetPos() - GetPos();
	const float attackDistance = WOLF_BODY_WIDTH * 0.5f + WOLF_ATTACK_WIDTH;
	if (std::fabs(difference.x) <= attackDistance &&
		std::fabs(difference.y) <= WOLF_ATTACK_HEIGHT)
	{
		m_faceRight = difference.x >= 0.0f;
		ChangeStatus(Status::Attack);
	}
	else
	{
		ChangeStatus(Status::Move);
	}
}

void WolfEnemyEntity::UpdateMove(PlayerEntity* player)
{
	if (!CanSeePlayer(player))
	{
		m_awareness = Awareness::Undetected;
		ChangeStatus(Status::Idle);
		return;
	}

	const Vector2d difference = player->GetPos() - GetPos();
	const float attackDistance = WOLF_BODY_WIDTH * 0.5f + WOLF_ATTACK_WIDTH;
	if (std::fabs(difference.x) <= attackDistance &&
		std::fabs(difference.y) <= WOLF_ATTACK_HEIGHT)
	{
		if (m_velocity != nullptr)
		{
			Vector2d velocity = m_velocity->GetVelocity();
			velocity.x = 0.0f;
			m_velocity->SetVelocity(velocity);
		}
		m_faceRight = difference.x >= 0.0f;
		ChangeStatus(Status::Attack);
		return;
	}

	m_faceRight = difference.x >= 0.0f;
	if (m_velocity != nullptr)
	{
		Vector2d velocity = m_velocity->GetVelocity();
		velocity.x = m_faceRight ? WOLF_MOVE_SPEED : -WOLF_MOVE_SPEED;
		m_velocity->SetVelocity(velocity);
	}
}

void WolfEnemyEntity::UpdateAttack(float deltaTime, PlayerEntity* player)
{

	if (m_velocity != nullptr)
	{
		Vector2d velocity = m_velocity->GetVelocity();
		velocity.x = 0.0f;
		m_velocity->SetVelocity(velocity);
	}

	if (!m_roarBuffApplied)
	{
		ApplyRoarToAllies();
		m_roarBuffApplied = true;
	}

	if (m_stateTimer < WOLF_ROAR_DURATION)
	{
		m_roarDamageTimer += deltaTime;
		m_roarHealTimer += deltaTime;

		while (m_roarDamageTimer >= WOLF_ATTACK_HIT_TIME)
		{
			TryHitPlayer(player);
			m_roarDamageTimer -= WOLF_ATTACK_HIT_TIME;
		}

		while (m_roarHealTimer >= WOLF_ALLY_HEAL_INTERVAL)
		{
			HealNearbyAllies();
			m_roarHealTimer -= WOLF_ALLY_HEAL_INTERVAL;
		}

		return;
	}
	

	if (!m_attackRecoveryStarted)
	{
		m_attackRecoveryStarted = true;
		PlayMotion("idle", WOLF_TEXTURE_IDLE, 3);
	}

	if (m_stateTimer >= WOLF_ROAR_DURATION + WOLF_ATTACK_RECOVERY_TIME)
	{
		ChangeStatus(Status::Idle);
	}
}

void WolfEnemyEntity::UpdateDead()
{
	if (m_stateTimer >= WOLF_DEAD_MOTION_TIME)
	{
		OnDead();
	}
}

void WolfEnemyEntity::TryHitPlayer(PlayerEntity* player)
{
	if (player == nullptr || player->GetCollision() == nullptr)
	{
		return;
	}

	const Vector2d playerPos = player->GetPos();
	const Vector2d attackCenter = GetPos();

	const float combinedHalfWidth =
		WOLF_ATTACK_WIDTH * 0.5f + player->GetCollision()->GetWidth() * 0.5f;
	const float combinedHalfHeight =
		WOLF_ATTACK_HEIGHT * 0.5f + player->GetCollision()->GetHeight() * 0.5f;

	if (std::fabs(playerPos.x - attackCenter.x) <= combinedHalfWidth &&
		std::fabs(playerPos.y - attackCenter.y) <= combinedHalfHeight)
	{
		const float knockbackX = m_faceRight ? WOLF_ATTACK_KNOCKBACK : -WOLF_ATTACK_KNOCKBACK;
		player->TakeSlipDamage(static_cast<int>(WOLF_ATTACK_DAMAGE));
	}
}

void WolfEnemyEntity::TakeDamage(int damage, const Vector2d& knockback, bool attackDir)
{
	if (m_hp == nullptr || m_status == Status::Defeated)
	{
		return;
	}

	EnemyEntity::TakeDamage(damage, knockback, attackDir);

	if (m_hp->GetHP() <= 0)
	{
		if (m_velocity != nullptr)
		{
			Vector2d velocity = m_velocity->GetVelocity();
			velocity.x = 0.0f;
			m_velocity->SetVelocity(velocity);
		}

		ChangeStatus(Status::Defeated);
	}
}

bool WolfEnemyEntity::IsPlayerInSearchRange() const
{
	PlayerEntity* player = FindPlayer();
	if (player == nullptr || player->IsDead())
	{
		return false;
	}

	const Vector2d difference = player->GetPos() - GetPos();

	return std::fabs(difference.x) <= WOLF_SIGHT_RANGE_X &&
		std::fabs(difference.y) <= WOLF_SIGHT_RANGE_Y;
}

std::string WolfEnemyEntity::GetTexturePath() const
{
	return WOLF_TEXTURE_IDLE;
}

void WolfEnemyEntity::HealNearbyAllies()
{
	if (m_scene == nullptr)
	{
		return;
	}

	for (Actor* actor : m_scene->GetActors())
	{
		if (actor == nullptr || actor == this ||
			actor->GetType() != ActorType::Enemy ||
			actor->IsDead())
		{
			continue;
		}

		EnemyEntity* enemy = dynamic_cast<EnemyEntity*>(actor);
		if (enemy == nullptr)
		{
			continue;
		}

		const Vector2d difference = enemy->GetPos() - GetPos();

		if (std::fabs(difference.x) > WOLF_ATTACK_WIDTH ||
			std::fabs(difference.y) > WOLF_ATTACK_HEIGHT )
		{
			continue;
		}

		HPComponent* hp = enemy->GetHP();

		if (hp != nullptr && !hp->IsDead())
		{
			hp->Heal(static_cast<int>(WOLF_ALLY_HEAL_AMOUNT));
		}
	}
}

void WolfEnemyEntity::ApplyRoarToAllies()
{
	if (m_scene == nullptr)
	{
		return;
	}

	for (Actor* actor : m_scene->GetActors())
	{
		if (actor == nullptr || actor == this ||
			actor->GetType() != ActorType::Enemy ||
			actor->IsDead())
		{
			continue;
		}

		EnemyEntity* enemy = dynamic_cast<EnemyEntity*>(actor);
		if (enemy == nullptr)
		{
			continue;
		}

		const Vector2d difference = enemy->GetPos() - GetPos();

		if (std::fabs(difference.x) > WOLF_ATTACK_WIDTH * 0.5f ||
			std::fabs(difference.y) > WOLF_ATTACK_HEIGHT * 0.5f)
		{
			continue;
		}

		enemy->ApplyAttackPowerBuff(
			WOLF_ALLY_BUFF_MULTIPLIER,
			WOLF_ALLY_BUFF_DURATION
		);
	}
}