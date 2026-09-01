#include "WhiteEnemyEntity.h"
#include "PlayScene.h"
#include "Actor.h"
#include "PlayerEntity.h"
#include "CollisionComponent.h"
#include "GravityComponent.h"
#include "VelocityComponent.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "Renderer.h"
#include "Vector2d.h"
#include <DxLib.h>
#include <cmath>

class WhiteShurikenBullet : public EnemyEntity
{
public:
	WhiteShurikenBullet(
		Scene* scene,
		const Vector2d& pos,
		const Vector2d& velocity,
		float deleteRange,
		const std::string& texturePath,
		int damage,
		const Vector2d& drawSize,
		float rotateInterval,
		float rotateStep
	)
		: EnemyEntity(scene, pos, Vector2d(12.0f, 12.0f))
		, m_startPos(pos)
		, m_bulletVelocity(velocity)
		, m_deleteRange(deleteRange)
		, m_texturePath(texturePath)
		, m_damage(damage)
		, m_drawSize(drawSize)
		, m_rotateInterval(rotateInterval)
		, m_rotateStep(rotateStep)
		, m_rotationAngle(0.0f)
		, m_rotationTimer(0.0f)
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

		m_rotationTimer += deltaTime;
		while (m_rotateInterval > 0.0f && m_rotationTimer >= m_rotateInterval)
		{
			m_rotationTimer -= m_rotateInterval;
			m_rotationAngle += m_rotateStep;
		}

		Vector2d pos = m_transform->GetPosition();
		pos += m_velocity->GetVelocity() * deltaTime;
		m_transform->SetPosition(pos);

		if ((pos - m_startPos).length() > m_deleteRange || TryDamagePlayer())
		{
			SetState(Actor::State::Dead);
		}
	}

	void Draw() override
	{
		if (IsDead() || m_scene == nullptr || m_sprite == nullptr || m_transform == nullptr)
		{
			return;
		}

		Game* game = m_scene->GetGame();
		Renderer* renderer = game != nullptr ? game->GetRenderer() : nullptr;
		int handle = m_sprite->GetHandle();
		if (renderer == nullptr || handle < 0)
		{
			return;
		}

		int textureWidth = 0;
		int textureHeight = 0;
		GetGraphSize(handle, &textureWidth, &textureHeight);
		float scaleX = textureWidth > 0 ? m_drawSize.x / static_cast<float>(textureWidth) : 1.0f;
		float scaleY = textureHeight > 0 ? m_drawSize.y / static_cast<float>(textureHeight) : 1.0f;

		renderer->DrawSpriteEx(
			m_transform->GetPosition(),
			scaleX,
			scaleY,
			m_rotationAngle,
			handle,
			true,
			Vector2d(static_cast<float>(textureWidth), static_cast<float>(textureHeight)) * 0.5f
		);

#ifdef _DEBUG
		if (m_collision != nullptr)
		{
			m_collision->DrawDebug();
		}
#endif
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
			float knockbackX = m_bulletVelocity.x < 0.0f ? -300.0f : 300.0f;
			player->TakeDamage(m_damage, Vector2d(knockbackX, 0.0f));
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
	Vector2d m_drawSize;
	float m_rotateInterval;
	float m_rotateStep;
	float m_rotationAngle;
	float m_rotationTimer;
};

WhiteEnemyEntity::WhiteEnemyEntity(Scene* scene, const Vector2d& pos)
	: EnemyEntity(scene, pos, Vector2d(96, 150))
	, m_bulletCount(0)
	, m_attackOnce(false)
	, m_whiteState(0)
	, m_currentTexturePath("")
	, m_currentMotionName("")
	, m_chasePlayer(true)
	, m_hasLastMove(false)
	, m_lastWantedMoveX(0.0f)
	, m_wallStopTimer(0.0f)
	, m_lastMoveStartPos(pos)
	, m_damageState(0)
	, m_damageFrameTimer(0.0f)
	, m_damageHoldGroundFrame(false)
{
}

bool WhiteEnemyEntity::Init()
{
	if (!EnemyEntity::Init()) return false;

	m_anim = AddComponent<AnimationComponent>();
	m_anim->SetSprite(m_sprite);
	DefineAnimationClips();

	return true;
}


// çıìGîÕàÕÇ∆ãóó£îªíË
static const float WHITE_TILE_SIZE = 104.0f;
static const float WHITE_ENEMY_HALF_WIDTH = 48.0f;
static const float WHITE_PLAYER_HALF_WIDTH = 42.5f;
static const float WHITE_SWORD_GAP_RANGE = WHITE_TILE_SIZE * 0.7f;
static const float WHITE_FIND_RANGE = 800.0f;
static const float WHITE_NEAR_SWORD_RANGE = WHITE_ENEMY_HALF_WIDTH + WHITE_PLAYER_HALF_WIDTH + WHITE_SWORD_GAP_RANGE;
//åïçUåÇÇ∆éËó†åïçUåÇÇÃã´ñ⁄
static const float WHITE_BACK_RANGE = 250.0f;
static const float WHITE_STOP_SHURIKEN_RANGE = 612.0f;
static const float WHITE_SHURIKEN_RANGE = 650.0f;
static const float WHITE_BULLET_DELETE_RANGE = 1500.0f;
static const float WHITE_SAME_FLOOR_Y_RANGE = WHITE_TILE_SIZE * 0.5f;
static const float WHITE_STUCK_MOVE_EPS = 1.0f;
static const float WHITE_STUCK_IDLE_TIME = 0.25f;

// à⁄ìÆä÷åW
static const float WHITE_APPROACH_SPEED = 120.0f;
static const float WHITE_BACK_SPEED = 150.0f;
static const float WHITE_FAR_APPROACH_SPEED = 250.0f;
static const float WHITE_BULLET_SPEED = 250.0f;

// éûä‘ä÷åW
static const float WHITE_RECHECK_TIME = 0.3f;

// éËó†åïçUåÇ
static const float WHITE_SHURIKEN_COOLDOWN = 0.675f;
static const int WHITE_SHURIKEN_DAMAGE = 15;
static const float WHITE_SHURIKEN_SHOT_TIME = 40.0f / 60.0f;
static const float WHITE_SHURIKEN_END_TIME = 60.0f / 60.0f;
static const Vector2d WHITE_SHURIKEN_BULLET_DRAW_SIZE = Vector2d(48.0f, 48.0f);
static const float WHITE_SHURIKEN_BULLET_ROTATE_INTERVAL = 4.0f / 60.0f;
static const float WHITE_SHURIKEN_BULLET_ROTATE_STEP = 15.0f * 3.14159265f / 180.0f;

// åïçUåÇ
//åïçUåÇÅ®ë“ã@Å®åïçUåÇÇÃÉAÉCÉhÉãéûä‘
static const float WHITE_SWORD_COOLDOWN = 0.225f;
//åïçUåÇîÕàÕ
static const float WHITE_SWORD_ATTACK_RANGE = WHITE_BACK_RANGE - 20.0f;
static const float WHITE_SWORD_HEIGHT_RANGE = 80.0f;
static const int WHITE_SWORD_DAMAGE = 10;
static const float WHITE_SWORD_KNOCKBACK_X = 300.0f;
static const float WHITE_SWORD_KNOCKBACK_Y = 0.0f;
static const float WHITE_SWORD_HIT_TIME = 50.0f / 60.0f;
static const float WHITE_SWORD_ACTIVE_END_TIME = 72.5f / 60.0f;
static const float WHITE_SWORD_END_TIME = 87.5f / 60.0f;

// âÊëúéQè∆
static const char* WHITE_TEXTURE_IDLE = "assets/images/enemy/white/idle.png";
static const char* WHITE_TEXTURE_WALK = "assets/images/enemy/white/walk.png";
static const char* WHITE_TEXTURE_SHURIKEN = "assets/images/enemy/white/shuriken.png";
static const char* WHITE_TEXTURE_SWORD_PRE = "assets/images/enemy/white/sword_pre.png";
static const char* WHITE_TEXTURE_SWORD = "assets/images/enemy/white/sword.png";
static const char* WHITE_TEXTURE_SWORD_AFTER = "assets/images/enemy/white/sword_after.png";
static const char* WHITE_TEXTURE_SHEET = "assets/images/enemy/white/White.png";
static const char* WHITE_TEXTURE_HIT_WEAK = "assets/images/enemy/white/White2.png";
static const char* WHITE_TEXTURE_SHURIKEN_BULLET = "assets/images/enemy/white/Shuriken.png";
static const int WHITE_SHEET_X_NUM = 4;
static const int WHITE_SHEET_Y_NUM = 10;
static const int WHITE_HIT_WEAK_X_NUM = 4;
static const int WHITE_HIT_WEAK_Y_NUM = 1;

// É_ÉÅÅ[ÉWä÷åW
static const int WHITE_BLOW_MIN_DAMAGE = 10;
static const int WHITE_BLOW_LARGE_MIN_DAMAGE = 25;
static const int WHITE_DAMAGE_NONE = 0;
static const int WHITE_DAMAGE_WEAK = 1;
static const int WHITE_DAMAGE_BLOW = 2;
static const int WHITE_DAMAGE_BLOW_LARGE = 3;
static const int WHITE_DAMAGE_DEAD = 4;
static const int WHITE_LARGE_BLOW_GROUND_FRAME = 34;
static const float WHITE_WEAK_HIT_TOTAL_TIME = 25.0f / 60.0f;
static const float WHITE_BLOW_TOTAL_TIME = 55.0f / 60.0f;
static const float WHITE_BLOW_LARGE_HOLD_TIME = 52.5f / 60.0f;
static const float WHITE_BLOW_LARGE_TOTAL_TIME = 67.5f / 60.0f;
static const float WHITE_DEAD_SHOW_TIME = 15.0f / 60.0f;

void WhiteEnemyEntity::DefineAnimationClips()
{
	if (m_anim == nullptr)
	{
		return;
	}

	AnimationClip idle;
	idle.frames = { 0, 1, 2, 3 };
	idle.frameDurations = {
		7.5f / 60.0f,
		7.5f / 60.0f,
		7.5f / 60.0f,
		7.5f / 60.0f
	};
	idle.loop = true;
	m_anim->AddClip("idle", idle);

	AnimationClip walk;
	walk.frames = { 4, 5, 6, 7, 8, 9, 10, 11 };
	walk.frameDurations = {
		7.5f / 60.0f,
		7.5f / 60.0f,
		7.5f / 60.0f,
		11.25f / 60.0f,
		7.5f / 60.0f,
		11.25f / 60.0f,
		3.75f / 60.0f,
		11.25f / 60.0f
	};
	walk.loop = true;
	m_anim->AddClip("walk", walk);

	AnimationClip backWalk;
	backWalk.frames = { 4, 5, 6, 7, 8, 9, 10, 11 };
	backWalk.frameDurations = {
		9.375f / 60.0f,
		9.375f / 60.0f,
		9.375f / 60.0f,
		13.125f / 60.0f,
		9.375f / 60.0f,
		13.125f / 60.0f,
		5.625f / 60.0f,
		13.125f / 60.0f
	};
	backWalk.loop = true;
	m_anim->AddClip("back_walk", backWalk);

	AnimationClip shuriken;
	shuriken.frames = { 12, 13, 14, 12, 15, 16, 17, 18 };
	shuriken.frameDurations = {
		10.0f / 60.0f,
		15.0f / 60.0f,
		12.5f / 60.0f,
		2.5f / 60.0f,
		2.5f / 60.0f,
		7.5f / 60.0f,
		2.5f / 60.0f,
		7.5f / 60.0f
	};
	shuriken.loop = false;
	m_anim->AddClip("shuriken", shuriken);

	AnimationClip sword;
	sword.frames = { 12, 13, 14, 19, 20, 21, 22, 23, 24 };
	sword.frameDurations = {
		10.0f / 60.0f,
		15.0f / 60.0f,
		12.5f / 60.0f,
		7.5f / 60.0f,
		5.0f / 60.0f,
		2.5f / 60.0f,
		7.5f / 60.0f,
		12.5f / 60.0f,
		7.5f / 60.0f
	};
	sword.loop = false;
	m_anim->AddClip("sword", sword);

	AnimationClip hitWeak;
	hitWeak.frames = { 0, 1, 2, 3 };
	hitWeak.frameDurations = {
		12.5f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		2.5f / 60.0f
	};
	hitWeak.loop = false;
	m_anim->AddClip("hit_weak", hitWeak);

	AnimationClip hitBlow;
	hitBlow.frames = { 25, 26, 30, 31, 32, 35, 36, 37, 38, 39 };
	hitBlow.frameDurations = {
		2.5f / 60.0f,
		7.5f / 60.0f,
		10.0f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		2.5f / 60.0f,
		7.5f / 60.0f,
		5.0f / 60.0f
	};
	hitBlow.loop = false;
	m_anim->AddClip("hit_blow", hitBlow);

	AnimationClip hitBlowLarge;
	hitBlowLarge.frames = {
		25, 26, 27, 28, 29,
		30, 31, 32, 33, 34,
		35, 36, 37, 38, 39
	};
	hitBlowLarge.frameDurations = {
		2.5f / 60.0f,
		15.0f / 60.0f,
		2.5f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		7.5f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		7.5f / 60.0f,
		5.0f / 60.0f,
		5.0f / 60.0f,
		2.5f / 60.0f,
		7.5f / 60.0f,
		5.0f / 60.0f
	};
	hitBlowLarge.loop = false;
	m_anim->AddClip("hit_blow_large", hitBlowLarge);

	AnimationClip hitDead;
	hitDead.frames = { 37 };
	hitDead.frameDurations = { 15.0f / 60.0f };
	hitDead.loop = false;
	m_anim->AddClip("hit_dead", hitDead);
}

float WhiteEnemyEntity::GetDirSign() const
{
	return m_dir ? 1.0f : -1.0f;
}

void WhiteEnemyEntity::SetFacing(bool flipH)
{
	if (m_transform == nullptr)
	{
		return;
	}

	Vector2d scale = m_transform->GetScale();
	float scaleX = std::fabs(scale.x);
	scale.x = flipH ? -scaleX : scaleX;
	m_transform->SetScale(scale);
}

void WhiteEnemyEntity::PrepareMoveTracking(float wantedMoveX)
{
	m_lastMoveStartPos = m_transform != nullptr ? m_transform->GetPosition() : Vector2d::Zero();
	m_lastWantedMoveX = wantedMoveX;
	m_hasLastMove = true;
}

bool WhiteEnemyEntity::TryGetPlayerInfo(Vector2d& playerPos, PlayerEntity*& player, bool& playerOnGround) const
{
	player = nullptr;
	playerOnGround = false;

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
		playerOnGround = player != nullptr && player->OnGround();
		return true;
	}

	return false;
}

void WhiteEnemyEntity::PlaySheetMotion(const std::string& motionName)
{
	if (m_currentMotionName == motionName)
	{
		return;
	}

	if (m_sprite == nullptr || m_anim == nullptr)
	{
		return;
	}

	if (m_currentTexturePath != WHITE_TEXTURE_SHEET)
	{
		if (!m_sprite->LoadTextureDiv(WHITE_TEXTURE_SHEET, WHITE_SHEET_X_NUM, WHITE_SHEET_Y_NUM))
		{
			return;
		}

		m_currentTexturePath = WHITE_TEXTURE_SHEET;
	}

	m_anim->Play(motionName, true);
	m_currentMotionName = motionName;
}


void WhiteEnemyEntity::StartShurikenAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 1;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	PlaySheetMotion("shuriken");

	m_attackOnce = false;
	m_actionLock = true;
}


void WhiteEnemyEntity::StartSwordAttack()
{
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	m_attackType = 2;
	m_attackTimer = 0.0f;

	m_attackActive = false;

	m_currentMotionName = "";
	PlaySheetMotion("sword");

	m_attackOnce = false;
	m_actionLock = true;
}

void WhiteEnemyEntity::CancelAttackForDamage()
{
	m_attackType = 0;
	m_attackActive = false;
	m_attackOnce = false;
	m_actionTimer = 0.0f;
	m_cooldownTimer = 0.0f;
	m_actionLock = true;
	m_hasLastMove = false;
}

void WhiteEnemyEntity::PlayDamageMotion(
	const std::string& motionName,
	const std::string& texturePath,
	int xNum,
	int yNum
)
{
	if (m_sprite == nullptr || m_anim == nullptr)
	{
		return;
	}

	if (m_currentTexturePath != texturePath)
	{
		if (!m_sprite->LoadTextureDiv(texturePath, xNum, yNum))
		{
			return;
		}
		m_currentTexturePath = texturePath;
	}

	m_anim->Play(motionName, true);
	m_currentMotionName = motionName;
}

void WhiteEnemyEntity::StartWeakHit()
{
	CancelAttackForDamage();

	PlayDamageMotion(
		"hit_weak",
		WHITE_TEXTURE_HIT_WEAK,
		WHITE_HIT_WEAK_X_NUM,
		WHITE_HIT_WEAK_Y_NUM
	);
	m_damageState = WHITE_DAMAGE_WEAK;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartBlowHit(const Vector2d& knockback)
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(knockback);

	PlayDamageMotion(
		"hit_blow",
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM
	);
	m_damageState = WHITE_DAMAGE_BLOW;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartLargeBlowHit(const Vector2d& knockback)
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(knockback);

	PlayDamageMotion(
		"hit_blow_large",
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM
	);
	m_damageState = WHITE_DAMAGE_BLOW_LARGE;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::StartDeadHit()
{
	CancelAttackForDamage();
	m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

	PlayDamageMotion(
		"hit_dead",
		WHITE_TEXTURE_SHEET,
		WHITE_SHEET_X_NUM,
		WHITE_SHEET_Y_NUM
	);
	m_damageState = WHITE_DAMAGE_DEAD;
	m_damageFrameTimer = 0.0f;
	m_damageHoldGroundFrame = false;
}

void WhiteEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
	if (m_hp == nullptr)
	{
		return;
	}

	m_hp->Damage(damage);

	if (m_hp->GetHP() <= 0)
	{
		StartDeadHit();
		return;
	}

	Vector2d damageKnockback = knockback;
	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;
	bool playerOnGround = false;
	if (TryGetPlayerInfo(playerPos, player, playerOnGround))
	{
		Vector2d myPos = GetPos();
		float awaySign = myPos.x < playerPos.x ? -1.0f : 1.0f;
		damageKnockback.x = std::fabs(damageKnockback.x) * awaySign;
	}

	if (damage >= WHITE_BLOW_LARGE_MIN_DAMAGE)
	{
		StartLargeBlowHit(damageKnockback);
	}
	else if (damage >= WHITE_BLOW_MIN_DAMAGE)
	{
		StartBlowHit(damageKnockback);
	}
	else
	{
		StartWeakHit();
	}
}

void WhiteEnemyEntity::UpdateDamageMotion(float deltaTime)
{
	if (m_damageState == WHITE_DAMAGE_NONE)
	{
		return;
	}

	float totalTime = 0.0f;
	float holdStartTime = 0.0f;

	if (m_damageState == WHITE_DAMAGE_WEAK)
	{
		totalTime = WHITE_WEAK_HIT_TOTAL_TIME;
	}
	else if (m_damageState == WHITE_DAMAGE_BLOW)
	{
		totalTime = WHITE_BLOW_TOTAL_TIME;
	}
	else if (m_damageState == WHITE_DAMAGE_BLOW_LARGE)
	{
		holdStartTime = WHITE_BLOW_LARGE_HOLD_TIME;
		totalTime = WHITE_BLOW_LARGE_TOTAL_TIME;
	}
	else if (m_damageState == WHITE_DAMAGE_DEAD)
	{
		m_damageFrameTimer += deltaTime;
		if (m_damageFrameTimer >= WHITE_DEAD_SHOW_TIME)
		{
			SetState(Actor::State::Dead);
		}
		return;
	}

	if (totalTime <= 0.0f)
	{
		m_damageState = WHITE_DAMAGE_NONE;
		return;
	}

	if (m_damageHoldGroundFrame)
	{
		if (m_isGround == false)
		{
			return;
		}

		m_damageHoldGroundFrame = false;
	}

	m_damageFrameTimer += deltaTime;

	if (m_damageState == WHITE_DAMAGE_BLOW_LARGE &&
		m_damageFrameTimer >= holdStartTime &&
		m_isGround == false)
	{
		if (m_sprite != nullptr)
		{
			m_sprite->SetFrame(WHITE_LARGE_BLOW_GROUND_FRAME);
		}
		m_damageFrameTimer = holdStartTime;
		m_damageHoldGroundFrame = true;
		return;
	}

	if (m_damageFrameTimer >= totalTime)
	{
		m_damageState = WHITE_DAMAGE_NONE;
		m_actionLock = false;
		m_currentMotionName = "";
		m_damageHoldGroundFrame = false;
	}
}

void WhiteEnemyEntity::Update(float deltaTime)
{
	if (m_damageState != WHITE_DAMAGE_NONE)
	{
		UpdateDamageMotion(deltaTime);
		if (m_anim != nullptr && m_damageState != WHITE_DAMAGE_NONE && m_damageHoldGroundFrame == false)
		{
			m_anim->Update(deltaTime);
		}
		if (m_damageState == WHITE_DAMAGE_NONE)
		{
			EnemyEntity::Update(deltaTime);
		}
		else if (m_damageState != WHITE_DAMAGE_DEAD)
		{
			if (m_gravity != nullptr)
			{
				m_gravity->Update(deltaTime);
			}
			MoveAndCollide(deltaTime);
		}
		return;
	}

	Vector2d playerPos = Vector2d::Zero();
	PlayerEntity* player = nullptr;
	bool playerOnGround = false;

	if (!TryGetPlayerInfo(playerPos, player, playerOnGround))
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
		EnemyEntity::Update(deltaTime);
		return;
	}

	Vector2d myPos = m_transform->GetPosition();

	float distanceX = playerPos.x - myPos.x;
	float distanceY = playerPos.y - myPos.y;
	float distance = distanceX;

	SetFacing(m_attackType == 0 ? distanceX >= 0.0f : m_dir);


	if (distance < 0.0f)
	{
		distance *= -1.0f;
	}

	if (m_cooldownTimer > 0.0f)
	{
		m_cooldownTimer -= deltaTime;

		if (m_cooldownTimer < 0.0f)
		{
			m_cooldownTimer = 0.0f;
		}
	}

	if (m_findPlayer == false)
	{
		if (distance < WHITE_FIND_RANGE)
		{
			m_findPlayer = true;
		}
		else
		{
			PlaySheetMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			//continue;
			EnemyEntity::Update(deltaTime);
			return;
		}
	}

	/*----------------

		èÛë‘àÍóó
		||whiteState||
		0 ë“ã@ÅEçıìG
		1 ê⁄ãﬂÇµÇ»Ç™ÇÁéËó†åïçUåÇ
		2 å„ëﬁÇµÇ»Ç™ÇÁåïçUåÇÇÃçƒégópÇë“Ç¬
		3 í‚é~ÇµÇƒéËó†åïçUåÇ
		4 í‚é~ÇµÇƒåïçUåÇ
		5 âìãóó£Ç©ÇÁê⁄ãﬂ
		6 éÄñS

		çUåÇÉ^ÉCÉv
		||attackType||
		0 çUåÇÇ»Çµ
		1 éËó†åïçUåÇ
		2 åïçUåÇ

   ---------------*/

	if (m_attackType == 0)
	{
		if (m_actionLock == false)
		{
			if (distanceX >= 0.0f)
			{
				m_dir = true;
			}
			else
			{
				m_dir = false;
			}

			// ãﬂãóó£Ç≈ÇÕåïÇégÇ¢ÅAçƒégópë“ÇøÇÃä‘ÇÕå„ëﬁÇ∑ÇÈÅB
			if (distance < WHITE_BACK_RANGE)
			{
				if (m_cooldownTimer <= 0.0f)
				{
					m_whiteState = 4;
				}
				else
				{
					m_whiteState = 2;
				}
			}

			else if (distance < WHITE_STOP_SHURIKEN_RANGE)
			{
				m_whiteState = 3;
			}

			else if (distance < WHITE_SHURIKEN_RANGE)
			{
				m_whiteState = 1;
			}
			else
			{
				m_whiteState = 5;
			}

			m_actionTimer = 0.0f;
			m_actionLock = true;
		}

		m_actionTimer += deltaTime;

		float dir = GetDirSign();

		if (m_cooldownTimer <= 0.0f && distance < WHITE_BACK_RANGE)
		{
			PlaySheetMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			StartSwordAttack();
			EnemyEntity::Update(deltaTime);
			m_hasLastMove = false;
			return;
		}

		switch (m_whiteState)
		{
		case 0:
			PlaySheetMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
			break;

		case 1:
			// éËó†åïÇÃéÀíˆÇ‹Ç≈ê⁄ãﬂÇ∑ÇÈÅB
			PlaySheetMotion("walk");
			PrepareMoveTracking(WHITE_APPROACH_SPEED * dir);
			m_velocity->SetVelocity(Vector2d(WHITE_APPROACH_SPEED * dir, 0.0f));

			if (distance < WHITE_BACK_RANGE || distance > WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}
			// åïÇÃçƒégópë“ÇøíÜÇÕä‘çáÇ¢ÇéÊÇÈÅB
			if (m_cooldownTimer <= 0.0f)
			{
				if (distance < WHITE_BACK_RANGE)
				{
					StartSwordAttack();
				}
				else
				{
					StartShurikenAttack();
				}
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 2:

			if (m_cooldownTimer <= 0.0f)
			{
				PlaySheetMotion("idle");
				m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
				StartSwordAttack();
			}
			else
			{
				PlaySheetMotion("back_walk");
				PrepareMoveTracking(-WHITE_BACK_SPEED * dir);
				m_velocity->SetVelocity(Vector2d(-WHITE_BACK_SPEED * dir, 0.0f));
			}

			if (distance >= WHITE_BACK_RANGE || m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 3:
			// éËó†åïÇÃéÀíˆì‡Ç≈í‚é~Ç∑ÇÈÅB
			PlaySheetMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (distance < WHITE_BACK_RANGE || distance >= WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_cooldownTimer <= 0.0f)
			{
				if (distance < WHITE_BACK_RANGE)
				{
					StartSwordAttack();
				}
				else
				{
					StartShurikenAttack();
				}
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 4:
			// ãﬂãóó£Ç≈í‚é~ÇµÇƒåïÇégÇ§ÅB
			PlaySheetMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

			if (m_cooldownTimer <= 0.0f)
			{
				StartSwordAttack();
			}
			else if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 5:
			PlaySheetMotion("walk");
			PrepareMoveTracking(WHITE_FAR_APPROACH_SPEED * dir);
			m_velocity->SetVelocity(Vector2d(WHITE_FAR_APPROACH_SPEED * dir, 0.0f));

			if (distance < WHITE_SHURIKEN_RANGE)
			{
				m_actionLock = false;
			}

			if (m_actionTimer >= WHITE_RECHECK_TIME)
			{
				m_actionLock = false;
			}
			break;

		case 6:
			// éÄñSèÛë‘Ç÷à⁄çsÇ∑ÇÈÅB
			SetState(Actor::State::Dead);
			break;
		}
	}
	else if (m_attackType == 1)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		m_attackType = 1;



		if (attackTime >= WHITE_SHURIKEN_SHOT_TIME && m_attackOnce == false)
		{
			float dir = GetDirSign();
			Vector2d bulletPos(myPos.x + 48.0f * dir, myPos.y - 30.0f);
			Vector2d bulletVel(WHITE_BULLET_SPEED * dir, 0.0f);

			m_scene->SpawnActor(
				new WhiteShurikenBullet(
					m_scene,
					bulletPos,
					bulletVel,
					WHITE_BULLET_DELETE_RANGE,
					WHITE_TEXTURE_SHURIKEN_BULLET,
					WHITE_SHURIKEN_DAMAGE,
					WHITE_SHURIKEN_BULLET_DRAW_SIZE,
					WHITE_SHURIKEN_BULLET_ROTATE_INTERVAL,
					WHITE_SHURIKEN_BULLET_ROTATE_STEP
				)
			);

			m_attackOnce = true;
			m_attackActive = true;
		}

		if (attackTime >= WHITE_SHURIKEN_END_TIME)
		{
			m_attackType = 0;

			m_cooldownTimer = WHITE_SHURIKEN_COOLDOWN;

			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
			m_attackType = 0;
		}
	}
	else if (m_attackType == 2)
	{
		m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));

		SetFacing(m_dir);

		m_attackTimer += deltaTime;
		float attackTime = m_attackTimer;

		m_attackActive = attackTime >= WHITE_SWORD_HIT_TIME && attackTime < WHITE_SWORD_ACTIVE_END_TIME;

		if (m_attackActive == true && m_attackOnce == false)
		{
			float attackRange = WHITE_SWORD_ATTACK_RANGE;
			float heightRange = WHITE_SWORD_HEIGHT_RANGE;

			float dx = playerPos.x - myPos.x;
			float dy = playerPos.y - myPos.y;

			bool hit = false;

			if (m_dir == true)
			{
				if (dx > 0.0f && dx < attackRange)
				{
					hit = true;
				}
			}
			else
			{
				if (dx < 0.0f && dx > -attackRange)
				{
					hit = true;
				}
			}

			if (hit == true)
			{
				if (dy < 0.0f)
				{
					dy *= -1.0f;
				}

				if (dy < heightRange)
				{
					if (player != nullptr)
					{
						float knockbackX = playerPos.x < myPos.x ? -WHITE_SWORD_KNOCKBACK_X : WHITE_SWORD_KNOCKBACK_X;
						player->TakeDamage(
							WHITE_SWORD_DAMAGE,
							Vector2d(knockbackX, WHITE_SWORD_KNOCKBACK_Y)
						);
						m_attackOnce = true;
					}
				}
			}
		}

		if (attackTime >= WHITE_SWORD_END_TIME)
		{
			m_cooldownTimer = WHITE_SWORD_COOLDOWN;
			m_actionTimer = 0.0f;
			m_actionLock = false;
			m_attackOnce = false;
			m_attackActive = false;
			m_attackType = 0;
		}
	}


	EnemyEntity::Update(deltaTime);

	if (m_hasLastMove && m_attackType == 0)
	{
		Vector2d movedPos = m_transform != nullptr ? m_transform->GetPosition() : Vector2d::Zero();
		float movedX = movedPos.x - m_lastMoveStartPos.x;
		bool wantedMove = m_lastWantedMoveX > WHITE_STUCK_MOVE_EPS || m_lastWantedMoveX < -WHITE_STUCK_MOVE_EPS;
		bool blockedMove = movedX > -WHITE_STUCK_MOVE_EPS && movedX < WHITE_STUCK_MOVE_EPS;

		if (wantedMove && blockedMove)
		{
			PlaySheetMotion("idle");
			m_velocity->SetVelocity(Vector2d(0.0f, 0.0f));
			m_actionLock = false;
		}
	}

	m_hasLastMove = false;
}

std::string WhiteEnemyEntity::GetTexturePath() const
{
	return WHITE_TEXTURE_IDLE;
}




