#include "KasaTenguEnemyEntity.h"

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

//仮表示
#define KASATENGU_USE_PLACEHOLDER_ART

namespace
{
    constexpr float KASATENGU_BODY_WIDTH = 120.0f;
    constexpr float KASATENGU_BODY_HEIGHT = 180.0f;
    constexpr float KASATENGU_SIGHT_RANGE_X = 800.0f;   // 仮
    constexpr float KASATENGU_SIGHT_RANGE_Y = 180.0f;   // 仮
    constexpr float KASATENGU_MOVE_SPEED = 600.0f;

    // 2連攻撃(1段目 / 2段目)
    constexpr float KASATENGU_ATTACK1_WIDTH = 50.0f;
    constexpr float KASATENGU_ATTACK1_HEIGHT = 60.0f;
    constexpr float KASATENGU_ATTACK2_WIDTH = 60.0f;
    constexpr float KASATENGU_ATTACK2_HEIGHT = 50.0f;
    constexpr int KASATENGU_ATTACK1_DAMAGE = 10;
    constexpr int KASATENGU_ATTACK2_DAMAGE = 15;
    constexpr float KASATENGU_ATTACK1_HIT_TIME = 30.0f / 60.0f;   // 仮
    constexpr float KASATENGU_ATTACK2_HIT_TIME = 80.0f / 60.0f;   // 仮
    constexpr float KASATENGU_ATTACK_MOTION_TIME = 120.0f / 60.0f;
    constexpr float KASATENGU_ATTACK_RECOVERY_TIME = 1.2f;
    constexpr float KASATENGU_ATTACK_KNOCKBACK = 250.0f;          // 仮

    constexpr float KASATENGU_IDLE_FRAME_TIME = 9.0f / 60.0f;
    constexpr float KASATENGU_MOVE_FRAME_TIME = 6.0f / 60.0f;
    constexpr float KASATENGU_DEAD_FRAME_TIME = 14.0f / 60.0f;
    constexpr float KASATENGU_DEAD_MOTION_TIME = KASATENGU_DEAD_FRAME_TIME * 7.0f;

#ifdef KASATENGU_USE_PLACEHOLDER_ART
    constexpr int KASATENGU_ATTACK_SPRITE_COUNT = 3;    // 仮表示
    const char* KASATENGU_TEXTURE_IDLE = "assets/images/Enemy/kooni/idle.png";
    const char* KASATENGU_TEXTURE_MOVE = "assets/images/Enemy/kooni/move.png";
    const char* KASATENGU_TEXTURE_ATTACK = "assets/images/Enemy/kooni/attack.png";
    const char* KASATENGU_TEXTURE_DEAD = "assets/images/Enemy/kooni/dead.png";
#else
    constexpr int KASATENGU_ATTACK_SPRITE_COUNT = 30;
    const char* KASATENGU_TEXTURE_IDLE = "assets/images/Enemy/kasatengu/idle.png";
    const char* KASATENGU_TEXTURE_MOVE = "assets/images/Enemy/kasatengu/move.png";
    const char* KASATENGU_TEXTURE_ATTACK = "assets/images/Enemy/kasatengu/attack.png";
    const char* KASATENGU_TEXTURE_DEAD = "assets/images/Enemy/kasatengu/dead.png";
#endif

    constexpr float KASATENGU_ATTACK_FRAME_TIME =
        KASATENGU_ATTACK_MOTION_TIME / static_cast<float>(KASATENGU_ATTACK_SPRITE_COUNT);

    AnimationClip MakeClip(int spriteCount, float frameTime, bool loop)
    {
        AnimationClip clip;
        for (int i = 0; i < spriteCount; ++i)
        {
            clip.frames.push_back(i);
            clip.frameDurations.push_back(frameTime);
        }
        clip.loop = loop;
        return clip;
    }
}

KasaTenguEnemyEntity::KasaTenguEnemyEntity(Scene* scene, const Vector2d& pos)
    : EnemyEntity(scene, pos, Vector2d(KASATENGU_BODY_WIDTH, KASATENGU_BODY_HEIGHT))
    , m_status(Status::Idle)
    , m_awareness(Awareness::Undetected)
    , m_stateTimer(0.0f)
    , m_faceRight(true)
    , m_attackHit1(false)
    , m_attackHit2(false)
    , m_attackRecoveryStarted(false)
    , m_hitSound(nullptr)
{
}

bool KasaTenguEnemyEntity::Init()
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
        _T("assets/sounds/player/weakAttack1.wav")   // 仮
    );

    m_anim->AddClip("idle", MakeClip(3, KASATENGU_IDLE_FRAME_TIME, true));
    m_anim->AddClip("move", MakeClip(3, KASATENGU_MOVE_FRAME_TIME, true));
    m_anim->AddClip("attack", MakeClip(KASATENGU_ATTACK_SPRITE_COUNT, KASATENGU_ATTACK_FRAME_TIME, false));
    m_anim->AddClip("dead", MakeClip(7, KASATENGU_DEAD_FRAME_TIME, false));

    PlayMotion("idle", KASATENGU_TEXTURE_IDLE, 3);
    return true;
}

void KasaTenguEnemyEntity::Update(float deltaTime)
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

PlayerEntity* KasaTenguEnemyEntity::FindPlayer() const
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

bool KasaTenguEnemyEntity::CanSeePlayer(const PlayerEntity* player) const
{
    if (player == nullptr)
    {
        return false;
    }

    const Vector2d difference = player->GetPos() - GetPos();
    return std::fabs(difference.x) <= KASATENGU_SIGHT_RANGE_X &&
        std::fabs(difference.y) <= KASATENGU_SIGHT_RANGE_Y;
}

void KasaTenguEnemyEntity::ChangeStatus(Status nextStatus)
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
        PlayMotion("idle", KASATENGU_TEXTURE_IDLE, 3);
        break;
    case Status::Move:
        PlayMotion("move", KASATENGU_TEXTURE_MOVE, 3);
        break;
    case Status::Attack:
        m_attackHit1 = false;
        m_attackHit2 = false;
        m_attackRecoveryStarted = false;
        PlayMotion("attack", KASATENGU_TEXTURE_ATTACK, KASATENGU_ATTACK_SPRITE_COUNT);
        break;
    case Status::Defeated:
        PlayMotion("dead", KASATENGU_TEXTURE_DEAD, 7);
        break;
    }
}

void KasaTenguEnemyEntity::PlayMotion(
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

void KasaTenguEnemyEntity::UpdateIdle(PlayerEntity* player)
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
    const float attackDistance = KASATENGU_BODY_WIDTH * 0.5f + KASATENGU_ATTACK2_WIDTH;
    if (std::fabs(difference.x) <= attackDistance &&
        std::fabs(difference.y) <= KASATENGU_ATTACK1_HEIGHT)
    {
        m_faceRight = difference.x >= 0.0f;
        ChangeStatus(Status::Attack);
    }
    else
    {
        ChangeStatus(Status::Move);
    }
}

void KasaTenguEnemyEntity::UpdateMove(PlayerEntity* player)
{
    if (!CanSeePlayer(player))
    {
        m_awareness = Awareness::Undetected;
        ChangeStatus(Status::Idle);
        return;
    }

    const Vector2d difference = player->GetPos() - GetPos();
    const float attackDistance = KASATENGU_BODY_WIDTH * 0.5f + KASATENGU_ATTACK2_WIDTH;
    if (std::fabs(difference.x) <= attackDistance &&
        std::fabs(difference.y) <= KASATENGU_ATTACK1_HEIGHT)
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
        velocity.x = m_faceRight ? KASATENGU_MOVE_SPEED : -KASATENGU_MOVE_SPEED;
        m_velocity->SetVelocity(velocity);
    }
}

void KasaTenguEnemyEntity::UpdateAttack(float deltaTime, PlayerEntity* player)
{
    (void)deltaTime;

    if (m_velocity != nullptr)
    {
        Vector2d velocity = m_velocity->GetVelocity();
        velocity.x = 0.0f;
        m_velocity->SetVelocity(velocity);
    }

    if (!m_attackHit1 && m_stateTimer >= KASATENGU_ATTACK1_HIT_TIME)
    {
        TryHitPlayer(player, 1);
        m_attackHit1 = true;
    }

    if (!m_attackHit2 && m_stateTimer >= KASATENGU_ATTACK2_HIT_TIME)
    {
        TryHitPlayer(player, 2);
        m_attackHit2 = true;
    }

    if (!m_attackRecoveryStarted && m_stateTimer >= KASATENGU_ATTACK_MOTION_TIME)
    {
        m_attackRecoveryStarted = true;
        PlayMotion("idle", KASATENGU_TEXTURE_IDLE, 3);
    }

    if (m_stateTimer >= KASATENGU_ATTACK_MOTION_TIME + KASATENGU_ATTACK_RECOVERY_TIME)
    {
        ChangeStatus(Status::Idle);
    }
}

void KasaTenguEnemyEntity::UpdateDead()
{
    if (m_stateTimer >= KASATENGU_DEAD_MOTION_TIME)
    {
        OnDead();
    }
}

void KasaTenguEnemyEntity::TryHitPlayer(PlayerEntity* player, int stage)
{
    if (player == nullptr || player->GetCollision() == nullptr)
    {
        return;
    }

    const float attackWidth = (stage == 1) ? KASATENGU_ATTACK1_WIDTH : KASATENGU_ATTACK2_WIDTH;
    const float attackHeight = (stage == 1) ? KASATENGU_ATTACK1_HEIGHT : KASATENGU_ATTACK2_HEIGHT;
    const int attackDamage = (stage == 1) ? KASATENGU_ATTACK1_DAMAGE : KASATENGU_ATTACK2_DAMAGE;

    const Vector2d playerPos = player->GetPos();
    const Vector2d attackCenter(
        GetPos().x + (m_faceRight ? 1.0f : -1.0f) *
        (KASATENGU_BODY_WIDTH * 0.5f + attackWidth * 0.5f),
        GetPos().y
    );

    const float combinedHalfWidth =
        attackWidth * 0.5f + player->GetCollision()->GetWidth() * 0.5f;
    const float combinedHalfHeight =
        attackHeight * 0.5f + player->GetCollision()->GetHeight() * 0.5f;

    if (std::fabs(playerPos.x - attackCenter.x) <= combinedHalfWidth &&
        std::fabs(playerPos.y - attackCenter.y) <= combinedHalfHeight)
    {
        const float knockbackX = m_faceRight ? KASATENGU_ATTACK_KNOCKBACK : -KASATENGU_ATTACK_KNOCKBACK;
        player->TakeDamage(attackDamage, Vector2d(knockbackX, 0.0f));
        if (m_hitSound != nullptr)
        {
            m_hitSound->Play();
        }
    }
}

void KasaTenguEnemyEntity::TakeDamage(int damage, const Vector2d& knockback)
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

std::string KasaTenguEnemyEntity::GetTexturePath() const
{
    return KASATENGU_TEXTURE_IDLE;
}