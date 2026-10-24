#include "boss_02.h"
#include "AnimationComponent.h"
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
        else if (gap > m_parameters.approachStartDistance)
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
            // Cancel the pending approach when the player returns within range.
            m_approachWaitTime = 0.0f;
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
    m_behavior = behavior;
    m_behaviorTime = 0.0f;
    PlayBehaviorAnimation();
}

void Boss_02::PlayBehaviorAnimation()
{
    if (m_anim) m_anim->Play(m_behavior == Behavior::Idle ? "idle" :
        m_behavior == Behavior::Move ? "move" : "defeated", true);
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
    const int eyeX = static_cast<int>(center.x + (m_dir ? w * 0.6f : -w * 0.6f));
    const int eyeY = static_cast<int>(center.y - h * 0.5f);
    DrawBox(eyeX - 3, eyeY - 3, eyeX + 3, eyeY + 3, GetColor(255, 255, 255), TRUE);
}

void Boss_02::SpawnForTest(Scene* scene, const Vector2d& pos)
{
    if (scene) scene->AddActor(new Boss_02(scene, pos));
}
