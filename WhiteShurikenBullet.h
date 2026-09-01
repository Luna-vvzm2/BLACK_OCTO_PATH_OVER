#pragma once
#include "EntityActor.h"
#include "PlayerEntity.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "VelocityComponent.h"

class WhiteShurikenBullet : public EntityActor
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
		: EntityActor(scene, pos, Vector2d(12.0f, 12.0f))
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
			float knockbackX = player->GetPos().x < m_transform->GetPosition().x ? -300.0f : 300.0f;
			player->TakeDamage(m_damage, Vector2d(knockbackX, -200.0f));
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
