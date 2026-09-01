#include "ArrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "CollisionComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "EnemyBullet.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "Renderer.h"
#include "Game.h"
#include "BlockActor.h"
#include "DropItemEntity.h"
#include "EntityActor.h"
#include <cmath>


ArrowEnemyEntity::ArrowEnemyEntity(Scene* scene, const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(96, 190))
    , m_attackState(AttackState::Idle)
    , m_attackTimer(0.0f)
    , m_attackInterval(3.0f)
    , m_attackExecuted(false)
    , m_deathTimer(0.0f)
    , m_hpTestTimer(0.0f)

{
}
bool ArrowEnemyEntity::Init() {
	if (!EnemyEntity::Init()) return false;

	m_hp = AddComponent<HPComponent>(420);


    m_dropTable.clear();

    m_dropTable.push_back({ ItemType::Coin, 1.0f });
    m_dropTable.push_back({ ItemType::Kunai,1.0f });
	

    m_sprite->LoadTextureDiv(
        GetTexturePath(),
        4,      // 列数（例）
        7       // 行数（例）
    );

    m_animation = AddComponent<AnimationComponent>();
    m_animation->SetSprite(m_sprite);

    AnimationClip idle;
    idle.frames = {0};
    idle.speed = 0.18f;
    idle.loop = true;

    AnimationClip warning;
    warning.frames = {1,2,3,4,5,6,7,8};
    warning.speed = 0.10f;
    warning.loop = false;

    AnimationClip attack;
    attack.frames = { 9,0 };
    attack.speed = 0.08f;
    attack.loop = false;

    AnimationClip hit;
    hit.frames = {10,11,12,13};
    hit.speed = 0.08f;
    hit.loop = false;

    AnimationClip dead;
    dead.frames = {10,11,12,13,14,15,16,17,18,19,20,21,22};
    dead.speed = 0.10f;
    dead.loop = false;

    m_animation->AddClip("Idle", idle);
    m_animation->AddClip("Warning", warning);
    m_animation->AddClip("Attack", attack);
    m_animation->AddClip("Hit", hit);
    m_animation->AddClip("Dead", dead);

    m_animation->Play("Idle");

    m_transform->SetScale(Vector2d(0.4f, 0.4f));

    return true;

}

void ArrowEnemyEntity::Update(float deltaTime)
{
   
    // ?????S????牽??????
    if (GetState() == Actor::State::Dead)
        return;

    /*m_damageTimer += deltaTime;
    if (m_damageTimer >= 1.0f)
    {
        m_damageTimer -= 1.0f;

        if (m_hp)
        {
            m_hp->Damage(10);
        }
    }*/

 

    auto player =
        static_cast<PlayScene*>(m_scene)->GetPlayer();
    if (!player)
    {
        EnemyEntity::Update(deltaTime);
        return;
    }

    // ?v???C???[??????
    float distance =
        std::abs(player->GetPos().x - GetPos().x);

    switch (m_attackState)
    {
    case AttackState::Idle:
        m_sprite->SetDrawOffset(0.0f, -15.0f);
        m_animation->Play("Idle");
        if (distance <= m_detectRange)
        {
            m_attackTimer += deltaTime;

            if (m_attackTimer >= 3.0f)
            {
                m_attackTimer = 2.0f;
                m_attackExecuted = false;
                m_attackState = AttackState::Warning;
            }
        }
        

        if (player->GetPos().x > GetPos().x)
        {
            m_sprite->SetFlipX(true);
        }
        else
        {
            m_sprite->SetFlipX(false);
        }
        break;

    case AttackState::Warning:

        if (m_animation->GetCurrentName() != "Warning")
        {
            m_animation->Play("Warning");
        }

        m_attackTimer -= deltaTime;

        
        if (!m_attackExecuted && m_attackTimer <= 1.0f)
        {
            m_targetPos.x = player->GetPos().x;
            m_targetPos.y = GetGroundY(m_targetPos.x);

            m_attackExecuted = true;
        }

        if (m_attackTimer <= 0.0f)
        {
            
            m_attackTimer = 0.2f;
            m_attackState = AttackState::Attack;
            m_attackExecuted = false;
        }

        break;

    case AttackState::Attack:
        if (m_animation->GetCurrentName() != "Attack")
        {
            m_animation->Play("Attack");
        }

        if (!m_attackExecuted)
        {
            Vector2d playerPos = player->GetPos();

            float radius = 200.0f;

            float dx = playerPos.x - m_targetPos.x;
            float dy = playerPos.y - m_targetPos.y;

            float distanceSq = dx * dx + dy * dy;
            float groundY = GetGroundY(m_targetPos.x);

            float lineHalfW = 32.0f;
            float lineCenterY = groundY * 0.5f;
            float lineHalfH = groundY * 0.5f;

            bool hitLine =
                std::abs(playerPos.x - m_targetPos.x) <= lineHalfW &&
                std::abs(playerPos.y - lineCenterY) <= lineHalfH;
            bool hitCircle = distanceSq <= radius * radius;

            if (hitLine || hitCircle)
            {
                Vector2d knockback;

                if (playerPos.x < m_targetPos.x)
                {
                    knockback = Vector2d(-800.0f, -250.0f);
                }
                else
                {
                    knockback = Vector2d(800.0f, -250.0f);
                }

                player->TakeDamage(20, knockback);
            }

            m_attackExecuted = true;
        }

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackTimer = 1.0f;
            m_attackState = AttackState::Cooldown;
        }

        break;


    case AttackState::Cooldown:
        m_animation->Play("Idle");
        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0.0f)
        {
            m_attackState = AttackState::Idle;
        }

        break;

    case AttackState::Hit:
        if (m_animation->GetCurrentName() != "Hit")
        {
            m_animation->Play("Hit");
        }
        m_hitTimer -= deltaTime;

        if (m_hitTimer <= 0.0f)
        {
            SetVel(Vector2d::Zero());

            m_attackState = AttackState::Idle;
        }

        break;

    case AttackState::Dead:
        m_sprite->SetDrawOffset(0.0f, -20.0f);
        if (m_animation->GetCurrentName() != "Dead")
        {
            m_animation->Play("Dead");
        }

        if (m_deathTimer > 1.0f)
        {
            float t = (m_deathTimer - 1.0f) / 1.0f;

            int alpha = (int)(255 * (1.0f - t));

            m_sprite->SetAlpha(alpha);

        }
        SetVel(Vector2d::Zero());

        m_deathTimer += deltaTime;

        if (m_deathTimer >= 1.5f)
        {
            OnDead();
            return;
        }

        break;
    }

    EnemyEntity::Update(deltaTime);
}


void ArrowEnemyEntity::Draw()
{
   

    EnemyEntity::Draw();



    Renderer* renderer =
        m_scene->GetGame()->GetRenderer();
    if (!renderer)
        return;
    float groundY = GetGroundY(m_targetPos.x);
    if (m_attackState == AttackState::Warning)
    {
        float height = groundY;

        renderer->DrawRectCenter(
            Vector2d(m_targetPos.x, height*0.5f),
            64.0f, height,
            Color(255, 255, 0), false);
    }

    if (m_attackState == AttackState::Attack)
    {
        float height = groundY;
        renderer->DrawRectCenter(
            Vector2d(m_targetPos.x, height * 0.5f),
            64.0f, height,
            Color(255, 0, 0), false);
        renderer->DrawCircle(
            m_targetPos,
            200.0f,
            Color(255, 0, 0),
            false);
    }

}

float ArrowEnemyEntity::GetGroundY(float x)
{
    float groundY = 100000.0f;

    for (auto actor : m_scene->GetActors())
    {
        if (actor->GetType() != ActorType::Block)
            continue;

        auto block = static_cast<BlockActor*>(actor);

        Vector2d blockPos = block->GetPos();

        auto col = block->GetCollision();
        if (!col)
            continue;

        float halfW = col->GetWidth() * 0.5f;

        float left = blockPos.x - halfW;
        float right = blockPos.x + halfW;

        if (x >= left && x <= right)
        {
            float topY =
                blockPos.y - col->GetHeight() * 0.5f;

            if (topY < groundY)
            {
                groundY = topY;
            }
        }
    }

    return groundY;
}

void ArrowEnemyEntity::TakeDamage(int damage,
    const Vector2d& knockback)
{
    if (m_hp == nullptr)
    {
        return;
    }

    m_hp->Damage(damage);

    if (m_hp->GetHP() <= 0)
    {
        m_attackState = AttackState::Dead;
        m_deathTimer = 0.0f;
        return;
    }

    m_attackState = AttackState::Hit;

    Vector2d force = knockback;

    PlayScene* playScene =
        static_cast<PlayScene*>(GetScene());

    PlayerEntity* player =
        playScene ? playScene->GetPlayer() : nullptr;

    if (player)
    {
        float sign =
            GetPos().x < player->GetPos().x ?
            -1.0f : 1.0f;

        force.x = std::fabs(force.x) * sign;
    }

    SetVel(force);

    m_hitTimer = 0.3f;
}
std::string ArrowEnemyEntity::GetTexturePath() const {
    
    return "assets/images/Enemy/arrow.png";

}
