#include "boss_01.h"
#include "AnimationComponent.h"
#include "CollisionComponent.h"
#include "GravityComponent.h"
#include "HPComponent.h"
#include "Scene.h"
#include "SpriteComponent.h"
#include "VelocityComponent.h"
#include <algorithm>
#include <cmath>

Boss_01::Boss_01(Scene* scene, const Vector2d& pos)
    : Boss_01(scene, pos, Parameters{})
{
}

Boss_01::Boss_01(Scene* scene, const Vector2d& pos, const Parameters& parameters)
    : BossEntity(scene, pos, parameters.hitboxSize), m_parameters(parameters)
{
}

bool Boss_01::Init()
{
    if (m_initialized) return true;
    if (!m_scene || !m_components.empty()) return false;
    if (m_parameters.maxHP <= 0 ||
        !std::isfinite(m_parameters.defeatHPRatio) ||
        m_parameters.defeatHPRatio <= 0.0f || m_parameters.defeatHPRatio >= 1.0f ||
        !std::isfinite(m_parameters.hitboxSize.x) || m_parameters.hitboxSize.x <= 0.0f ||
        !std::isfinite(m_parameters.hitboxSize.y) || m_parameters.hitboxSize.y <= 0.0f ||
        !std::isfinite(m_parameters.moveSpeed) || m_parameters.moveSpeed < 0.0f ||
        !std::isfinite(m_parameters.retreatStartDistance) || m_parameters.retreatStartDistance < 0.0f ||
        !std::isfinite(m_parameters.retreatStopDistance) ||
        m_parameters.retreatStopDistance <= m_parameters.retreatStartDistance)
        return false;

    // BossEntity::Init は重力を追加で登録するため、共通の敵初期化を一度だけ行う。
    if (!EnemyEntity::Init()) return false;
    if (!m_transform || !m_velocity || !m_sprite || !m_collision || !m_hp || !m_gravity)
        return false;
    m_anim = AddComponent<AnimationComponent>();
    if (!m_anim) return false;
    m_anim->SetSprite(m_sprite);
    m_hpMax = GetMaxHP();
    m_moveSpeed = m_parameters.moveSpeed;
    m_dropTable.clear();
    m_initialized = true;
    SetVel(Vector2d::Zero());

    // HPComponent経由の直接ダメージでも終了条件を取りこぼさない。
    m_hp->OnHPChanged = [this](int newHP, int) {
        if (newHP <= GetDefeatHP()) EnterDefeated();
    };
    return true;
}

int Boss_01::GetDefeatHP() const
{
    return static_cast<int>(std::floor(
        static_cast<double>(m_parameters.maxHP) * m_parameters.defeatHPRatio));
}

void Boss_01::Update(float deltaTime)
{
    if (!m_initialized || GetState() != Actor::State::Active ||
        !std::isfinite(deltaTime) || deltaTime <= 0.0f) return;

    if (m_hp->GetHP() <= GetDefeatHP()) EnterDefeated();
    UpdateAI(deltaTime);

    // 速度コンポーネントによる直接移動とMoveAndCollideの二重移動を避ける。
    // 重力もEnemyEntity::UpdateGravityだけで処理する。
    for (auto* component : m_components)
    {
        if (component == m_velocity || component == m_gravity) continue;
        component->Update(deltaTime);
    }
    if (!IsBattleFinished()) UpdateGravity(deltaTime);
    else SetVel(Vector2d::Zero());
}

void Boss_01::UpdateAI(float deltaTime)
{
    if (IsBattleFinished()) return;

    EntityActor* player = nullptr;
    for (auto* actor : m_scene->GetActors())
    {
        if (actor && actor->GetType() == ActorType::Player &&
            actor->GetState() == Actor::State::Active)
        {
            player = dynamic_cast<EntityActor*>(actor);
            if (player) break;
        }
    }

    if (!player || !m_canMove)
    {
        ChangeBehavior(Behavior::Idle);
        SetVel({ 0.0f, GetVel().y });
        return;
    }

    const float difference = player->GetPos().x - GetPos().x;
    const float distance = std::fabs(difference);
    if (difference != 0.0f) m_dir = difference > 0.0f;
    m_sprite->SetFlipX(m_dir != m_sourceFacesRight);

    if (m_behavior == Behavior::Idle && distance < m_parameters.retreatStartDistance)
        ChangeBehavior(Behavior::Move);
    else if (m_behavior == Behavior::Move && distance >= m_parameters.retreatStopDistance)
        ChangeBehavior(Behavior::Idle);

    float speed = 0.0f;
    if (m_behavior == Behavior::Move)
    {
        // 停止距離を通り過ぎない範囲で、プレイヤーに背を向けず後退する。
        speed = (std::min)(m_moveSpeed,
            (m_parameters.retreatStopDistance - distance) / deltaTime);
        speed *= m_dir ? -1.0f : 1.0f;
    }
    SetVel({ speed, GetVel().y });
}

void Boss_01::UpdateAttack(float)
{
}

void Boss_01::TakeDamage(int damage, const Vector2d&)
{
    if (!m_initialized || IsBattleFinished() || damage <= 0 || m_hp->IsInvincible()) return;
    // 強い一撃でもHP600で踏みとどまり、通常敵の死亡・消去へ進ませない。
    const int remaining = m_hp->GetHP() - GetDefeatHP();
    if (remaining <= 0) { EnterDefeated(); return; }
    m_hp->Damage((std::min)(damage, remaining));
}

void Boss_01::TakeMetsu(int)
{
    // 通常敵向けの即死処理を有効にしない。洲条はHP30%で戦闘終了する。
}

void Boss_01::OnDead()
{
    if (m_initialized) EnterDefeated();
}

void Boss_01::EnterDefeated()
{
    if (IsBattleFinished()) return;
    m_canMove = false;
    m_attack = false;
    m_attackActive = false;
    SetVel(Vector2d::Zero());
    ChangeBehavior(Behavior::Defeated);
    // Actor::State::Dead は設定しない。片膝の呼吸アニメーションを残す。
    if (OnBattleFinished) OnBattleFinished();
}

void Boss_01::ChangeBehavior(Behavior behavior)
{
    if (m_behavior == behavior) return;
    m_behavior = behavior;
    PlayBehaviorAnimation();
}

void Boss_01::PlayBehaviorAnimation()
{
    if (!m_anim) return;
    switch (m_behavior)
    {
    case Behavior::Idle: m_anim->Play("boss_01_idle", true); break;
    case Behavior::Move: m_anim->Play("boss_01_move", true); break;
    case Behavior::Defeated: m_anim->Play("boss_01_defeated", true); break;
    }
}

bool Boss_01::SetAnimationFrames(const std::vector<int>& idle,
    const std::vector<int>& move, const std::vector<int>& defeated, bool sourceFacesRight)
{
    if (!m_initialized || idle.size() != 3 || move.size() != 6 || defeated.size() != 7)
        return false;
    std::vector<int> frames = idle;
    frames.insert(frames.end(), move.begin(), move.end());
    frames.insert(frames.end(), defeated.begin(), defeated.end());
    for (int handle : frames)
    {
        int width = 0, height = 0;
        if (handle < 0 || GetGraphSize(handle, &width, &height) < 0 || width <= 0 || height <= 0)
            return false;
    }

    m_sourceFacesRight = sourceFacesRight;
    m_sprite->SetEffectFrames(frames);
    m_sprite->SetDrawSize(m_parameters.hitboxSize.x, m_parameters.hitboxSize.y);
    m_sprite->SetFlipX(m_dir != m_sourceFacesRight);

    // プログラムドキュメントの60fpsを基準とする。
    // 待機9f/3枚、移動12f/6枚、やられ14f/7枚。全状態でループ。
    const auto addClip = [this](const char* name, int first, int count, float totalFrames) {
        AnimationClip clip;
        for (int i = 0; i < count; ++i) clip.frames.push_back(first + i);
        clip.speed = totalFrames / (60.0f * count);
        clip.loop = true;
        m_anim->AddClip(name, clip);
    };
    addClip("boss_01_idle", 0, 3, 9.0f);
    addClip("boss_01_move", 3, 6, 12.0f);
    addClip("boss_01_defeated", 9, 7, 14.0f);
    PlayBehaviorAnimation();
    return true;
}
