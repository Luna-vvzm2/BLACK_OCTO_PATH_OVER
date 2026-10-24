#include "boss_02.h"
#include "AnimationComponent.h"
#include "CollisionComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "PlayerEntity.h"
#include "PlayScene.h"
#include "Scene.h"
#include "SpriteComponent.h"
#include "VelocityComponent.h"
#include <algorithm>
#include <cmath>

Boss_02::Boss_02(Scene* scene, const Vector2d& pos)
    : Boss_02(scene, pos, Parameters{}) {}

Boss_02::Boss_02(Scene* scene, const Vector2d& pos, const Parameters& parameters)
    : BossEntity(scene, pos, parameters.hitboxSize), m_parameters(parameters) {}

bool Boss_02::Init()
{
    if (m_initialized) return true;
    if (!m_scene || m_parameters.maxHP <= 0 ||
        !std::isfinite(m_initialPos.x) || !std::isfinite(m_initialPos.y) ||
        !std::isfinite(m_parameters.hitboxSize.x) || m_parameters.hitboxSize.x <= 0.0f ||
        !std::isfinite(m_parameters.hitboxSize.y) || m_parameters.hitboxSize.y <= 0.0f ||
        !std::isfinite(m_parameters.moveSpeed) || m_parameters.moveSpeed < 0.0f ||
        !std::isfinite(m_parameters.approachStopDistance) || m_parameters.approachStopDistance < 0.0f ||
        !std::isfinite(m_parameters.approachStartDistance) ||
        m_parameters.approachStartDistance <= m_parameters.approachStopDistance ||
        !std::isfinite(m_parameters.approachDelaySeconds) || m_parameters.approachDelaySeconds < 0.0f ||
        !std::isfinite(m_parameters.knockbackSeconds) || m_parameters.knockbackSeconds < 0.0f)
        return false;
    const auto& p = m_parameters;
    if (!std::isfinite(p.attackReach) || p.attackReach <= p.approachStopDistance ||
        !std::isfinite(p.thrustHeight) || p.thrustHeight <= 0.0f ||
        !std::isfinite(p.sweepHeight) || p.sweepHeight <= 0.0f ||
        !std::isfinite(p.attackOffsetY) ||
        !std::isfinite(p.firstHitStartFrame) || !std::isfinite(p.firstHitEndFrame) ||
        !std::isfinite(p.secondHitStartFrame) || !std::isfinite(p.secondHitEndFrame) ||
        p.firstHitStartFrame <= 0.0f || p.firstHitEndFrame <= p.firstHitStartFrame ||
        p.secondHitStartFrame <= p.firstHitEndFrame ||
        p.secondHitEndFrame <= p.secondHitStartFrame || p.secondHitEndFrame >= 120.0f ||
        p.firstHitDamage < 0 || p.secondHitDamage < 0 ||
        p.fastFirstHitDamage < 0 || p.fastSecondHitDamage < 0 ||
        !std::isfinite(p.recoverySeconds) || p.recoverySeconds < 0.0f ||
        !std::isfinite(p.fastRecoverySeconds) || p.fastRecoverySeconds < 0.0f ||
        !std::isfinite(p.attackKnockback.x) || !std::isfinite(p.attackKnockback.y)) return false;
    // BossEntity::Init adds gravity a second time. Reuse the enemy setup once.
    if (!EnemyEntity::Init()) return false;
    m_anim = AddComponent<AnimationComponent>();
    if (!m_transform || !m_velocity || !m_sprite || !m_collision || !m_hp || !m_gravity || !m_anim)
        return false;
    m_anim->SetSprite(m_sprite);
    m_hpMax = m_parameters.maxHP;
    m_moveSpeed = m_parameters.moveSpeed;
    m_dropTable.clear();
    m_initialized = true;
    return true;
}

void Boss_02::Update(float dt)
{
    if (!m_initialized || GetState() != Actor::State::Active || !std::isfinite(dt) || dt <= 0.0f)
        return;
    if (m_hp->IsDead()) OnDead();
    // VelocityComponent moves without collision. Integrate movement only below.
    for (auto* component : m_components)
        if (component != m_velocity && component != m_gravity) component->Update(dt);
    m_behaviorTime += dt;
    if (IsBattleFinished())
    {
        if (m_behaviorTime >= 14.0f / 60.0f)
        {
            SetState(Actor::State::Dead);
            if (OnBattleFinished) OnBattleFinished();
        }
        return;
    }
    if (m_knockbackRemaining > 0.0f)
        m_knockbackRemaining = (std::max)(0.0f, m_knockbackRemaining - dt);
    else
        UpdateAI(dt);
    m_gravity->Update(dt);
    MoveAndCollide(dt);
    if (m_behavior == Behavior::Move &&
        (GetVel().x == 0.0f || (m_jetTargetX - GetPos().x) * (m_dir ? 1.0f : -1.0f) <= 0.01f))
    {
        auto stopped = GetVel();
        stopped.x = 0.0f;
        SetVel(stopped);
        m_approachWaitTime = 0.0f;
        ChangeBehavior(Behavior::Idle);
    }
    if (m_isGround)
    {
        auto velocity = GetVel();
        velocity.y = 0.0f;
        SetVel(velocity);
    }
}

void Boss_02::UpdateAI(float dt)
{
    if (IsAttacking() || m_behavior == Behavior::Recovery)
    {
        auto velocity = GetVel();
        velocity.x = 0.0f;
        SetVel(velocity);
        UpdateAttack(dt);
        return;
    }
    PlayerEntity* player = nullptr;
    for (auto* actor : m_scene->GetActors())
        if (actor && actor->GetType() == ActorType::Player && actor->GetState() == Actor::State::Active)
        {
            player = dynamic_cast<PlayerEntity*>(actor);
            if (player) break;
        }
    auto velocity = GetVel();
    velocity.x = 0.0f;
    if (!player || !m_canMove)
    {
        m_approachWaitTime = 0.0f;
        ChangeBehavior(Behavior::Idle);
    }
    else
    {
        const float dx = player->GetPos().x - GetPos().x;
        const float gap = std::abs(dx) - (GetSize().x + player->GetSize().x) * 0.5f;
        if (m_behavior != Behavior::Move && dx != 0.0f) m_dir = dx > 0.0f;
        m_sprite->SetFlipX(m_dir != m_sourceFacesRight);
        if (m_behavior == Behavior::Move)
        {
            if ((m_jetTargetX - GetPos().x) * (m_dir ? 1.0f : -1.0f) <= 0.01f)
            {
                m_approachWaitTime = 0.0f;
                ChangeBehavior(Behavior::Idle);
            }
        }
        else if (gap > (std::min)(m_parameters.approachStartDistance, m_parameters.attackReach))
        {
            // Remain idle during detection; do not restart this timer each frame.
            m_approachWaitTime += dt;
            if (m_approachWaitTime >= m_parameters.approachDelaySeconds)
            {
                m_approachWaitTime = 0.0f;
                // Lock the destination and facing once. No homing during jet movement.
                m_jetTargetX = GetPos().x + (m_dir ? 1.0f : -1.0f) *
                    (gap - m_parameters.approachStopDistance);
                ChangeBehavior(Behavior::Move);
            }
        }
        else
        {
            m_approachWaitTime = 0.0f;
            const auto* col = player->GetCollision();
            const float playerHeight = col ? col->GetHeight() : player->GetSize().y;
            const float playerY = player->GetPos().y + (col ? col->GetOffset().y : 0.0f);
            if (m_isGround && gap <= m_parameters.attackReach &&
                std::abs(playerY - GetAttackCenter().y) <
                    (m_parameters.sweepHeight + playerHeight) * 0.5f)
                StartDoubleStrike();
        }
        if (m_behavior == Behavior::Move)
        {
            // Clamp to the stopping distance even on a slow frame.
            const float speed = (std::min)(m_parameters.moveSpeed,
                (std::max)(0.0f, (m_jetTargetX - GetPos().x) * (m_dir ? 1.0f : -1.0f)) / dt);
            velocity.x = (m_dir ? 1.0f : -1.0f) * speed;
        }
    }
    SetVel(velocity);
}

void Boss_02::TakeDamage(int damage, const Vector2d& knockback)
{
    if (!m_initialized || GetState() != Actor::State::Active || IsBattleFinished() ||
        damage <= 0 || m_hp->IsInvincible()) return;
    // Keep facing the attacker while knockback temporarily suspends approach AI.
    if (std::isfinite(knockback.x) && knockback.x != 0.0f)
    {
        m_dir = knockback.x < 0.0f;
        m_sprite->SetFlipX(m_dir != m_sourceFacesRight);
    }
    EnemyEntity::TakeDamage(damage,
        std::isfinite(knockback.x) && std::isfinite(knockback.y) ? knockback : Vector2d::Zero());
    if (!IsBattleFinished())
    {
        m_knockbackRemaining = m_parameters.knockbackSeconds;
        m_approachWaitTime = 0.0f;
        ChangeBehavior(Behavior::Idle);
    }
}

void Boss_02::TakeMetsu(int metsu)
{
    if (!m_initialized || IsBattleFinished() || metsu <= 0) return;
    // Saturate without overflowing on large input values.
    m_metsuGauge += (std::min)(metsu, m_metsuMax - m_metsuGauge);
    m_metsu = m_metsuGauge >= m_metsuMax;
}

void Boss_02::OnDead()
{
    if (!m_initialized || IsBattleFinished()) return;
    SetVel(Vector2d::Zero());
    m_canMove = false;
    m_attackActive = false;
    m_knockbackRemaining = 0.0f;
    m_approachWaitTime = 0.0f;
    ChangeBehavior(Behavior::Defeated);
}

void Boss_02::ChangeBehavior(Behavior behavior)
{
    if (m_behavior == behavior) return;
    m_activeStrike = 0;
    m_attackActive = false;
    m_behavior = behavior;
    m_behaviorTime = 0.0f;
    PlayBehaviorAnimation();
}

void Boss_02::PlayBehaviorAnimation()
{
    if (!m_anim) return;
    const char* clip = "idle";
    if (m_behavior == Behavior::Move) clip = "move";
    else if (m_behavior == Behavior::Defeated) clip = "defeated";
    else if (m_behavior == Behavior::DoubleStrike) clip = "doubleStrike";
    else if (m_behavior == Behavior::FastDoubleStrike) clip = "fastDoubleStrike";
    m_anim->Play(clip, true);
}

bool Boss_02::SetAnimationFrames(const std::vector<int>& idle, const std::vector<int>& move,
    const std::vector<int>& defeated, bool sourceFacesRight)
{
    if (!m_initialized || IsBattleFinished() || idle.size() != 3 || move.size() != 3 || defeated.size() != 7)
        return false;
    std::vector<int> handles = idle;
    handles.insert(handles.end(), move.begin(), move.end());
    handles.insert(handles.end(), defeated.begin(), defeated.end());
    if (std::any_of(handles.begin(), handles.end(), [](int handle) { return handle < 0; })) return false;
    m_baseAnimationFrames = handles;
    m_sprite->SetEffectFrames(handles);
    AnimationClip clip;
    clip.frames = { 0, 1, 2 };
    clip.speed = 3.0f / 60.0f;
    m_anim->AddClip("idle", clip);
    clip.frames = { 3, 4, 5 };
    clip.speed = 2.0f / 60.0f;
    m_anim->AddClip("move", clip);
    clip.frames = { 6, 7, 8, 9, 10, 11, 12 };
    clip.loop = false;
    m_anim->AddClip("defeated", clip);
    ConfigureAttackAnimations();
    m_sourceFacesRight = sourceFacesRight;
    m_sprite->SetFlipX(m_dir != m_sourceFacesRight);
    PlayBehaviorAnimation();
    return true;
}

void Boss_02::Draw()
{
    if (!m_initialized || GetState() == Actor::State::Dead) return;
    Actor::Draw();
    if (!m_parameters.showDebugShapes) return;
    auto center = GetPos();
    float zoom = 1.0f;
    if (auto* play = dynamic_cast<PlayScene*>(m_scene))
    {
        center = play->GetCamera().WorldToScreen(center);
        zoom = play->GetCamera().GetZoom();
    }
    const float w = m_parameters.hitboxSize.x * zoom * 0.5f;
    const float h = m_parameters.hitboxSize.y * zoom * 0.5f;
    const unsigned int color = IsBattleFinished() ? GetColor(150, 150, 150) :
        m_behavior == Behavior::Move ? GetColor(70, 190, 255) : GetColor(240, 210, 70);
    DrawBox(static_cast<int>(center.x - w), static_cast<int>(center.y - h),
        static_cast<int>(center.x + w), static_cast<int>(center.y + h), color,
        m_sprite->GetHandle() < 0 ? TRUE : FALSE);
    if (m_activeStrike != 0)
    {
        auto attackCenter = GetAttackCenter();
        if (auto* play = dynamic_cast<PlayScene*>(m_scene))
            attackCenter = play->GetCamera().WorldToScreen(attackCenter);
        const float aw = m_parameters.attackReach * zoom * 0.5f;
        const float ah = (m_activeStrike == 1 ? m_parameters.thrustHeight :
            m_parameters.sweepHeight) * zoom * 0.5f;
        DrawBox(static_cast<int>(attackCenter.x - aw), static_cast<int>(attackCenter.y - ah),
            static_cast<int>(attackCenter.x + aw), static_cast<int>(attackCenter.y + ah),
            m_fastAttack ? GetColor(255, 70, 230) : GetColor(255, 90, 50), FALSE);
    }
    const int eyeX = static_cast<int>(center.x + (m_dir ? w * 0.6f : -w * 0.6f));
    const int eyeY = static_cast<int>(center.y - h * 0.5f);
    DrawBox(eyeX - 3, eyeY - 3, eyeX + 3, eyeY + 3, GetColor(255, 255, 255), TRUE);
}

void Boss_02::SpawnForTest(Scene* scene, const Vector2d& pos)
{
    if (scene) scene->AddActor(new Boss_02(scene, pos));
}


bool Boss_02::IsAttacking() const
{
    return m_behavior == Behavior::DoubleStrike || m_behavior == Behavior::FastDoubleStrike;
}

Vector2d Boss_02::GetAttackCenter() const
{
    return GetPos() + Vector2d((m_dir ? 1.0f : -1.0f) *
        (m_parameters.hitboxSize.x + m_parameters.attackReach) * 0.5f,
        m_parameters.attackOffsetY);
}

void Boss_02::StartDoubleStrike()
{
    // Strictly below half HP; the selected speed stays fixed for the full combo.
    m_fastAttack = static_cast<double>(m_hp->GetHP()) < m_parameters.maxHP * 0.5;
    m_previousAttackTime = 0.0f;
    for (int i = 0; i < 2; ++i) { m_hitPlayer[i] = false; m_strikeNotified[i] = false; }
    ChangeBehavior(m_fastAttack ? Behavior::FastDoubleStrike : Behavior::DoubleStrike);
}

void Boss_02::UpdateAttack(float)
{
    if (m_behavior == Behavior::Recovery)
    {
        const float recovery = m_fastAttack ? m_parameters.fastRecoverySeconds : m_parameters.recoverySeconds;
        if (m_behaviorTime >= recovery) ChangeBehavior(Behavior::Idle);
        return;
    }
    if (!IsAttacking()) return;
    const float rate = m_fastAttack ? 120.0f : 60.0f;
    const float nowFrame = m_behaviorTime * rate;
    const float previousFrame = m_previousAttackTime * rate;
    const float starts[2] = { m_parameters.firstHitStartFrame, m_parameters.secondHitStartFrame };
    const float ends[2] = { m_parameters.firstHitEndFrame, m_parameters.secondHitEndFrame };
    m_activeStrike = 0;
    m_attackActive = false;
    for (int i = 0; i < 2; ++i)
    {
        // Include a crossed window on a slow frame so an entire swing is not lost.
        if (nowFrame >= starts[i] && previousFrame < ends[i])
        {
            if (!m_strikeNotified[i])
            {
                m_strikeNotified[i] = true;
                if (OnAttackStrike) OnAttackStrike(i + 1, m_fastAttack);
                if (!IsAttacking()) return;
            }
            if (!m_hitPlayer[i]) TryHitPlayer(i + 1);
            if (nowFrame < ends[i]) m_activeStrike = i + 1;
        }
    }
    m_attackActive = m_activeStrike != 0;
    m_previousAttackTime = m_behaviorTime;
    if (nowFrame >= 120.0f)
    {
        const float overrun = m_behaviorTime - 120.0f / rate;
        ChangeBehavior(Behavior::Recovery);
        m_behaviorTime = overrun;
    }
}

void Boss_02::TryHitPlayer(int stage)
{
    const auto center = GetAttackCenter();
    const float height = stage == 1 ? m_parameters.thrustHeight : m_parameters.sweepHeight;
    // Snapshot protects iteration if damage callbacks add actors.
    const auto actors = m_scene->GetActors();
    for (auto* actor : actors)
    {
        if (!actor || actor->GetType() != ActorType::Player || actor->GetState() != Actor::State::Active) continue;
        auto* player = dynamic_cast<PlayerEntity*>(actor);
        if (!player || !player->GetHP() || player->GetHP()->IsDead()) continue;
        const auto* col = player->GetCollision();
        if (!col) continue;
        const auto pos = player->GetPos() + col->GetOffset();
        if (std::abs(pos.x - center.x) >= (m_parameters.attackReach + col->GetWidth()) * 0.5f ||
            std::abs(pos.y - center.y) >= (height + col->GetHeight()) * 0.5f) continue;
        const int damage = stage == 1 ?
            (m_fastAttack ? m_parameters.fastFirstHitDamage : m_parameters.firstHitDamage) :
            (m_fastAttack ? m_parameters.fastSecondHitDamage : m_parameters.secondHitDamage);
        m_hitPlayer[stage - 1] = true;
        player->TakeDamage(damage, Vector2d((m_dir ? 1.0f : -1.0f) *
            std::abs(m_parameters.attackKnockback.x), m_parameters.attackKnockback.y));
        break; // One contact per swing; retain the player's existing dodge/invulnerability rules.
    }
}

bool Boss_02::SetAttackAnimationFrames(const std::vector<int>& normal, const std::vector<int>& fast)
{
    if (!m_initialized || IsBattleFinished() || IsAttacking() ||
        m_baseAnimationFrames.size() != 13 || normal.size() != 30 ||
        (!fast.empty() && fast.size() != 30)) return false;
    const auto invalid = [](int handle) { return handle < 0; };
    if (std::any_of(normal.begin(), normal.end(), invalid) ||
        std::any_of(fast.begin(), fast.end(), invalid)) return false;
    m_attackFrames = normal;
    m_fastAttackFrames = fast.empty() ? normal : fast;
    ConfigureAttackAnimations();
    PlayBehaviorAnimation();
    return true;
}

void Boss_02::ConfigureAttackAnimations()
{
    if (m_attackFrames.empty()) return;
    auto handles = m_baseAnimationFrames;
    const int base = static_cast<int>(handles.size());
    handles.insert(handles.end(), m_attackFrames.begin(), m_attackFrames.end());
    handles.insert(handles.end(), m_fastAttackFrames.begin(), m_fastAttackFrames.end());
    m_sprite->SetEffectFrames(handles);
    AnimationClip clip;
    clip.loop = false;
    clip.speed = 4.0f / 60.0f;
    for (int i = 0; i < 30; ++i) clip.frames.push_back(base + i);
    m_anim->AddClip("doubleStrike", clip);
    clip.frames.clear();
    clip.speed = 2.0f / 60.0f;
    for (int i = 0; i < 30; ++i) clip.frames.push_back(base + 30 + i);
    m_anim->AddClip("fastDoubleStrike", clip);
}
