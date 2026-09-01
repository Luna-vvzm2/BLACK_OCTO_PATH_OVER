#include "ScarecrowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Key.h"
#include "Input.h"
#include "Game.h"
#include "Scene.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"


ScarecrowEnemyEntity::ScarecrowEnemyEntity(
    Scene* scene,
    const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(96, 190))
{
}

bool ScarecrowEnemyEntity::Init()
{
    if (!EnemyEntity::Init())
        return false;

    m_velocity = GetComponent<VelocityComponent>();
    m_state = Idle;
    // AnimationComponent取得
    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    m_sprite->LoadTextureDiv(
        "assets/images/Enemy/scarecrow.png",
        4,
        5
    );

    AnimationClip idle;
    idle.frames = { 0 };
    idle.speed = 0.15f;
    idle.loop = true;
    m_anim->AddClip("idle", idle);

    AnimationClip hit;
    hit.frames = { 1,2,3,4 };
    hit.speed = 0.15f;
    hit.loop = false;
    m_anim->AddClip("hit", hit);

    AnimationClip launch;
    launch.frames = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
    launch.speed = 0.15f;
    launch.loop = false;
    m_anim->AddClip("launch", launch);


    if (m_anim->GetCurrentName() != "idle")
    {
        m_anim->Play("idle");
    }
    m_transform->SetScale(Vector2d(0.4f, 0.4f));
    
    return true;
}

void ScarecrowEnemyEntity::Update(float deltaTime)
{

    m_sprite->SetFlipH(m_faceRight);

    if (m_state == BlowHit)
    {
        if (m_anim->GetCurrentName() != "hit")
        {
            m_anim->Play("hit");
        }
        m_hitTimer -= deltaTime;

        if (m_hitTimer <= 0.0f)
        {
            m_state = Idle;

            Vector2d vel = m_velocity->GetVelocity();
            vel.x = 0.0f;
            m_velocity->SetVelocity(vel);
        }

        EnemyEntity::Update(deltaTime);
        return;
    }
    switch (m_state)
    {
    case Idle:
        UpdateIdle(deltaTime);
        break;

    }

    EnemyEntity::Update(deltaTime);

    if (m_anim)
    {
        m_anim->Update(deltaTime);
    }

}

void ScarecrowEnemyEntity::Draw()
{
    EnemyEntity::Draw();
}

void ScarecrowEnemyEntity::UpdateIdle(float dt)
{
    m_sprite->SetDrawOffset(0.0f, 15.0f);
    if (m_anim->GetCurrentName() != "idle")
    {
        m_anim->Play("idle");
    }
}






void ScarecrowEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{


    // 吹っ飛び開始
    m_state = BlowHit;

    // プレイヤーと逆方向へ飛ばす
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

        // プレイヤーから離れる方向へ吹っ飛ぶ
        force.x = std::fabs(force.x) * sign;

        // プレイヤーのいる方向を向く
        if (player->GetPos().x > GetPos().x)
        {
            m_faceRight = true;
        }
        else
        {
            m_faceRight = false;
        }
    }
    m_velocity->SetVelocity(force);

    m_hitTimer = 0.3f;
    m_metsuGauge = 0;
}


