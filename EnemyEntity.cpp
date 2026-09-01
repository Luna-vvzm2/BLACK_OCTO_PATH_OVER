#include "EnemyEntity.h"
#include "BlockActor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "Game.h"
#include "PlayScene.h"
#include "DropData.h"
#include "DropItemEntity.h"
#include <cstdlib>

EnemyEntity::EnemyEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_hp(nullptr)
    , m_hpMax(100)
    , m_gravity(nullptr)
    , m_anim(nullptr)

    , m_dir(true)
    , m_jumpSpeed(0.0f)
    , m_moveSpeed(200.0f)

    , m_guard(0)
    , m_guardMax(100)

    , m_metsuGauge(0)
    , m_metsuMax(75)
    , m_metsu(0)

    , m_damageCancel(0)
    , m_attackActive(0)

    , m_findPlayer(0)

    , m_attack(false)
    , m_attackType(0)
    , m_attackTimer(0.0f)

    , m_actionTimer(0.0f)
    , m_cooldownTimer(0.0f)
    , m_actionLock(false)

    , m_canMove(true)
{
}

bool EnemyEntity::Init() {
    if (!EntityActor::Init())
        return false;

    m_hp = AddComponent<HPComponent>(GetMaxHP());
    m_gravity = AddComponent<GravityComponent>(2800.0f);

    // テスト用：必ずコインを1つ落とす
    m_dropTable.clear();
    m_dropTable.push_back({ ItemType::Coin, 1.0f });
    m_dropTable.push_back({ ItemType::Kunai,0.9f });
    m_dropTable.push_back({ ItemType::Heal,0.9f });
    return true;
}

void EnemyEntity::Update(float deltaTime) {
   
    EntityActor::Update(deltaTime);

   
    if (GetState() == Actor::State::Dead)
        return;

  

    UpdateAI();

    UpdateGravity(deltaTime);



}

void EnemyEntity::UpdateGravity(float deltaTime)
{
    if (!m_velocity)
        return;

    Vector2d vel = m_velocity->Get();

    // 空中にいるときだけ重力を加える
    if (!m_isGround)
    {
        const float gravity = 1800.0f;
        vel.y += gravity * deltaTime;

        const float maxFallSpeed = 1200.0f;
        if (vel.y > maxFallSpeed)
        {
            vel.y = maxFallSpeed;
        }
    }

    // ★重力を反映した速度を戻す
    m_velocity->Set(vel);

    // 移動＆地形衝突
    MoveAndCollide(deltaTime);

    // 着地したら縦速度を止める
    if (m_isGround)
    {
        Vector2d stopVel = m_velocity->Get();
        stopVel.y = 0.0f;
        m_velocity->Set(stopVel);
    }
}
void EnemyEntity::OnDead()
{
    if (GetState() == Actor::State::Dead)
        return;


    for (const auto& drop : m_dropTable)
    {
        float r = static_cast<float>(rand()) / RAND_MAX;

        if (r <= drop.probability)
        {
            SpawnItem(drop.type);
        }
    }
    SetState(Actor::State::Dead);
}


void EnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{   
    if (m_hp == nullptr)
    {
        return;
    }

    m_hp->Damage(damage);

    if (m_hp->GetHP() <= 0)
    {
        OnDeadFromDamage(damage, knockback);

        return;
    }

    Vector2d vel = m_velocity->Get();
    vel = knockback;
    m_velocity->Set(vel);

    OnDamaged(damage, knockback);
}

void EnemyEntity::TakeMetsu(int metsu)
{
    m_metsuGauge += metsu;

    if (m_metsuGauge >= m_metsuMax && !m_metsu)
    {
        m_metsuGauge = m_metsuMax;
        m_metsu = true;

        PlayScene* play = static_cast<PlayScene*>(m_scene);
    }
}

void EnemyEntity::MetsuAttacked()
{
    // 追加ドロップ

    // エフェクト


    // 死亡
    m_hp->Damage((int)m_hp->GetHP());
    OnDead();
}

void EnemyEntity::OnDeadFromDamage(int damage, const Vector2d& knockback)
{
    OnDead();
}

std::string EnemyEntity::GetTexturePath() const {
    return "";
}

void EnemyEntity::SpawnItem(ItemType type)
{
    if (!m_scene || !m_transform)
        return;

    Vector2d pos = m_transform->GetPosition();
    pos.y -= 80.0f;

    m_scene->AddActor(new DropItemEntity(m_scene, pos, type));
}
/*

    auto bullet =
        new EnemyBullet(m_scene,縲GetPos());
    
    m_scene->AddActor(bullet);


*/
void EnemyEntity::UpdateMove(float deltaTime)
{
    if (m_velocity) {
        Vector2d v = m_velocity->Get();
        v.x = (m_dir ? 1.0f : -1.0f) * m_moveSpeed;
        m_velocity->Set(v);
    }
}

void EnemyEntity::UpdateAttack(float deltaTime)
{
}

void EnemyEntity::UpdateState()
{
}