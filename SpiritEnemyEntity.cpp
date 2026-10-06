#include "SpiritEnemyEntity.h"
#include "EnemyBullet.h"
#include "VelocityComponent.h"
#include "HPComponent.h"
#include "CollisionComponent.h"
#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "PlayScene.h"
#include "PlayerEntity.h"
#include "SpiritBulletEntity.h"
#include "AlertGaugeComponent.h"

#include <cmath>

SpiritEnemyEntity::SpiritEnemyEntity(
	Scene*scene,
	const Vector2d&pos
)
    : EnemyEntity(scene, pos, Vector2d(80.0f, 80.0f))
    , m_state(Idle)
    , m_animation(nullptr)
    , m_floatTimer(0.0f)
    , m_attackTimer(0.0f)
    , m_deadTimer(0.0f)
    , m_detectRange(700.0f)
    , m_keepDistance(500.0f)
    , m_moveSpeed(500.0f)
    , m_faceRight(true)
    , m_isDying(false)
    , m_targetPosition(Vector2d::Zero())
    , m_hasTargetPosition(false)
{
}

bool SpiritEnemyEntity::Init() 
{
    if (!EnemyEntity::Init())
    {
        return false;
    }

    // 怨霊のHP
    m_hp = AddComponent<HPComponent>(80);

    // 怨霊本体の当たり判定
    // 仕様：80px × 80px
    m_collision->SetRect(80.0f, 80.0f);

    // 初期状態
    m_state = Idle;

    m_floatTimer = 0.0f;
    m_attackTimer = 0.0f;
    m_deadTimer = 0.0f;

    m_faceRight = true;
    m_isDying = false;
    m_targetPosition = Vector2d::Zero();
    m_hasTargetPosition = false;

    return true;
}


void SpiritEnemyEntity::Update(float deltaTime)
{
    if (GetState() == Actor::State::Dead)
    {
        return;
    }

    // 浮遊用タイマー
    m_floatTimer += deltaTime;

    // 攻撃中のタイマー
    if (m_state == Attack)
    {
        m_attackTimer += deltaTime;

        // 攻撃モーション終了
        if (m_attackTimer >= 0.96f)
        {
            m_attackTimer = 0.0f;
            m_state = Recovery;
        }
    }
    // 攻撃後の硬直
    else if (m_state == Recovery)
    {
        m_attackTimer += deltaTime;

        if (m_attackTimer >= 1.2f)
        {
            m_attackTimer = 0.0f;
            m_state = Move;
        }
    }

    // AIを更新
    UpdateAI();


    // 警戒ゲージを更新
    UpdateAlertGauge(deltaTime);

    // 怨霊は重力を使用しない
    Vector2d pos = GetPos();
    Vector2d vel = GetVel();

    pos += vel * deltaTime;

    SetPos(pos);

    // EnemyEntity::Update() は呼ばない
    // EntityActor::Update() もここでは呼ばない
}

void SpiritEnemyEntity::UpdateAI()
{
    PlayScene* playScene = dynamic_cast<PlayScene*>(m_scene);

    if (playScene == nullptr)
    {
        return;
    }

    PlayerEntity* player = playScene->GetPlayer();

    if (player == nullptr || player->IsDead())
    {
        SetVel(Vector2d::Zero());
        return;
    }

    Vector2d myPos = GetPos();
    Vector2d playerPos = player->GetPos();

    float distanceX = playerPos.x - myPos.x;
    float absDistanceX = std::abs(distanceX);

    switch (m_state)
    {
    case Idle:
    {
        // 上下にふわふわ浮遊
        float floatSpeed = std::sin(m_floatTimer) * 10.0f;

        SetVel(Vector2d(0.0f, floatSpeed));

        // プレイヤーを検知したらMoveへ
        if (absDistanceX <= m_detectRange)
        {
            m_state = Move;
        }
    }
    break;

    case Move:
    {
        // 警戒中はその場で浮遊
        SetVel(Vector2d(
            0.0f,
            std::sin(m_floatTimer) * 10.0f
        ));

        // 警戒ゲージが100になったら戦闘開始
        if (m_alertGauge != nullptr &&
            m_alertGauge->IsCombat())
        {
            m_state = CombatMove;
            m_attackTimer = 0.0f;
            m_hasTargetPosition = false;
        }
    }
    break;

    case CombatMove:
    {
        float distanceX = playerPos.x - myPos.x;
        float absDistanceX = std::abs(distanceX);

        const float keepDistance = 500.0f;
        const float arrivalDistance = 5.0f;

        // まだ目標地点を決めていない
        if (!m_hasTargetPosition)
        {
            // 近すぎる場合
            if (absDistanceX < keepDistance)
            {
                float direction =
                    (distanceX > 0.0f) ? -1.0f : 1.0f;

                // 現在のプレイヤー位置から500px離れた場所を目標地点にする
                m_targetPosition = Vector2d(
                    playerPos.x + direction * keepDistance,
                    myPos.y
                );

                m_hasTargetPosition = true;
            }
            // 遠すぎる場合
            else if (absDistanceX > keepDistance)
            {
                float direction =
                    (distanceX > 0.0f) ? 1.0f : -1.0f;

                m_targetPosition = Vector2d(
                    playerPos.x + direction * keepDistance,
                    myPos.y
                );

                m_hasTargetPosition = true;
            }
            else
            {
                // すでに約500pxなら移動不要
                SetVel(Vector2d(
                    0.0f,
                    std::sin(m_floatTimer) * 10.0f
                ));

                m_state = Attack;
                m_attackTimer = 0.0f;
            }
        }

        // 目標地点が決まっている
        if (m_hasTargetPosition)
        {
            float targetDistance =
                m_targetPosition.x - myPos.x;

            float absTargetDistance =
                std::abs(targetDistance);

            // 目標地点に到着
            if (absTargetDistance <= arrivalDistance)
            {
                SetVel(Vector2d(
                    0.0f,
                    std::sin(m_floatTimer) * 10.0f
                ));

                m_hasTargetPosition = false;

                m_state = Attack;
                m_attackTimer = 0.0f;
            }
            else
            {
                float direction =
                    (targetDistance > 0.0f) ? 1.0f : -1.0f;

                SetVel(Vector2d(
                    direction * m_moveSpeed,
                    std::sin(m_floatTimer) * 10.0f
                ));

                m_faceRight = (direction > 0.0f);
            }
        }
    }
    break;

    case Attack:
    {
        // 攻撃中はその場で浮遊
        float floatSpeed = std::sin(m_floatTimer) * 10.0f;

        SetVel(Vector2d(0.0f, floatSpeed));

        // 攻撃開始から一定時間で弾を発射
        if (m_attackTimer >= 0.5f)
        {
            // 怨霊からプレイヤーへ向かう方向
            Vector2d direction = playerPos - myPos;

            // ベクトルの長さ
            float length = std::sqrt(
                direction.x * direction.x +
                direction.y * direction.y
            );

            // 方向ベクトルを正規化
            if (length > 0.0f)
            {
                direction.x /= length;
                direction.y /= length;
            }

            // 怨霊の向きを更新
            m_faceRight = (direction.x > 0.0f);

            // 弾の出現位置
            Vector2d bulletPos = GetPos();

            bulletPos.x += direction.x * 40.0f;
            bulletPos.y += direction.y * 40.0f;

            // 弾速700px/sでプレイヤー方向へ飛ばす
            Vector2d bulletVelocity(
                direction.x * 700.0f,
                direction.y * 700.0f
            );

            SpiritBulletEntity* bullet = new SpiritBulletEntity(
                m_scene,
                bulletPos,
                bulletVelocity,
                1500.0f,
                GetTexturePath()
            );

            m_scene->AddActor(bullet);

            m_attackTimer = 0.0f;
            m_state = Recovery;
        }
    }
    break;

    case Recovery:
    {
        SetVel(Vector2d(
            0.0f,
            std::sin(m_floatTimer) * 10.0f
        ));
    }
    break;

    case Dead:
    {
        SetVel(Vector2d::Zero());
    }
    break;

    default:
        SetVel(Vector2d::Zero());
        break;
    }
}

bool SpiritEnemyEntity::IsPlayerInSearchRange() const
{
    PlayScene* playScene =
        dynamic_cast<PlayScene*>(m_scene);

    if (playScene == nullptr)
    {
        return false;
    }

    PlayerEntity* player = playScene->GetPlayer();

    if (player == nullptr || player->IsDead())
    {
        return false;
    }

    Vector2d enemyPos = GetPos();
    Vector2d playerPos = player->GetPos();

    float distanceX =
        std::abs(playerPos.x - enemyPos.x);

    float distanceY =
        std::abs(playerPos.y - enemyPos.y);

    return distanceX <= m_detectRange &&
        distanceY <= 250.0f;
}


void SpiritEnemyEntity::Draw()
{
    EnemyEntity::Draw();

    if (m_collision != nullptr)
    {
        m_collision->DrawDebug();
    }
}

std::string SpiritEnemyEntity::GetTexturePath() const
{
    return "assets/images/Enemy/spirit.png";
}