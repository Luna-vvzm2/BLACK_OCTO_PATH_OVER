
#include "YellowEnemyEntity.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "Vector2d.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "DropItemEntity.h"
#include "EntityActor.h"
#include "AnimationComponent.h"
#include "SpriteComponent.h"

#include <cmath>

YellowEnemyEntity::YellowEnemyEntity(Scene* scene, const Vector2d& pos) : EnemyEntity(scene, pos, Vector2d(96, 190))
{


}


bool YellowEnemyEntity::Init() {
    if (!EnemyEntity::Init()) return false;

    m_hp = AddComponent<HPComponent>(600);
    m_metsuMax = 100;

    // 攻撃判定
    m_attackCollision = AddComponent<CollisionComponent>();
    m_attackCollision->SetNone();
    m_state = Idle;
    m_attackTimer = 0.0f;

    m_faceRight = true;

    // =========================
 // ドロップ設定（テスト用）
 // =========================
    m_dropTable.clear();

    // 必ずコインを落とす
    m_dropTable.push_back({ ItemType::Coin, 1.0f });

    m_sprite->LoadTextureDiv(
        GetTexturePath(),
        4,
        11
    );
    m_animation = AddComponent<AnimationComponent>();
    m_animation->SetSprite(m_sprite);

    AnimationClip stay;
    stay.frames = { 0,1,2,3,4 };
    stay.speed = 0.12f;
    stay.loop = true;


    AnimationClip walk;
    walk.frames = { 5,6,7,8,9,10,11,12,13,14 };
    walk.speed = 0.12f;
    walk.loop = true;

    AnimationClip attackReady;
    attackReady.frames = { 15,16,17,18 };
    attackReady.speed = 0.12f;
    attackReady.loop = false;

    AnimationClip attack1;
    attack1.frames = { 19,20,21 };
    attack1.speed = 0.08f;
    attack1.loop = false;

    AnimationClip attack2;
    attack2.frames = { 22,23,24,25 };
    attack2.speed = 0.08f;
    attack2.loop = false;

    AnimationClip hit;
    hit.frames = { 26,27,28,29, };
    hit.speed = 0.08f;
    hit.loop = false;

    AnimationClip hitback;
    hitback.frames = { 26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42 };
    hitback.speed = 0.08f;
    hitback.loop = false;


    AnimationClip dead;
    dead.frames = { 26,27,28,29,30,31,32,33,34,35,36,37,38,39 };
    dead.speed = 0.1f;
    dead.loop = false;

    // ④ 登録
    m_animation->AddClip("Stay", stay);
    m_animation->AddClip("Walk", walk);
    m_animation->AddClip("AttackReady", attackReady);
    m_animation->AddClip("Attack1", attack1);
    m_animation->AddClip("Attack2", attack2);
    m_animation->AddClip("Hit", hit);
    m_animation->AddClip("HitBack", hitback);
    m_animation->AddClip("Dead", dead);

    // ⑤ 最初は待機アニメーション
    m_animation->Play("Stay");
    m_transform->SetScale(Vector2d(0.4f, 0.4f));
    return true;
}

void YellowEnemyEntity::Update(float deltaTime) {

    m_isHit = false;
    // 先に死亡済みなら何もしない
    if (GetState() == Actor::State::Dead)
        return;

    if (m_state == BlowHit)
    {
        if (m_animation->GetCurrentName() != "Hit")
        {
            m_animation->Play("Hit");
        }
        m_hitTimer -= deltaTime;

        if (m_hitTimer <= 0.0f)
        {
            m_state = Idle;
        }

        EnemyEntity::Update(deltaTime);
        return;
    }


    PlayScene* playScene = static_cast<PlayScene*>(GetScene());
    PlayerEntity* player = playScene ? playScene->GetPlayer() : nullptr;
    Vector2d vel = m_velocity->Get();

    if (!player) {
        vel.x = 0.0f;
        m_velocity->Set(vel);
        EnemyEntity::Update(deltaTime);
        return;
    }
    float distance =
        player->GetPos().x - GetPos().x;


    if (m_hp && m_hp->GetHP() <= 0 && !m_isDying)
    {
        m_isDying = true;
        m_state = Dead;

        m_deadTimer = 0.0f;

        m_velocity->Set(Vector2d::Zero());

        m_animation->Play("Dead", true);
    }

    switch (m_state)
    {

    case Idle:
        m_sprite->SetDrawOffset(0.0f, 0.0f);
        if (m_animation->GetCurrentName() != "Stay")
        {
            m_animation->Play("Stay");
        }
        vel.x = 0.0f;
        if (std::abs(distance) < m_detectRange)
        {
            m_state = Chase;
        }
        break;

    case Dead:
    {
        vel.x = 0.0f;

        m_deadTimer += deltaTime;
        m_sprite->SetDrawOffset(0.0f, 10.0f);
        if (m_deadTimer > 1.0f)
        {
            float t = (m_deadTimer - 1.0f) / 1.0f;

            int alpha = (int)(255 * (1.0f - t));

            m_sprite->SetAlpha(alpha);
        }
        if (m_deadTimer >= 1.5f)
        {
            OnDead();
            return;
        }

        break;
    }
    case Chase:
        m_sprite->SetDrawOffset(0.0f, -10.0f);
        if (m_animation->GetCurrentName() != "Walk")
        {
            m_animation->Play("Walk");
        }
        if (distance > 0)
        {
            m_faceRight = true;
            m_sprite->SetFlipX(true);
            vel.x = m_moveSpeed;
        }
        else
        {
            m_faceRight = false;
            m_sprite->SetFlipX(false);
            vel.x = -m_moveSpeed;
        }

        if (std::abs(distance) < m_attackRange)
        {
            m_state = AttackReady;
            m_attackTimer = 0.6f;
            vel.x = 0.0f;
        }
        break;

    case AttackReady:
        m_sprite->SetDrawOffset(0, -20.0f);
        if (m_animation->GetCurrentName() != "AttackReady")
        {
            m_animation->Play("AttackReady");
        }
        vel.x = 0.0f;
        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Attack1;
            m_attackTimer = 0.3f;
            m_attackHit = false;
            Vector2d pos = GetPos();

            if (m_faceRight)
                pos.x += 100.0f;
            else
                pos.x -= 100.0f;

            m_transform->SetPosition(pos);
        }


        break;

    case Attack1:
        m_sprite->SetDrawOffset(0.0f, -20.0f);
        if (m_animation->GetCurrentName() != "Attack1")
        {
            m_animation->Play("Attack1");
        }
        vel.x = 0.0f;

        if (m_faceRight)
        {
            m_attackCollision->SetRect(150.0f, 200.0f);
            m_attackCollision->SetOffset(
                Vector2d(100.0f, 0.0f));

        }
        else
        {
            m_attackCollision->SetRect(150.0f, 200.0f);
            m_attackCollision->SetOffset(
                Vector2d(-100.0f, 0.0f));

        }

        m_attackTimer -= deltaTime;
        if (m_attackTimer <= 0)
        {
            m_state = Attack2;
            m_attackTimer = 0.3f;
            m_attackHit = false;
        }
        break;

    case Attack2:
        m_sprite->SetDrawOffset(0.0f, -20.0f);
        if (m_animation->GetCurrentName() != "Attack2")
        {
            m_animation->Play("Attack2");
        }
        vel.x = 0.0f;

        if (m_faceRight)
        {
            m_attackCollision->SetRect(300.0f, 200.0f);
            m_attackCollision->SetOffset(
                Vector2d(0.0f, 0.0f));
        }
        else
        {
            m_attackCollision->SetRect(300.0f, 200.0f);
            m_attackCollision->SetOffset(
                Vector2d(0.0f, 0.0f));
        }

        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Recovery;
            m_attackTimer = 1.0f;
        }
        break;

    case Recovery:
        m_sprite->SetDrawOffset(0.0f, 0.0f);
        if (m_animation->GetCurrentName() != "Stay")
        {
            m_animation->Play("Stay");
        }
        vel.x = 0.0f;
        m_attackCollision->SetNone();
        m_attackCollision->SetOffset(
            Vector2d::Zero());
        m_attackTimer -= deltaTime;

        if (m_attackTimer <= 0)
        {
            m_state = Idle;
        }
        break;
    }
    auto playerCollision =
        player->GetComponent<CollisionComponent>();

    if ((m_state == Attack1 || m_state == Attack2) &&
        playerCollision &&
        m_attackCollision->CheckCollision(playerCollision))
    {
        if (!m_attackHit)
        {
            Vector2d knockback;

            if (m_faceRight)
            {
                knockback = Vector2d(2000.0f,400.0f);
            }
            else
            {
                knockback = Vector2d(-2000.0f,-400.0f);
            }

            if (m_state == Attack1)
            {
                player->TakeDamage(15, knockback);
            }
            else
            {
                player->TakeDamage(20, knockback);
            }

            m_attackHit = true;
        }
    }
    m_velocity->Set(vel);
    EnemyEntity::Update(deltaTime);

}


std::string YellowEnemyEntity::GetTexturePath() const {
    return "assets/images/Enemy/yellow.png";
}


void YellowEnemyEntity::Draw()
{
    EnemyEntity::Draw();

#ifdef _DEBUG

    m_attackCollision->DrawDebug();

#endif
}

void YellowEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
{
    if (m_hp == nullptr)
    {
        return;
    }

    m_hp->Damage(damage);

    if (m_hp->GetHP() <= 0)
    {
        m_state = Dead;
        return;
    }

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

        force.x = std::fabs(force.x) * sign;
    }

    m_velocity->SetVelocity(force);

    m_hitTimer = 0.3f;
    m_attackCollision->SetNone();
    m_attackCollision->SetOffset(Vector2d::Zero());
}


