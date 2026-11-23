#include "EnemyEntity.h"
#include "BlockActor.h"
#include "TransformComponent.h"
#include "VelocityComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "CollisionComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "AlertGaugeComponent.h"
#include "Game.h"
#include "PlayScene.h"
#include "DropData.h"
#include "DropItemEntity.h"
#include "PlayerEntity.h"
#include "Renderer.h"
#include "Scene.h"
#include <cstdlib>
#include <iostream>
#include <DxLib.h>

EnemyEntity::EnemyEntity(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EntityActor(scene, pos, size)
    , m_hp(nullptr)
    , m_hpMax(100)
    , m_alertGauge(nullptr)
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
    , m_alertGaugeTimer(0.0f)
    , m_alertChangeTimer(0.0f)
    , m_wasPlayerInSearchRange(false)

    , m_searchRangeGroundY(0.0f)
    , m_searchRangeGroundYInitialized(false)
{
}

bool EnemyEntity::Init() {
    if (!EntityActor::Init())
        return false;

    SetChaseFlag(false);
    m_hp = AddComponent<HPComponent>(GetMaxHP());
    m_gravity = AddComponent<GravityComponent>(2800.0f);
    m_alertGauge = AddComponent<AlertGaugeComponent>();

    // テスト用：必ずコインを1つ落とす
    m_dropTable.clear();
    m_dropTable.push_back({ ItemType::Coin, 0.0f });
    m_dropTable.push_back({ ItemType::Kunai,0.0f });
    m_dropTable.push_back({ ItemType::Heal,1.15f });
    return true;

}

void EnemyEntity::Update(float deltaTime)
{
    EntityActor::Update(deltaTime);

    if (GetState() == Actor::State::Dead)
    {
        SetChaseFlag(false);
        return;
    }

    UpdateAI();

    if (UseAlertGauge())
    {
        UpdateAlertGauge(deltaTime);
    }

    UpdateGravity(deltaTime);

    if (m_attackBuffTimer > 0.0f)
    {
        m_attackBuffTimer -= deltaTime;

        if (m_attackBuffTimer <= 0.0f)
        {
            m_attackBuffTimer = 0.0f;
            m_attackPowerMultiplier = 1.0f;
        }
    }
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

    //地面に着地している場合、足元のY座標を保存
    if (m_isGround)
    {
        Vector2d pos = GetPos();
        Vector2d size = GetSize();

        m_searchRangeGroundY = pos.y + size.y * 0.5;
        m_searchRangeGroundYInitialized = true;
    }

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
    SetChaseFlag(false);
    if (GetState() == Actor::State::Dead)
        return;

    if (m_scene != nullptr)
    {
        const float alertRange = 300.0f;
        const float alertAmount = 20.0f;

        Vector2d myPos = GetPos();

        for (Actor* actor : m_scene->GetActors())
        {
            if (actor == nullptr || actor == this)
                continue;

            if (actor->GetType() != ActorType::Enemy)
                continue;


            if (actor->GetState() == Actor::State::Dead)
                continue;
            EnemyEntity* enemy = dynamic_cast<EnemyEntity*>(actor);

            if (enemy == nullptr || enemy->m_alertGauge == nullptr)
                continue;
            Vector2d enemyPos = enemy->GetPos();

            float distanceX = std::abs(myPos.x - enemyPos.x);
            float distanceY = std::abs(myPos.y - enemyPos.y);

            // 死亡した敵が対象の敵の索敵範囲内にいる場合
            if (distanceX <= enemy->SEARCH_RANGE_X &&
                distanceY <= enemy->SEARCH_RANGE_Y)
            {
                if (enemy->m_alertGauge->GetGauge() < 50.0f)
                {
                    enemy->m_alertGauge->SetGauge(50.0f);
                }
            }
        }
    }

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


void EnemyEntity::TakeDamage(int damage, const Vector2d& knockback, bool attackDir)
{   
    if (m_hp == nullptr)
    {
        return;
    }
    
    int finalDamage = damage;

    //無防備状態か判定
    const bool isDefenseless = m_alertGauge != nullptr && m_alertGauge->IsDefenseless();

    const bool isCombat = m_alertGauge != nullptr && m_alertGauge->IsCombat();
    float multiplier = 1.0f;

    if (isDefenseless)
    {
        ;
        //無防備状態の倍率
        switch (m_enemyRank)
        {
        case EnemyRank::Small:
            multiplier = 20.0f;
            break;

        case EnemyRank::Elite:
            multiplier = 3.0f;
            break;

        case EnemyRank::Boss:
            multiplier = 2.0f;
            break;
        }
    }
    else if (isCombat)
    {
        //プレイヤーの位置から攻撃方向を判定
        PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);

        PlayerEntity* player = nullptr;
        if (playScene != nullptr)
        {
            player = playScene->GetPlayer();
        }

        if (player != nullptr && !player->IsDead())
        {
            const float playerX = player->GetPos().x;
            const float enemyX = GetPos().x;

            //m_dir == true は右向きと仮定
            bool isPlayerInFront = m_dir ? playerX > enemyX : playerX < enemyX;
            if (attackDir) { isPlayerInFront = !isPlayerInFront; }

            if (isPlayerInFront)
            {
                //前方からの攻撃
                switch (m_enemyRank)
                {
                case EnemyRank::Small:
                    multiplier = 0.7f;
                    break;

                case EnemyRank::Elite:
                    multiplier = 0.5f;
                    break;

                case EnemyRank::Boss:
                    multiplier = 0.3f;
                    break;
                }
            }
            else
            {
                //後方からの攻撃
                switch (m_enemyRank)
                {
                case EnemyRank::Small:
                    multiplier = 20.0f;
                    break;

                case EnemyRank::Elite:
                    multiplier = 3.0f;
                    break;

                case EnemyRank::Boss:
                    multiplier = 2.0f;
                    break;
                }
            }
        }
    }

    finalDamage = static_cast<int>(damage * multiplier);

    //ダメージを適用
    m_hp->Damage(finalDamage);

    if (m_alertGauge != nullptr)
    {
        if (isDefenseless)
        {
            //ダメージ適用後、ゲージを0にリセット
            m_alertGauge->SetGauge(0.0f);

            //10秒間、ゲージがマイナスになるのを制限
            m_defenselessLockTimer = 10.0f;


            if (m_isInSmoke)
            {
                m_isInSmoke = false;

                // 警戒ゲージを臨戦状態にする
                m_alertGauge->SetGauge(100.0f);

                // 煙幕による追跡解除状態を解除し、
                // 臨戦状態のAI処理へ移行する
            }
        }
        else
        {
            //通常攻撃では、攻撃命中ごとに＋8
            m_alertGauge->AddGauge(8.0f);
        }
    }

    if (m_hp->GetHP() <= 0)
    {
        OnDeadFromDamage(finalDamage, knockback);
        return;
    }

    if (m_velocity != nullptr)
    {
        m_velocity->Set(knockback);
    }

    if (finalDamage > 0) FacePlayerOnDamage(knockback);
    OnDamaged(finalDamage, knockback);

    Vector2d vel = m_velocity->Get();
    vel = knockback;
    m_velocity->Set(vel);

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

bool EnemyEntity::IsPlayerInSearchRange() const
{
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);
    if (playScene == nullptr) return false;

    PlayerEntity* player = playScene->GetPlayer();
    if (player == nullptr || player->IsDead()) return false;

    // 隠れているプレイヤーは索敵対象にしない
    if (player->GetIsHide())
    {
        return false;
    }

    if (player->GetIsJutsu(0) && m_chaseFlag == false) {
        return false;
    }

    Vector2d enemyPos = GetPos();
    Vector2d playerPos = player->GetPos();

    float distanceX = std::abs(playerPos.x - enemyPos.x);
    float distanceY = std::abs(playerPos.y - enemyPos.y);

    return distanceX <= SEARCH_RANGE_X &&
        distanceY <= SEARCH_RANGE_Y;
}

void EnemyEntity::UpdateAlertGauge(float deltaTime)
{
    // 無防備状態解除後の制限時間を減らす
    if (m_defenselessLockTimer > 0.0f)
    {
        m_defenselessLockTimer -= deltaTime;

        if (m_defenselessLockTimer < 0.0f)
        {
            m_defenselessLockTimer = 0.0f;
        }
    }

    if (m_alertGauge == nullptr)
    {
        return;
    }
    // 蛸で隠れているか確認
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);

    if (playScene != nullptr)
    {
        
        PlayerEntity* player = playScene->GetPlayer();

        if (player != nullptr && !player->IsDead())
        {
            if (player->GetIsHide())
            {
                //隠れている間は警戒ゲージを減らす
                m_alertGaugeTimer += deltaTime;

                if (m_alertGaugeTimer >= 1.0f)
                {
                    m_alertGaugeTimer = 0.0f;
                    m_alertGauge->AddGauge(-1.0f);
                }

                // 通常の索敵によるゲージ増加を行わない
                return;
            }
        }
    }

    m_alertGaugeTimer += deltaTime;

    if (m_alertGaugeTimer < 1.0f)
    {
        return;
    }

    m_alertGaugeTimer = 0.0f;

    const bool isPlayerInRange = IsPlayerInSearchRange();
    // 範囲内・範囲外が切り替わったら増減量をリセット
    if (isPlayerInRange != m_wasPlayerInSearchRange)
    {
        m_alertChangeTimer = 0.0f;
    }

    m_wasPlayerInSearchRange = isPlayerInRange;

    // 3 → 6 → 9 → ... → 30
    m_alertChangeTimer += 3.0f;

    float changeAmount = m_alertChangeTimer;

    if (changeAmount > 30.0f)
    {
        changeAmount = 30.0f;
    }

    if (isPlayerInRange)
    {
        // +1 → +10
        m_alertGauge->AddGauge(changeAmount);
    }
    else
    {
        if (m_defenselessLockTimer > 0.0f)
        {
            if (m_alertGauge->GetGauge() > 0.0f)
            {
                m_alertGauge->AddGauge(-1.0f);

                if (m_alertGauge->GetGauge() < 0.0f)
                {
                    m_alertGauge->SetGauge(0.0f);
                }
            }
        }
        else
        {
            //通常の減少処理
            // -1 → -10
            m_alertGauge->AddGauge(-changeAmount);
        }
    }
    
}
bool EnemyEntity::UseAlertGauge() const
{
    return true;
}

void EnemyEntity::SetEnemyRank(EnemyRank rank)
{
    m_enemyRank = rank;
}

void EnemyEntity::ApplySmoke()
{
    m_isInSmoke = true;

    // 警戒ゲージを無防備状態にする
    m_alertGauge->SetGauge(-100.0f);

    // プレイヤーの追跡を解除
    // 攻撃中なら攻撃をキャンセル
}

void EnemyEntity::EndSmoke()
{
    if (!m_isInSmoke)
        return;

    m_isInSmoke = false;

    // 警戒ゲージを警戒状態にする
    m_alertGauge->SetGauge(0.0f);
}

//追加
void EnemyEntity::ApplyAttackPowerBuff(float multiplier, float duration)
{
    if (multiplier < 1.0f || duration <= 0.0f)
    {
        return;
    }

    m_attackPowerMultiplier = multiplier;
    m_attackBuffTimer = duration;
}

void EnemyEntity::Draw()
{
    // 死亡している敵は描画しない
    if (IsDead())
    {
        return;
    }

    // Sceneの取得
    Scene* scene = GetScene();

    if (scene == nullptr || scene->GetGame() == nullptr)
    {
        Actor::Draw();
        return;
    }

    // Rendererの取得
    Renderer* renderer = scene->GetGame()->GetRenderer();

    if (renderer == nullptr)
    {
        Actor::Draw();
        return;
    }

    // 敵の位置とサイズを取得
    Vector2d pos = GetPos();

#ifdef _DEBUG
    if (m_alertGauge != nullptr && m_alertGauge->GetGauge() < 0.0f)
    {
        const float rangeWidth = SEARCH_RANGE_X * 2.0f;
        const float rangeHeight = 25.0f;

        // 地面のY座標を取得
        float groundY = m_searchRangeGroundY;

        // 帯の中心位置
        Vector2d rangeCenter(
            pos.x,
            groundY + rangeHeight * 0.5f
        );

        int oldBlendMode;
        int oldBlendParam;

        GetDrawBlendMode(&oldBlendMode, &oldBlendParam);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 60);

        renderer->DrawRectCenter(
            rangeCenter,
            rangeWidth,
            rangeHeight,
            Color(255, 0, 0),
            true,
            true
        );

        SetDrawBlendMode(oldBlendMode, oldBlendParam);
    }
#endif
    Actor::Draw();
}
// 原画の左右が敵ごとに異なるため、向きが変わる場合だけ現在の反転を切り替える。
void EnemyEntity::FacePlayerOnDamage(const Vector2d& knockback)
{
    if (GetState() == Actor::State::Dead || !m_transform) return;

    // 横ノックバックがある場合は、その逆方向が攻撃の来た側。
    float sourceDirection = -knockback.x;
    if (sourceDirection == 0.0f && m_scene)
    {
        // 横方向の押し出しがない攻撃では、現在のプレイヤー位置を参照する。
        for (auto* actor : m_scene->GetActors())
        {
            if (!actor || actor->GetType() != ActorType::Player ||
                actor->GetState() == Actor::State::Dead) continue;
            const auto* player = dynamic_cast<const EntityActor*>(actor);
            if (player) sourceDirection = player->GetPos().x - GetPos().x;
            break;
        }
    }
    // 真上・真下や方向不明の場合、現在の向きを維持する。
    if (sourceDirection == 0.0f) return;
    const bool faceRight = sourceDirection > 0.0f;
    if (m_dir == faceRight) return;
    m_dir = faceRight;
    if (m_sprite) m_sprite->SetFlipX(!m_sprite->GetFlipX());
}
