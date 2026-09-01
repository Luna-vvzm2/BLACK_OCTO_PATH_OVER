#include "EnemyBullet.h"
#include "Scene.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "CollisionComponent.h"
#include "HPComponent.h"
#include "PlayerEntity.h"
#include "SpriteComponent.h"

static const float ENEMY_BULLET_KNOCKBACK_X = 300.0f;
static const float ENEMY_BULLET_KNOCKBACK_Y = -200.0f;

EnemyBullet::EnemyBullet(
	Scene* scene,
	const Vector2d& pos,
	const Vector2d& velocity,
	float deleteRange,
	const std::string& texturePath,
	int damage,
	const Vector2d& drawSize,
	bool rotate,
	float rotateInterval,
	float rotateStep
)
	: EntityActor(scene, pos, Vector2d(12.0f, 12.0f))
	, m_startPos(pos)
	, m_bulletVelocity(velocity)
	, m_deleteRange(deleteRange)
	, m_texturePath(texturePath)
	, m_damage(damage)
	, m_drawSize(drawSize)
	, m_rotate(rotate)
	, m_rotateInterval(rotateInterval)
	, m_rotateStep(rotateStep)
	, m_rotationAngle(0.0f)
	, m_rotationTimer(0.0f)
{
}

bool EnemyBullet::Init()
{
	if (!EntityActor::Init()) return false;

	m_velocity->SetVelocity(m_bulletVelocity);
	m_collision->SetCircle(6.0f);
	m_transform->SetAngle(m_rotationAngle);
	if (m_drawSize.x > 0.0f && m_drawSize.y > 0.0f)
	{
		m_sprite->SetDrawSize(m_drawSize.x, m_drawSize.y);
	}

	return true;
}

void EnemyBullet::Update(float deltaTime)
{
	Actor::Update(deltaTime);

	Vector2d pos = m_transform->GetPosition();
	Vector2d vel = m_velocity->GetVelocity();

	if (m_rotate && m_rotateInterval > 0.0f)
	{
		m_rotationTimer += deltaTime;
		while (m_rotationTimer >= m_rotateInterval)
		{
			m_rotationTimer -= m_rotateInterval;
			m_rotationAngle += m_rotateStep;
		}
		m_transform->SetAngle(m_rotationAngle);
	}

	pos += vel * deltaTime;
	m_transform->SetPosition(pos);

	if ((pos - m_startPos).length() > m_deleteRange)
	{
		SetState(Actor::State::Dead);
		return;
	}

	if (TryDamagePlayer())
	{
		SetState(Actor::State::Dead);
	}
}

bool EnemyBullet::TryDamagePlayer()
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

		CollisionComponent* playerCollision = actor->GetComponent<CollisionComponent>();
		if (playerCollision == nullptr || m_collision == nullptr)
		{
			return false;
		}

		if (!m_collision->CheckCollision(playerCollision))
		{
			continue;
		}

		PlayerEntity* player = static_cast<PlayerEntity*>(actor);
		if (player != nullptr)
		{
			float knockbackX = player->GetPos().x < m_transform->GetPosition().x ? -ENEMY_BULLET_KNOCKBACK_X : ENEMY_BULLET_KNOCKBACK_X;
			player->TakeDamage(
				m_damage,
				Vector2d(knockbackX, ENEMY_BULLET_KNOCKBACK_Y)
			);
		}

		return true;
	}

	return false;
}

std::string EnemyBullet::GetTexturePath() const
{
	return m_texturePath;
}