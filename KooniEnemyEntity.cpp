#include "KooniEnemyEntity.h"

#include "Actor.h"
#include "AnimationComponent.h"
#include "CollisionComponent.h"
#include "HPComponent.h"
#include "PlayerEntity.h"
#include "Scene.h"
#include "SoundComponent.h"
#include "SpriteComponent.h"
#include "VelocityComponent.h"

#include <cmath>

namespace
{
constexpr float KOONI_BODY_WIDTH = 104.0f;
constexpr float KOONI_BODY_HEIGHT = 156.0f;
constexpr float KOONI_SIGHT_RANGE_X = 800.0f;
constexpr float KOONI_SIGHT_RANGE_Y = 120.0f;
constexpr float KOONI_MOVE_SPEED = 294.0f;

constexpr float KOONI_ATTACK_WIDTH = 60.0f;
constexpr float KOONI_ATTACK_HEIGHT = 60.0f;
constexpr float KOONI_ATTACK_INNER_EXTENSION = 12.0f;
constexpr int KOONI_ATTACK_DAMAGE = 15;
constexpr float KOONI_ATTACK_HIT_TIME = 40.0f / 60.0f;
constexpr float KOONI_ATTACK_MOTION_TIME = 80.0f / 60.0f;
constexpr float KOONI_ATTACK_RECOVERY_TIME = 1.0f;
constexpr float KOONI_ATTACK_KNOCKBACK = 250.0f;

constexpr float KOONI_IDLE_FRAME_TIME = 9.0f / 60.0f;
constexpr float KOONI_MOVE_FRAME_TIME = 6.0f / 60.0f;
constexpr float KOONI_ATTACK_FRAME_TIME = 10.0f / 60.0f;
constexpr float KOONI_DEAD_FADE_TIME = 1.0f;
constexpr float KOONI_SPRITE_DRAW_HEIGHT = 176.8f;
constexpr float KOONI_SPRITE_DRAW_OFFSET_X = 79.3f;
constexpr float KOONI_SPRITE_DRAW_OFFSET_Y = -9.0f;

const char* KOONI_TEXTURE_SHEET = "assets/images/Enemy/kooni/kooni_sheet.png";
}

KooniEnemyEntity::KooniEnemyEntity(Scene* scene, const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(KOONI_BODY_WIDTH, KOONI_BODY_HEIGHT))
    , m_status(Status::Idle)
    , m_awareness(Awareness::Undetected)
    , m_stateTimer(0.0f)
    , m_faceRight(true)
    , m_attackHit(false)
    , m_attackRecoveryStarted(false)
    , m_attackSound(nullptr)
{
}

bool KooniEnemyEntity::Init()
{
    if (!EnemyEntity::Init())
    {
        return false;
    }

    m_anim = AddComponent<AnimationComponent>();
    if (m_anim == nullptr || m_sprite == nullptr)
    {
        return false;
    }

    m_anim->SetSprite(m_sprite);
    m_attackSound = AddComponent<SoundComponent>();
    if (!m_sprite->LoadTextureDiv(KOONI_TEXTURE_SHEET, 4, 4))
    {
        return false;
    }
    m_sprite->SetDrawSize(0.0f, KOONI_SPRITE_DRAW_HEIGHT);
    m_sprite->SetFlipX(m_faceRight);
    m_sprite->SetDrawOffset(KOONI_SPRITE_DRAW_OFFSET_X, KOONI_SPRITE_DRAW_OFFSET_Y);

    AnimationClip idle;
    idle.frames = { 0, 1, 2, 3 };
    idle.frameDurations = { KOONI_IDLE_FRAME_TIME, KOONI_IDLE_FRAME_TIME,
        KOONI_IDLE_FRAME_TIME, KOONI_IDLE_FRAME_TIME };
    idle.loop = true;
    m_anim->AddClip("idle", idle);

    AnimationClip move;
    move.frames = { 4, 5, 6, 7 };
    move.frameDurations = { KOONI_MOVE_FRAME_TIME, KOONI_MOVE_FRAME_TIME,
        KOONI_MOVE_FRAME_TIME, KOONI_MOVE_FRAME_TIME };
    move.loop = true;
    m_anim->AddClip("move", move);

    AnimationClip attack;
    attack.frames = { 8, 9, 10, 11, 12, 13, 14, 15 };
    attack.frameDurations = { KOONI_ATTACK_FRAME_TIME, KOONI_ATTACK_FRAME_TIME,
        KOONI_ATTACK_FRAME_TIME, KOONI_ATTACK_FRAME_TIME,
        KOONI_ATTACK_FRAME_TIME, KOONI_ATTACK_FRAME_TIME,
        KOONI_ATTACK_FRAME_TIME, KOONI_ATTACK_FRAME_TIME };
    attack.loop = false;
    m_anim->AddClip("attack", attack);

    AnimationClip defeated;
    defeated.frames = { 0 };
    defeated.loop = false;
    m_anim->AddClip("defeated", defeated);

    PlayMotion("idle");
    return true;
}

void KooniEnemyEntity::Update(float deltaTime)
{
    if (m_status == Status::Defeated)
    {
        m_stateTimer += deltaTime;

        if (m_velocity != nullptr)
        {
            Vector2d velocity = m_velocity->GetVelocity();
            velocity.x = 0.0f;
            m_velocity->SetVelocity(velocity);
        }

        EnemyEntity::Update(deltaTime);
        UpdateDead();
        return;
    }

    PlayerEntity* player = FindPlayer();
    m_stateTimer += deltaTime;

    switch (m_status)
    {
    case Status::Idle:
        UpdateIdle(player);
        break;
    case Status::Move:
        UpdateMove(player);
        break;
    case Status::Attack:
        UpdateAttack(deltaTime, player);
        break;
    case Status::Defeated:
        break;
    }

    if (m_sprite != nullptr)
    {
        m_sprite->SetFlipX(m_faceRight);
        m_sprite->SetDrawOffset(
            m_faceRight ? KOONI_SPRITE_DRAW_OFFSET_X : -KOONI_SPRITE_DRAW_OFFSET_X,
            KOONI_SPRITE_DRAW_OFFSET_Y);
    }

    EnemyEntity::Update(deltaTime);
}

PlayerEntity* KooniEnemyEntity::FindPlayer() const
{
    if (m_scene == nullptr)
    {
        return nullptr;
    }

    for (Actor* actor : m_scene->GetActors())
    {
        if (actor != nullptr && actor->GetType() == ActorType::Player && !actor->IsDead())
        {
            return static_cast<PlayerEntity*>(actor);
        }
    }

    return nullptr;
}

bool KooniEnemyEntity::CanSeePlayer(const PlayerEntity* player) const
{
    if (player == nullptr)
    {
        return false;
    }

    if (player->GetIsHide()) { return false; }

    const Vector2d difference = player->GetPos() - GetPos();
    return std::fabs(difference.x) <= KOONI_SIGHT_RANGE_X &&
        std::fabs(difference.y) <= KOONI_SIGHT_RANGE_Y;
}

void KooniEnemyEntity::ChangeStatus(Status nextStatus)
{
    if (m_status == nextStatus)
    {
        return;
    }

    m_status = nextStatus;
    m_stateTimer = 0.0f;

    switch (m_status)
    {
    case Status::Idle:
        PlayMotion("idle");
        break;
    case Status::Move:
        PlayMotion("move");
        break;
    case Status::Attack:
        m_attackHit = false;
        m_attackRecoveryStarted = false;
        PlayMotion("attack");
        if (m_attackSound != nullptr)
        {
            m_attackSound->Play("enemy_Kooni_attack");
        }
        break;
    case Status::Defeated:
        PlayMotion("defeated");
        if (m_sprite != nullptr)
        {
            m_sprite->SetAlpha(255);
        }
        break;
    }
}

void KooniEnemyEntity::PlayMotion(const std::string& name)
{
    if (m_sprite == nullptr || m_anim == nullptr)
    {
        return;
    }

    m_anim->Play(name, true);
}

void KooniEnemyEntity::UpdateIdle(PlayerEntity* player)
{
    if (m_velocity != nullptr)
    {
        Vector2d velocity = m_velocity->GetVelocity();
        velocity.x = 0.0f;
        m_velocity->SetVelocity(velocity);
    }

    if (!CanSeePlayer(player))
    {
        m_awareness = Awareness::Undetected;
        return;
    }

    if (m_awareness == Awareness::Undetected)
    {
        m_awareness = Awareness::Detected;
        m_stateTimer = 0.0f;
        return;
    }

    const Vector2d difference = player->GetPos() - GetPos();
    const float attackDistance = KOONI_BODY_WIDTH * 0.5f + KOONI_ATTACK_WIDTH;
    if (std::fabs(difference.x) <= attackDistance &&
        std::fabs(difference.y) <= KOONI_ATTACK_HEIGHT)
    {
        m_faceRight = difference.x >= 0.0f;
        ChangeStatus(Status::Attack);
    }
    else
    {
        ChangeStatus(Status::Move);
    }
}

void KooniEnemyEntity::UpdateMove(PlayerEntity* player)
{
    if (!CanSeePlayer(player))
    {
        m_awareness = Awareness::Undetected;
        ChangeStatus(Status::Idle);
        return;
    }

    const Vector2d difference = player->GetPos() - GetPos();
    const float attackDistance = KOONI_BODY_WIDTH * 0.5f + KOONI_ATTACK_WIDTH;
    if (std::fabs(difference.x) <= attackDistance &&
        std::fabs(difference.y) <= KOONI_ATTACK_HEIGHT)
    {
        if (m_velocity != nullptr)
        {
            Vector2d velocity = m_velocity->GetVelocity();
            velocity.x = 0.0f;
            m_velocity->SetVelocity(velocity);
        }
        m_faceRight = difference.x >= 0.0f;
        ChangeStatus(Status::Attack);
        return;
    }

    m_faceRight = difference.x >= 0.0f;
    if (m_velocity != nullptr)
    {
        Vector2d velocity = m_velocity->GetVelocity();
        velocity.x = m_faceRight ? KOONI_MOVE_SPEED : -KOONI_MOVE_SPEED;
        m_velocity->SetVelocity(velocity);
    }
}

void KooniEnemyEntity::UpdateAttack(float deltaTime, PlayerEntity* player)
{
    (void)deltaTime;

    if (m_velocity != nullptr)
    {
        Vector2d velocity = m_velocity->GetVelocity();
        velocity.x = 0.0f;
        m_velocity->SetVelocity(velocity);
    }

    if (!m_attackHit && m_stateTimer >= KOONI_ATTACK_HIT_TIME)
    {
        TryHitPlayer(player);
        m_attackHit = true;
    }

    if (!m_attackRecoveryStarted && m_stateTimer >= KOONI_ATTACK_MOTION_TIME)
    {
        m_attackRecoveryStarted = true;
        PlayMotion("idle");
    }

    if (m_stateTimer >= KOONI_ATTACK_MOTION_TIME + KOONI_ATTACK_RECOVERY_TIME)
    {
        ChangeStatus(Status::Idle);
    }
}

void KooniEnemyEntity::UpdateDead()
{
    if (m_sprite != nullptr)
    {
        const float remaining = 1.0f - m_stateTimer / KOONI_DEAD_FADE_TIME;
        m_sprite->SetAlpha(remaining > 0.0f ? static_cast<int>(255.0f * remaining) : 0);
    }

    if (m_stateTimer >= KOONI_DEAD_FADE_TIME)
    {
        OnDead();
    }
}

void KooniEnemyEntity::TryHitPlayer(PlayerEntity* player)
{
    if (player == nullptr || player->GetCollision() == nullptr)
    {
        return;
    }

    const Vector2d playerPos = player->GetPos();
    const Vector2d attackCenter(
        GetPos().x + (m_faceRight ? 1.0f : -1.0f) *
            (KOONI_BODY_WIDTH * 0.5f +
                (KOONI_ATTACK_WIDTH - KOONI_ATTACK_INNER_EXTENSION) * 0.5f),
        GetPos().y
    );

    const float combinedHalfWidth =
        (KOONI_ATTACK_WIDTH + KOONI_ATTACK_INNER_EXTENSION) * 0.5f +
        player->GetCollision()->GetWidth() * 0.5f;
    const float combinedHalfHeight =
        KOONI_ATTACK_HEIGHT * 0.5f + player->GetCollision()->GetHeight() * 0.5f;

    if (std::fabs(playerPos.x - attackCenter.x) <= combinedHalfWidth &&
        std::fabs(playerPos.y - attackCenter.y) <= combinedHalfHeight)
    {
        const float knockbackX = m_faceRight ? KOONI_ATTACK_KNOCKBACK : -KOONI_ATTACK_KNOCKBACK;
        player->TakeDamage(KOONI_ATTACK_DAMAGE, Vector2d(knockbackX, 0.0f));
    }
}

void KooniEnemyEntity::TakeDamage(int damage, const Vector2d& knockback, bool attackDir)
{
    if (m_hp == nullptr || m_status == Status::Defeated)
    {
        return;
    }

    m_hp->Damage(damage);
    if (m_hp->GetHP() <= 0)
    {
        if (m_velocity != nullptr)
        {
            Vector2d velocity = m_velocity->GetVelocity();
            velocity.x = 0.0f;
            m_velocity->SetVelocity(velocity);
        }
        ChangeStatus(Status::Defeated);
        return;
    }

    if (m_velocity != nullptr)
    {
        m_velocity->SetVelocity(knockback);
    }
}

std::string KooniEnemyEntity::GetTexturePath() const
{
    return "";
}

