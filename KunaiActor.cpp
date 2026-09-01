#include "KunaiActor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "Scene.h"
#include "EnemyEntity.h"
#include "BlockActor.h"

KunaiActor::KunaiActor(Scene* scene, const Vector2d& pos, bool dir) 
	: Actor(scene)
    , m_transform(nullptr)
    , m_velocity(nullptr)
    , m_collision(nullptr)
    , m_sprite(nullptr)
    , m_lifeTimer(0.0f)
    , m_spawnPos(pos)
    , m_dir(dir)
    , m_stopped(false)
{
}

bool KunaiActor::Init()
{
    m_transform = AddComponent<TransformComponent>();
    m_velocity = AddComponent<VelocityComponent>();
    m_collision = AddComponent<CollisionComponent>();
    m_sprite = AddComponent<SpriteComponent>("assets/images/entities/players/kunai.png", true);

    m_sprite->SetFlipX(!m_dir);

    m_spawnPos.x += m_dir ? 100.0f : -100.0f;

    m_transform->SetPosition(m_spawnPos);
    m_collision->SetRect(66, 16);

    Vector2d vel;

    vel.x = m_dir ? 1800.0f : -1800.0f;
    vel.y = 0.0f;

    m_velocity->Set(vel);

    return true;
}

void KunaiActor::Update(float deltaTime)
{
    Actor::Update(deltaTime);
    m_lifeTimer += deltaTime;

    if (m_lifeTimer > 10.0f)
    {
        SetState(State::Dead);
    }

    float dist = (m_transform->GetPosition().x - m_spawnPos.x);
    if (dist > 1800.0f)
    {
        SetState(State::Dead);
    }

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Enemy)
            continue;

        EnemyEntity* enemy = static_cast<EnemyEntity*>(actor);

        if (m_collision->CheckCollision(enemy->GetCollision()))
        {
            enemy->TakeDamage( 100, { m_dir ? 300.0f : -300.0f, 0 } );
            enemy->TakeMetsu(60);
            SetState(State::Dead);

            return;
        }
    }
}

void KunaiActor::CheckBlockCollision()
{
    for (Actor* actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Block)
        {
            continue;
        }

        BlockActor* block = static_cast<BlockActor*>(actor);

        CollisionComponent* blockCol = block->GetCollision();

        if (!blockCol)
        {
            continue;
        }

        float diffX = m_transform->GetPosition().x - block->GetPos().x;
        float diffY = m_transform->GetPosition().y - block->GetPos().y;

        float overlapX =
            (m_collision->GetWidth() * 0.5f +
                blockCol->GetWidth() * 0.5f)
            - std::abs(diffX);

        float overlapY =
            (m_collision->GetHeight() * 0.5f +
                blockCol->GetHeight() * 0.5f)
            - std::abs(diffY);

        if (overlapX > 0 && overlapY > 0)
        {
            OnBlockHit();
            return;
        }
    }
}

void KunaiActor::OnBlockHit()
{
    m_velocity->Set(Vector2d::Zero());

    m_stopped = true;
}