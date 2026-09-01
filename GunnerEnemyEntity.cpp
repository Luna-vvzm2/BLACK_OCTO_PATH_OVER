#include "GunnerEnemyEntity.h"
#include "Scene.h"
#include "Actor.h"
#include "EntityActor.h"
#include "CollisionComponent.h"
#include "PlayerEntity.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include <cmath>

class GunnerBullet : public EntityActor
{
public:
	GunnerBullet(
		Scene* scene,
		const Vector2d& pos,
		const Vector2d& velocity,
		float deleteRange,
		const std::string& texturePath,
		int damage
	)
		: EntityActor(scene, pos, Vector2d(12.0f, 12.0f))
		, m_startPos(pos)
		, m_bulletVelocity(velocity)
		, m_deleteRange(deleteRange)
		, m_texturePath(texturePath)
		, m_damage(damage)
	{
	}

	bool Init() override
	{
		if (!EntityActor::Init()) return false;
		m_velocity->SetVelocity(m_bulletVelocity);
		m_collision->SetCircle(6.0f);
		return true;
	}

	void Update(float deltaTime) override
	{
		Actor::Update(deltaTime);

		Vector2d pos = m_transform->GetPosition();
		pos += m_velocity->GetVelocity() * deltaTime;
		m_transform->SetPosition(pos);

		if ((pos - m_startPos).length() > m_deleteRange || TryDamagePlayer())
		{
			SetState(Actor::State::Dead);
		}
	}

	ActorType GetType() const override { return ActorType::Ball; }

private:
	bool TryDamagePlayer()
	{
		for (Actor* actor : m_scene->GetActors())
		{
			if (actor == nullptr || actor->GetType() != ActorType::Player || actor->IsDead())
			{
				continue;
			}

			CollisionComponent* playerCollision = actor->GetComponent<CollisionComponent>();
			if (playerCollision == nullptr || !m_collision->CheckCollision(playerCollision))
			{
				continue;
			}

			PlayerEntity* player = static_cast<PlayerEntity*>(actor);
			float knockbackX = player->GetPos().x < m_transform->GetPosition().x ? -220.0f : 220.0f;
			player->TakeDamage(m_damage, Vector2d(knockbackX, -120.0f));
			return true;
		}

		return false;
	}

	std::string GetTexturePath() const override { return m_texturePath; }

	Vector2d m_startPos;
	Vector2d m_bulletVelocity;
	float m_deleteRange;
	std::string m_texturePath;
	int m_damage;
};

static const char* GUNNER_TEXTURE_SHEET = "assets/images/enemy/gunner/gunner_sheet.png";
static const int GUNNER_SHEET_X_NUM = 10;
static const int GUNNER_SHEET_Y_NUM = 4;

GunnerEnemyEntity::GunnerEnemyEntity(Scene* scene, const Vector2d& pos)
	: EnemyEntity(scene, pos, Vector2d(96, 190))
	, m_bulletCount(0)
	, m_attackOnce{}
	, m_attackDir(1)
	, m_gunnerState(0)
	, m_damageType(0)
	, m_damageDead(false)
	, m_damageTimer(0.0f)
	, m_turning(false)
	, m_turnTimer(0.0f)
	, m_currentMotionName("")
{
}

bool GunnerEnemyEntity::Init()
{
	m_hpMax = 200;
	if (!EnemyEntity::Init()) return false;

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);
	if (m_sprite != nullptr)
	{
		m_sprite->LoadTextureDiv(GUNNER_TEXTURE_SHEET, GUNNER_SHEET_X_NUM, GUNNER_SHEET_Y_NUM);
		m_sprite->SetDrawSize(96.0f, 190.0f);
	}
	DefineAnimationClips();
	PlayMotion("idle", true);

	return true;
}

static const float GUNNER_FIND_RANGE = 800.0f;          // search range
static const float GUNNER_BACK_RANGE = 220.0f;          // back step range
static const float GUNNER_STOP_RANGE = 300.0f;          // stop attack range
static const float GUNNER_ATTACK_RANGE = 900.0f;        // attack range
static const float GUNNER_BULLET_DELETE_RANGE = 900.0f; // bullet delete range

static const float GUNNER_APPROACH_SPEED = 120.0f;      // approach speed
static const float GUNNER_BACK_SPEED = 180.0f;          // back speed
static const float GUNNER_BULLET_SPEED = 250.0f;        // bullet speed
static const float GUNNER_COOLDOWN = 0.4f;              // attack cooldown
static const float GUNNER_RECHECK_TIME = 0.2f;          // state recheck time

static const int GUNNER_SHOT_COUNT = 3;                 // shot count
static const float GUNNER_SHOT_TIME[GUNNER_SHOT_COUNT] = { 21.0f / 60.0f, 41.0f / 60.0f, 61.0f / 60.0f };
static const float GUNNER_RELOAD_START_TIME = 70.0f / 60.0f;
static const float GUNNER_ATTACK_END_TIME = 113.0f / 60.0f;
static const int GUNNER_BULLET_DAMAGE = 15;              // bullet damage
static const int GUNNER_BLOW_MIN_DAMAGE = 10;
static const int GUNNER_BLOW_LARGE_MIN_DAMAGE = 25;
static const int GUNNER_DAMAGE_NONE = 0;
static const int GUNNER_DAMAGE_SHORT = 1;
static const int GUNNER_DAMAGE_BLOW = 2;
static const int GUNNER_DAMAGE_BLOW_LARGE = 3;
static const float GUNNER_HIT_TOTAL_TIME = 9.0f / 60.0f;
static const float GUNNER_DAMAGE1_DEAD_TIME = 16.0f / 60.0f;
static const float GUNNER_DAMAGE1_TOTAL_TIME = 22.0f / 60.0f;
static const float GUNNER_DAMAGE2_DEAD_TIME = 25.0f / 60.0f;
static const float GUNNER_DAMAGE2_TOTAL_TIME = 31.0f / 60.0f;
static const float GUNNER_TURN_TIME = 4.0f / 60.0f;


float GunnerEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

void GunnerEnemyEntity::SetFacing()
{
	if (m_sprite == nullptr)
	{
		return;
	}

	m_sprite->SetFlipX(m_dir);
}

void GunnerEnemyEntity::SetReverseFacing()
{
	if (m_sprite == nullptr)
	{
		return;
	}

	m_sprite->SetFlipX(!m_dir);
}

void GunnerEnemyEntity::StartTurnMotion()
{
	m_turning = true;
	m_turnTimer = 0.0f;
	m_actionLock = true;
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
	SetFacing();
	PlayMotion("turn", true);
}

void GunnerEnemyEntity::UpdateTurnMotion(float deltaTime)
{
	if (!m_turning)
	{
		return;
	}

	m_turnTimer += deltaTime;
	if (m_turnTimer >= GUNNER_TURN_TIME)
	{
		m_turning = false;
		m_turnTimer = 0.0f;
		m_actionLock = false;
	}
}

void GunnerEnemyEntity::StartDamageMotion(const Vector2d& knockback, int damage, bool dead)
{
	m_attackType = 0;
	m_attackActive = false;
	m_actionLock = true;
	m_turning = false;
	m_damageDead = dead;
	m_damageTimer = 0.0f;

	Vector2d damageKnockback = knockback;
	Vector2d playerPos = Vector2d::Zero();
	if (TryGetPlayerInfo(playerPos))
	{
		Vector2d myPos = GetPos();
		float awaySign = myPos.x < playerPos.x ? -1.0f : 1.0f;
		damageKnockback.x = std::fabs(damageKnockback.x) * awaySign;
	}
	m_velocity->SetVelocity(damageKnockback);
	SetFacing();

	if (damage >= GUNNER_BLOW_LARGE_MIN_DAMAGE)
	{
		m_damageType = GUNNER_DAMAGE_BLOW_LARGE;
		PlayMotion("damage2", true);
	}
	else if (damage >= GUNNER_BLOW_MIN_DAMAGE)
	{
		m_damageType = GUNNER_DAMAGE_BLOW;
		PlayMotion("damage1", true);
	}
	else
	{
		m_damageType = GUNNER_DAMAGE_SHORT;
		PlayMotion("hit", true);
	}
}

void GunnerEnemyEntity::UpdateDamageMotion(float deltaTime)
{
	if (m_damageType == GUNNER_DAMAGE_NONE)
	{
		return;
	}

	m_damageTimer += deltaTime;

	float deadTime = GUNNER_HIT_TOTAL_TIME;
	float totalTime = GUNNER_HIT_TOTAL_TIME;
	if (m_damageType == GUNNER_DAMAGE_BLOW)
	{
		deadTime = GUNNER_DAMAGE1_DEAD_TIME;
		totalTime = GUNNER_DAMAGE1_TOTAL_TIME;
	}
	else if (m_damageType == GUNNER_DAMAGE_BLOW_LARGE)
	{
		deadTime = GUNNER_DAMAGE2_DEAD_TIME;
		totalTime = GUNNER_DAMAGE2_TOTAL_TIME;
	}

	if (m_damageDead && m_damageTimer >= deadTime)
	{
		OnDead();
		return;
	}

	if (!m_damageDead && m_damageTimer >= totalTime)
	{
		SetFacing();
		m_damageType = GUNNER_DAMAGE_NONE;
		m_damageTimer = 0.0f;
		m_actionLock = false;
	}
}

bool GunnerEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos) const
{
	if (m_scene == nullptr)
	{
		return false;
	}

	for (Actor* actor : m_scene->GetActors())
	{
		if (actor == nullptr || actor->GetType() != ActorType::Player || actor->IsDead())
		{
			continue;
		}

		TransformComponent* transform = actor->GetComponent<TransformComponent>();
		if (transform == nullptr)
		{
			return false;
		}

		playerPos = transform->GetPosition();
		return true;
	}

	return false;
}

void GunnerEnemyEntity::PlayMotion(const std::string& motionName, bool reset)
{
	if (m_anim == nullptr)
	{
		return;
	}

	if (!reset && m_currentMotionName == motionName)
	{
		return;
	}

	m_anim->Play(motionName, reset);
	m_currentMotionName = motionName;
}

void GunnerEnemyEntity::DefineAnimationClips()
{
	if (m_anim == nullptr)
	{
		return;
	}

	AnimationClip idle;
	idle.frames = { 0 };
	idle.frameDurations = { 10.0f / 60.0f };
	idle.loop = true;
	m_anim->AddClip("idle", idle);

	AnimationClip walk;
	walk.frames = { 1, 1, 1, 1, 2, 2 };
	walk.frameDurations = {
		2.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f
	};
	walk.loop = true;
	m_anim->AddClip("walk", walk);

	AnimationClip turn;
	turn.frames = { 3, 3, 4, 4 };
	turn.frameDurations = {
		1.0f / 60.0f,
		1.0f / 60.0f,
		1.0f / 60.0f,
		1.0f / 60.0f
	};
	turn.loop = false;
	m_anim->AddClip("turn", turn);

	AnimationClip attack;
	attack.frames = {
		5, 5, 5, 5, 6, 7, 7, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		9, 10, 10, 9, 9, 9, 9, 9, 9, 9,
		9, 9, 9, 9, 9, 9, 9, 9, 9, 8,
		9, 10, 10, 9, 9, 9, 9, 9, 9, 9,
		9, 9, 9, 9, 9, 9, 9, 9, 9, 8,
		9, 10, 10, 7, 7, 6, 5, 5, 5, 5,
		11, 11, 12, 12, 12, 12, 12, 12, 13, 13,
		14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		17, 17, 17, 17, 17, 18, 17, 17, 17, 17,
		11, 11, 11
	};
	attack.speed = 1.0f / 60.0f;
	attack.loop = false;
	m_anim->AddClip("attack", attack);

	AnimationClip hit;
	hit.frames = { 19, 20, 21 };
	hit.frameDurations = {
		3.0f / 60.0f,
		5.0f / 60.0f,
		1.0f / 60.0f
	};
	hit.loop = false;
	m_anim->AddClip("hit", hit);

	AnimationClip damage1;
	damage1.frames = { 19, 23, 26, 27, 28, 30, 31, 32, 33, 34 };
	damage1.frameDurations = {
		1.0f / 60.0f,
		3.0f / 60.0f,
		4.0f / 60.0f,
		1.0f / 60.0f,
		10.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		1.0f / 60.0f,
		2.0f / 60.0f,
		4.0f / 60.0f
	};
	damage1.loop = false;
	m_anim->AddClip("damage1", damage1);

	AnimationClip damage2;
	damage2.frames = { 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34 };
	damage2.frameDurations = {
		1.0f / 60.0f,
		5.0f / 60.0f,
		1.0f / 60.0f,
		1.0f / 60.0f,
		3.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		10.0f / 60.0f,
		2.0f / 60.0f,
		2.0f / 60.0f,
		1.0f / 60.0f,
		2.0f / 60.0f,
		4.0f / 60.0f
	};
	damage2.loop = false;
	m_anim->AddClip("damage2", damage2);
}

// Start gun attack.
void GunnerEnemyEntity::StartGunAttack(const Vector2d& playerPos)
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	if (playerPos.x > m_transform->GetPosition().x)
	{
		m_attackDir = 1;
		m_dir = true;
	}
	else
	{
		m_attackDir = -1;
		m_dir = false;
	}
	SetFacing();

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;
	PlayMotion("attack", true);

	for (int shot = 0; shot < GUNNER_SHOT_COUNT; shot++)
	{
		m_attackOnce[shot] = false;
	}

	m_actionLock = true;
}

void GunnerEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
	if (damage <= 0 || m_hp == nullptr)
	{
		return;
	}

	m_hp->Damage(damage);
	StartDamageMotion(knockback, damage, m_hp->GetHP() <= 0);
}

void GunnerEnemyEntity::Update(float deltaTime)
{
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	if (m_damageType != GUNNER_DAMAGE_NONE)
	{
		UpdateDamageMotion(deltaTime);
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d playerPos = Vector2d::Zero();

	if (!TryGetPlayerInfo(playerPos))
	{
		PlayMotion("idle");
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d myPos = m_transform->GetPosition();

	float distanceX = playerPos.x - myPos.x;
	float distance = distanceX;

	if (distance < 0.0f)
	{
		distance *= -1.0f;
	}

	// Cooldown.
	if (m_cooldownTimer > 0.0f)
	{
		m_cooldownTimer -= deltaTime;

		if (m_cooldownTimer < 0.0f)
		{
			m_cooldownTimer = 0.0f;
		}
	}

	// Wait until player is found.
	if (m_findPlayer == false)
	{
		if (distance < GUNNER_FIND_RANGE)
		{
			m_findPlayer = true;
		}
		else
		{
			m_gunnerState = 0;
			PlayMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			EnemyEntity::Update(deltaTime);
			return;
		}
	}

	if (m_attackType == 0)
	{
		if (m_actionLock == false)
		{
			if (distanceX >= 0.0f)
			{
				if (!m_dir)
				{
					m_dir = true;
					StartTurnMotion();
					EnemyEntity::Update(deltaTime);
					return;
				}
			}
			else
			{
				if (m_dir)
				{
					m_dir = false;
					StartTurnMotion();
					EnemyEntity::Update(deltaTime);
					return;
				}
			}

			/*----------------

			state list
			||status||
			0 wait
			1 approach shot
			2 back shot
			3 stop shot
			4 dead

			attack list
			||attackType||
			0 move
			1 gun attack

			---------------*/

			if (distance < GUNNER_STOP_RANGE)
			{
				m_gunnerState = 3;
			}
			else if (distance < GUNNER_ATTACK_RANGE)
			{
				m_gunnerState = 1;
			}
			else
			{
				m_gunnerState = 1;
			}

			m_actionTimer = 0.0f;
			m_actionLock = true;
		}

		if (m_turning)
		{
			UpdateTurnMotion(deltaTime);
			EnemyEntity::Update(deltaTime);
			return;
		}

		m_actionTimer += deltaTime;

		float dir = GetDirSign();

		switch (m_gunnerState)
		{
		case 0:
			SetFacing();
			PlayMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			SetFacing();
			PlayMotion("walk");
			m_velocity->SetVelocity(Vector2d(GUNNER_APPROACH_SPEED * dir, 0.0f));

			if (distance < GUNNER_STOP_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartGunAttack(playerPos);
			}
			else if (m_actionTimer >= GUNNER_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 2:
			SetFacing();
			PlayMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (distance >= GUNNER_BACK_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartGunAttack(playerPos);
			}
			else if (m_actionTimer >= GUNNER_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 3:
			SetFacing();
			PlayMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (distance < GUNNER_BACK_RANGE || distance >= GUNNER_STOP_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				StartGunAttack(playerPos);
			}
			else if (m_actionTimer >= GUNNER_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 4:
			OnDead();
			break;
		}
	}
	else if (m_attackType == 1)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		m_attackActive = false;
		PlayMotion("attack");
		if (attackTime >= GUNNER_RELOAD_START_TIME)
		{
			SetFacing();
		}
		else
		{
			SetFacing();
		}

		for (int shot = 0; shot < GUNNER_SHOT_COUNT; shot++)
		{
			if (attackTime >= GUNNER_SHOT_TIME[shot] &&
				m_attackOnce[shot] == false)
			{
				Vector2d bulletPos(
					myPos.x + 48.0f * static_cast<float>(m_attackDir),
					myPos.y - 16.0f
				);
				Vector2d bulletVel(
					GUNNER_BULLET_SPEED * static_cast<float>(m_attackDir),
					0.0f
				);

				m_scene->SpawnActor(
					new GunnerBullet(
						m_scene,
						bulletPos,
						bulletVel,
						GUNNER_BULLET_DELETE_RANGE,
						"assets/images/enemy/bullet/gunner.png",
						GUNNER_BULLET_DAMAGE
					)
				);

				m_bulletCount++;
				m_attackOnce[shot] = true;
				m_attackActive = true;
			}
		}

		if (attackTime >= GUNNER_ATTACK_END_TIME)
		{
			SetFacing();
			m_attackType = 0;

			m_cooldownTimer = GUNNER_COOLDOWN;

			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackActive = false;

			for (int shot = 0; shot < GUNNER_SHOT_COUNT; shot++)
			{
				m_attackOnce[shot] = false;
			}
		}
	}

	EnemyEntity::Update(deltaTime);
}

std::string GunnerEnemyEntity::GetTexturePath() const
{
	return "";
}
