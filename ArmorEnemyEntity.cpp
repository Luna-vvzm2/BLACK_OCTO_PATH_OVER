#include "ArmorEnemyEntity.h"
#include "PlayScene.h"
#include "Actor.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "PlayerEntity.h"
#include "Vector2d.h"
#include <cmath>

ArmorEnemyEntity::ArmorEnemyEntity(Scene* scene, const Vector2d& pos)
	: EnemyEntity(scene, pos, Vector2d(96, 190))
	, m_attackOnce(false)
	, m_armorState(0)
	, m_guardRecoverTimer(0.0f)
	, m_ignoreHPChange(false)
	, m_guardRecovering(false)
	, m_guardRecoverMotionTimer(0.0f)
	, m_knockbackTimer(0.0f)
	, m_deadMotion(false)
	, m_deadMotionTimer(0.0f)
	, m_turning(false)
	, m_turnTimer(0.0f)
	, m_currentMotionName("")
{
}

static const char* ARMOR_TEXTURE_SHEET = "assets/images/enemy/armor/armer_sheet.png";
static const int ARMOR_SHEET_X_NUM = 3;
static const int ARMOR_SHEET_Y_NUM = 6;

bool ArmorEnemyEntity::Init()
{
	m_hpMax = 400;
	if (!EnemyEntity::Init()) return false;

	m_guardMax = 350;
	m_guard = m_guardMax;
	m_hp->OnHPChanged = [this](int newHP, int oldHP) {
		OnHPChanged(newHP, oldHP);
		};

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);
	if (m_sprite != nullptr)
	{
		m_sprite->LoadTextureDiv(ARMOR_TEXTURE_SHEET, ARMOR_SHEET_X_NUM, ARMOR_SHEET_Y_NUM);
		m_sprite->SetDrawSize(96.0f, 190.0f);
	}
	DefineAnimationClips();
	PlayMotion("idle", true);

	return true;
}

static const float ARMOR_FIND_RANGE = 800.0f;          // search range
static const float ARMOR_MOVE_SPEED = 120.0f;          // move speed
static const float ARMOR_COOLDOWN = 1.2f;              // attack cooldown
static const float ARMOR_ATTACK_RANGE = 180.0f;        // explosion x range
static const float ARMOR_ATTACK_HEIGHT_RANGE = 130.0f; // explosion y range
static const float ARMOR_ATTACK_KNOCKBACK_X = 420.0f;  // explosion knockback x
static const float ARMOR_ATTACK_KNOCKBACK_Y = -260.0f; // explosion knockback y
static const int ARMOR_ATTACK_DAMAGE = 15;             // explosion damage
static const float ARMOR_GUARD_RECOVER_TIME = 5.0f;    // guard recover seconds
static const float ARMOR_GUARD_RECOVER_MOTION_TIME = 10.0f / 60.0f;
static const float ARMOR_DAMAGE_KNOCKBACK_X = 420.0f;
static const float ARMOR_DAMAGE_KNOCKBACK_Y = -260.0f;
static const float ARMOR_DAMAGE_KNOCKBACK_TIME = 0.25f;

static const float ARMOR_PRE_TIME = 23.0f / 60.0f;     // frame 1
static const float ARMOR_ATTACK_TIME = 10.0f / 60.0f;  // frame 2
static const float ARMOR_AFTER_TIME = 2.0f / 60.0f;    // frame 3
static const float ARMOR_RECHECK_TIME = 0.2f;          // state recheck time
static const float ARMOR_DAMAGE_MOTION_TIME = 35.0f / 60.0f;
static const float ARMOR_TURN_MOTION_TIME = 5.0f / 60.0f;

float ArmorEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

void ArmorEnemyEntity::SetFacing()
{
	if (m_sprite == nullptr)
	{
		return;
	}

	m_sprite->SetFlipX(!m_dir);
}

bool ArmorEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos, PlayerEntity*& player) const
{
	player = nullptr;

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
		player = static_cast<PlayerEntity*>(actor);
		return true;
	}

	return false;
}

void ArmorEnemyEntity::UpdateGuardRecover(float deltaTime)
{
	if (m_guard > 0)
	{
		m_guardRecoverTimer = 0.0f;
		m_guardRecoverMotionTimer = 0.0f;
		m_guardRecovering = false;
		return;
	}

	if (m_knockbackTimer > 0.0f)
	{
		m_knockbackTimer -= deltaTime;
		if (m_knockbackTimer < 0.0f)
		{
			m_knockbackTimer = 0.0f;
		}
	}

	m_guardRecoverTimer += deltaTime;

	if (m_guardRecovering == false)
	{
		if (m_guardRecoverTimer >= ARMOR_GUARD_RECOVER_TIME)
		{
			m_guardRecovering = true;
			m_guardRecoverMotionTimer = 0.0f;
			m_attackType = 0;
			m_attackActive = false;
			m_attackOnce = false;
			m_actionLock = true;
		}
		return;
	}

	m_guardRecoverMotionTimer += deltaTime;
	PlayMotion("idle");

	if (m_guardRecoverMotionTimer >= ARMOR_GUARD_RECOVER_MOTION_TIME)
	{
		RecoverGuard();
	}
}

void ArmorEnemyEntity::PlayMotion(const std::string& motionName, bool reset)
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

void ArmorEnemyEntity::DefineAnimationClips()
{
	if (m_anim == nullptr)
	{
		return;
	}

	AnimationClip idle;
	idle.frames = { 9 };
	idle.frameDurations = { 10.0f / 60.0f };
	idle.loop = true;
	m_anim->AddClip("idle", idle);

	AnimationClip attack;
	attack.frames = { 0, 1, 2 };
	attack.frameDurations = {
		23.0f / 60.0f,
		10.0f / 60.0f,
		2.0f / 60.0f
	};
	attack.loop = false;
	m_anim->AddClip("attack", attack);

	AnimationClip damage;
	damage.frames = { 9, 3, 4, 5, 6 };
	damage.frameDurations = {
		1.0f / 60.0f,
		25.0f / 60.0f,
		5.0f / 60.0f,
		1.0f / 60.0f,
		3.0f / 60.0f
	};
	damage.loop = false;
	m_anim->AddClip("damage", damage);

	AnimationClip turn;
	turn.frames = { 9, 7, 8 };
	turn.frameDurations = {
		1.0f / 60.0f,
		3.0f / 60.0f,
		1.0f / 60.0f
	};
	turn.loop = false;
	m_anim->AddClip("turn", turn);

	AnimationClip run;
	run.frames = { 9, 10, 11, 12, 13, 14, 15, 16, 17 };
	run.frameDurations = {
		1.0f / 60.0f,
		3.0f / 60.0f,
		3.0f / 60.0f,
		1.0f / 60.0f,
		3.0f / 60.0f,
		3.0f / 60.0f,
		1.0f / 60.0f,
		3.0f / 60.0f,
		1.0f / 60.0f
	};
	run.loop = true;
	m_anim->AddClip("run", run);
}

// Start armor punch attack.
void ArmorEnemyEntity::StartArmorsAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;
	m_attackOnce = false;
	SetFacing();
	PlayMotion("attack", true);
	m_actionLock = true;
}

void ArmorEnemyEntity::TriggerExplosion(const Vector2d& playerPos, PlayerEntity* player)
{
	if (player == nullptr)
	{
		return;
	}

	Vector2d myPos = m_transform->GetPosition();
	float dx = playerPos.x - myPos.x;
	float dy = playerPos.y - myPos.y;

	if (std::fabs(dx) > ARMOR_ATTACK_RANGE || std::fabs(dy) > ARMOR_ATTACK_HEIGHT_RANGE)
	{
		return;
	}

	float knockbackX = dx < 0.0f ? -ARMOR_ATTACK_KNOCKBACK_X : ARMOR_ATTACK_KNOCKBACK_X;
	player->TakeDamage(
		ARMOR_ATTACK_DAMAGE,
		Vector2d(knockbackX, ARMOR_ATTACK_KNOCKBACK_Y)
	);
}

void ArmorEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
	if (damage <= 0 || m_hp == nullptr)
	{
		return;
	}

	m_guardRecoverTimer = 0.0f;
	m_guardRecoverMotionTimer = 0.0f;
	m_guardRecovering = false;

	if (m_guard > 0)
	{
		m_guard -= damage;

		if (m_guard <= 0)
		{
			BreakGuard(knockback);
		}
		return;
	}

	m_ignoreHPChange = true;
	m_hp->Damage(damage);
	m_ignoreHPChange = false;

	if (m_hp->GetHP() <= 0)
	{

		OnDead();
		StartDeadMotion();

		return;
	}

	StartDamageMotion(knockback);
}

void ArmorEnemyEntity::OnHPChanged(int newHP, int oldHP)
{
	if (m_ignoreHPChange || newHP >= oldHP)
	{
		return;
	}

	int damage = oldHP - newHP;
	m_guardRecoverTimer = 0.0f;
	m_guardRecoverMotionTimer = 0.0f;
	m_guardRecovering = false;

	if (m_guard > 0)
	{
		m_ignoreHPChange = true;
		m_hp->Heal(damage);
		m_ignoreHPChange = false;

		m_guard -= damage;
		if (m_guard <= 0)
		{
			BreakGuard(Vector2d(ARMOR_DAMAGE_KNOCKBACK_X, ARMOR_DAMAGE_KNOCKBACK_Y));
		}
		return;
	}

	if (newHP <= 0)
	{
		StartDeadMotion();
		return;
	}

	StartDamageMotion(Vector2d(ARMOR_DAMAGE_KNOCKBACK_X, ARMOR_DAMAGE_KNOCKBACK_Y));
}

void ArmorEnemyEntity::StartDamageMotion(const Vector2d& knockback)
{
	Vector2d damageKnockback = knockback;
	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;
	if (TryGetPlayerInfo(playerPos, player))
	{
		Vector2d myPos = GetPos();
		float awaySign = myPos.x < playerPos.x ? -1.0f : 1.0f;
		damageKnockback.x = std::fabs(damageKnockback.x) * awaySign;
	}

	m_velocity->SetVelocity(damageKnockback);
	m_knockbackTimer = ARMOR_DAMAGE_KNOCKBACK_TIME;
	PlayMotion("damage", true);

	if (m_guard <= 0)
	{
		m_attackType = 0;
		m_attackActive = false;
		m_attackOnce = false;
		m_actionLock = false;
	}
}

void ArmorEnemyEntity::StartDeadMotion()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
	m_attackType = 0;
	m_attackActive = false;
	m_attackOnce = false;
	m_actionLock = true;
	m_guardRecovering = false;
	m_turning = false;
	m_deadMotion = true;
	m_deadMotionTimer = 0.0f;
	PlayMotion("damage", true);
}

void ArmorEnemyEntity::StartTurnMotion()
{
	m_turning = true;
	m_turnTimer = 0.0f;
	m_actionLock = true;
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
	SetFacing();
	PlayMotion("turn", true);
}

void ArmorEnemyEntity::UpdateDeadMotion(float deltaTime)
{
	if (!m_deadMotion)
	{
		return;
	}

	m_deadMotionTimer += deltaTime;
	float ratio = ARMOR_DAMAGE_MOTION_TIME > 0.0f ? m_deadMotionTimer / ARMOR_DAMAGE_MOTION_TIME : 1.0f;
	if (ratio > 1.0f)
	{
		ratio = 1.0f;
	}

	if (m_deadMotionTimer >= ARMOR_DAMAGE_MOTION_TIME)
	{
		OnDead();
	}
}

void ArmorEnemyEntity::UpdateTurnMotion(float deltaTime)
{
	if (!m_turning)
	{
		return;
	}

	m_turnTimer += deltaTime;
	if (m_turnTimer >= ARMOR_TURN_MOTION_TIME)
	{
		m_turning = false;
		m_turnTimer = 0.0f;
		m_actionLock = false;
	}
}

void ArmorEnemyEntity::BreakGuard(const Vector2d& knockback)
{
	m_guard = 0;
	m_damageCancel = true;
	m_guardRecovering = false;
	m_guardRecoverTimer = 0.0f;
	m_guardRecoverMotionTimer = 0.0f;
	StartDamageMotion(knockback);
}

void ArmorEnemyEntity::RecoverGuard()
{
	m_guard = m_guardMax;
	m_damageCancel = false;
	m_guardRecovering = false;
	m_guardRecoverTimer = 0.0f;
	m_guardRecoverMotionTimer = 0.0f;
	m_knockbackTimer = 0.0f;
	m_actionLock = false;
}

void ArmorEnemyEntity::Update(float deltaTime)
{
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	if (m_deadMotion)
	{
		UpdateDeadMotion(deltaTime);
		EnemyEntity::Update(deltaTime);
		return;
	}

	UpdateGuardRecover(deltaTime);

	if (m_guardRecovering)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	if (m_knockbackTimer > 0.0f)
	{
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;

	if (!TryGetPlayerInfo(playerPos, player))
	{
		PlayMotion("idle");
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d myPos = m_transform->GetPosition();

	float distanceX = playerPos.x - myPos.x;
	float distanceY = playerPos.y - myPos.y;
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
		if (distance < ARMOR_FIND_RANGE)
		{
			m_findPlayer = true;
		}
		else
		{
			m_armorState = 0;
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
			1 approach
			2 attack
			4 dead

			attack list
			||attackType||
			0 move / non attack
			1 punch attack

			---------------*/

			if (distance < ARMOR_ATTACK_RANGE)
			{
				m_armorState = 2;
			}
			else if (distance < ARMOR_FIND_RANGE)
			{
				m_armorState = 1;
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

		switch (m_armorState)
		{
		case 0:
			PlayMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			PlayMotion("run");
			m_velocity->SetVelocity(Vector2d(ARMOR_MOVE_SPEED * dir, 0.0f));

			if (distance < ARMOR_ATTACK_RANGE)
			{
				m_actionLock = false;
			}
			break;

		case 2:
			PlayMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (m_cooldownTimer <= 0.0f)
			{
				StartArmorsAttack();
			}
			else if (m_actionTimer >= ARMOR_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 4:
			StartDeadMotion();
			break;
		}
	}
	else if (m_attackType == 1)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		const float ATTACK_START = ARMOR_PRE_TIME;
		const float ATTACK_END = ARMOR_PRE_TIME + ARMOR_ATTACK_TIME;
		const float END_TIME = ARMOR_PRE_TIME + ARMOR_ATTACK_TIME + ARMOR_AFTER_TIME;

		if (attackTime < ATTACK_START)
		{
			PlayMotion("attack");
			m_attackActive = false;
		}
		else if (attackTime < ATTACK_END)
		{
			PlayMotion("attack");
			m_attackActive = true;

			if (m_attackOnce == false)
			{
				TriggerExplosion(playerPos, player);
				m_attackOnce = true;
			}
		}
		else if (attackTime < END_TIME)
		{
			PlayMotion("attack");
			m_attackActive = false;
		}
		else
		{
			m_attackType = 0;
			m_attackActive = false;

			m_cooldownTimer = ARMOR_COOLDOWN;
			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
		}
	}

	EnemyEntity::Update(deltaTime);
}

std::string ArmorEnemyEntity::GetTexturePath() const
{
	return "";
}
