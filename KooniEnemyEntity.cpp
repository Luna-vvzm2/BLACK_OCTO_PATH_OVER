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
constexpr float KOONI_BODY_WIDTH = 80.0f;
constexpr float KOONI_BODY_HEIGHT = 120.0f;
constexpr float KOONI_SIGHT_RANGE_X = 800.0f;
constexpr float KOONI_SIGHT_RANGE_Y = 120.0f;
constexpr float KOONI_MOVE_SPEED = 600.0f;

constexpr float KOONI_ATTACK_WIDTH = 60.0f;
constexpr float KOONI_ATTACK_HEIGHT = 60.0f;
constexpr int KOONI_ATTACK_DAMAGE = 15;
constexpr float KOONI_ATTACK_HIT_TIME = 10.0f / 60.0f;
constexpr float KOONI_ATTACK_MOTION_TIME = 30.0f / 60.0f;
constexpr float KOONI_ATTACK_RECOVERY_TIME = 1.0f;
constexpr float KOONI_ATTACK_KNOCKBACK = 250.0f;

constexpr float KOONI_IDLE_FRAME_TIME = 9.0f / 60.0f;
constexpr float KOONI_MOVE_FRAME_TIME = 6.0f / 60.0f;
constexpr float KOONI_ATTACK_FRAME_TIME = 10.0f / 60.0f;
constexpr float KOONI_DEAD_FRAME_TIME = 14.0f / 60.0f;
constexpr float KOONI_DEAD_MOTION_TIME = KOONI_DEAD_FRAME_TIME * 7.0f;

const char* KOONI_TEXTURE_IDLE = "assets/images/Enemy/kooni/idle.png";
const char* KOONI_TEXTURE_MOVE = "assets/images/Enemy/kooni/move.png";
const char* KOONI_TEXTURE_ATTACK = "assets/images/Enemy/kooni/attack.png";
const char* KOONI_TEXTURE_DEAD = "assets/images/Enemy/kooni/dead.png";
}

KooniEnemyEntity::KooniEnemyEntity(Scene* scene, const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(KOONI_BODY_WIDTH, KOONI_BODY_HEIGHT))
    , m_status(Status::Idle)
    , m_awareness(Awareness::Undetected)
    , m_stateTimer(0.0f)
    , m_faceRight(true)
    , m_attackHit(false)
    , m_attackRecoveryStarted(false)
    , m_hitSound(nullptr)
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
    m_hitSound = AddComponent<SoundComponent>(
        _T("assets/sounds/player/weakAttack1.wav")
    );

    AnimationClip idle;
    idle.frames = { 0, 1, 2 };
    idle.frameDurations = { KOONI_IDLE_FRAME_TIME, KOONI_IDLE_FRAME_TIME, KOONI_IDLE_FRAME_TIME };
    idle.loop = true;
    m_anim->AddClip("idle", idle);

    AnimationClip move;
    move.frames = { 0, 1, 2 };
    move.frameDurations = { KOONI_MOVE_FRAME_TIME, KOONI_MOVE_FRAME_TIME, KOONI_MOVE_FRAME_TIME };
    move.loop = true;
    m_anim->AddClip("move", move);

    AnimationClip attack;
    attack.frames = { 0, 1, 2 };
    attack.frameDurations = { KOONI_ATTACK_FRAME_TIME, KOONI_ATTACK_FRAME_TIME, KOONI_ATTACK_FRAME_TIME };
    attack.loop = false;
    m_anim->AddClip("attack", attack);

    AnimationClip dead;
    dead.frames = { 0, 1, 2, 3, 4, 5, 6 };
    dead.frameDurations = {
        KOONI_DEAD_FRAME_TIME,
        KOONI_DEAD_FRAME_TIME,
        KOONI_DEAD_FRAME_TIME,
        KOONI_DEAD_FRAME_TIME,
        KOONI_DEAD_FRAME_TIME,
        KOONI_DEAD_FRAME_TIME,
        KOONI_DEAD_FRAME_TIME
    };
    dead.loop = false;
    m_anim->AddClip("dead", dead);

    PlayMotion("idle", KOONI_TEXTURE_IDLE, 3);
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
        m_sprite->SetFlipH(!m_faceRight);
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
        PlayMotion("idle", KOONI_TEXTURE_IDLE, 3);
        break;
    case Status::Move:
        PlayMotion("move", KOONI_TEXTURE_MOVE, 3);
        break;
    case Status::Attack:
        m_attackHit = false;
        m_attackRecoveryStarted = false;
        PlayMotion("attack", KOONI_TEXTURE_ATTACK, 3);
        break;
    case Status::Defeated:
        PlayMotion("dead", KOONI_TEXTURE_DEAD, 7);
        break;
    }
}

void KooniEnemyEntity::PlayMotion(
    const std::string& name,
    const char* texturePath,
    int frameCount)
{
    if (m_sprite == nullptr || m_anim == nullptr)
    {
        return;
    }

    if (m_currentTexturePath != texturePath)
    {
        if (!m_sprite->LoadTextureDiv(texturePath, frameCount, 1))
        {
            return;
        }
        m_currentTexturePath = texturePath;
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
        PlayMotion("idle", KOONI_TEXTURE_IDLE, 3);
    }

    if (m_stateTimer >= KOONI_ATTACK_MOTION_TIME + KOONI_ATTACK_RECOVERY_TIME)
    {
        ChangeStatus(Status::Idle);
    }
}

void KooniEnemyEntity::UpdateDead()
{
    if (m_stateTimer >= KOONI_DEAD_MOTION_TIME)
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
            (KOONI_BODY_WIDTH * 0.5f + KOONI_ATTACK_WIDTH * 0.5f),
        GetPos().y
    );

    const float combinedHalfWidth =
        KOONI_ATTACK_WIDTH * 0.5f + player->GetCollision()->GetWidth() * 0.5f;
    const float combinedHalfHeight =
        KOONI_ATTACK_HEIGHT * 0.5f + player->GetCollision()->GetHeight() * 0.5f;

    if (std::fabs(playerPos.x - attackCenter.x) <= combinedHalfWidth &&
        std::fabs(playerPos.y - attackCenter.y) <= combinedHalfHeight)
    {
        const float knockbackX = m_faceRight ? KOONI_ATTACK_KNOCKBACK : -KOONI_ATTACK_KNOCKBACK;
        player->TakeDamage(KOONI_ATTACK_DAMAGE, Vector2d(knockbackX, 0.0f));
        if (m_hitSound != nullptr)
        {
            m_hitSound->Play();
        }
    }
}

void KooniEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
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
    return KOONI_TEXTURE_IDLE;
}

