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
#include <cstdlib>
#include "PlayerEntity.h"
#include "PlayScene.h"
#include "BlockActor.h"
#include "Game.h"
#include "Renderer.h"

namespace
{
    bool Overlaps(const Vector2d& a, const Vector2d& size, const EntityActor* entity)
    {
        const auto* col = entity ? entity->GetCollision() : nullptr;
        if (!col || col->GetShape() == CollisionShape::None) return false;
        const auto b = entity->GetPos() + col->GetOffset();
        return std::fabs(a.x - b.x) <= (size.x + col->GetWidth()) * 0.5f &&
            std::fabs(a.y - b.y) <= (size.y + col->GetHeight()) * 0.5f;
    }

    // 移動する矩形を点に変換し、相手矩形を拡張した連続衝突判定。
    // 高速の手裏剣が薄い床やプレイヤーをすり抜けることを防ぐ。
    float Sweep(const Vector2d& from, const Vector2d& to, const Vector2d& size,
        const Vector2d& center, const Vector2d& targetSize)
    {
        float entry = 0.0f, exit = 1.0f;
        const float starts[] = {from.x, from.y};
        const float ends[] = {to.x, to.y};
        const float centers[] = {center.x, center.y};
        const float halves[] = {(size.x + targetSize.x) * 0.5f, (size.y + targetSize.y) * 0.5f};
        for (int axis = 0; axis < 2; ++axis)
        {
            const float delta = ends[axis] - starts[axis];
            const float low = centers[axis] - halves[axis];
            const float high = centers[axis] + halves[axis];
            if (std::fabs(delta) < 0.00001f)
            {
                if (starts[axis] < low || starts[axis] > high) return 2.0f;
                continue;
            }
            float a = (low - starts[axis]) / delta, b = (high - starts[axis]) / delta;
            if (a > b) std::swap(a, b);
            entry = (std::max)(entry, a);
            exit = (std::min)(exit, b);
            if (entry > exit) return 2.0f;
        }
        return entry;
    }
}

Boss_01::Boss_01(Scene* scene, const Vector2d& pos)
    : Boss_01(scene, pos, Parameters{})
{
}

void Boss_01::SpawnForTest(Scene* scene, const Vector2d& pos)
{
    if (!scene || !std::isfinite(pos.x) || !std::isfinite(pos.y)) return;
    Parameters parameters;
    parameters.showDebugShapes = true;
    // AddActorがInitを呼び、失敗した場合の破棄も行う。
    scene->AddActor(new Boss_01(scene, pos, parameters));
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

    const float positiveValues[] = {m_parameters.jumpSpeed, m_parameters.jumpHeight,
        m_parameters.approachStartDistance, m_parameters.approachStopDistance, m_parameters.knockbackSeconds,
        m_parameters.knockbackHorizontalScale, m_parameters.knockbackVerticalScale,
        m_parameters.shurikenSpeed, m_parameters.shurikenSize.x, m_parameters.shurikenSize.y,
        m_parameters.meleeSize.x, m_parameters.meleeSize.y, m_parameters.trapSize.x,
        m_parameters.trapSize.y, m_parameters.trapLifetime, m_parameters.poisonDuration,
        m_parameters.poisonDamagePerSecond, m_parameters.poisonMoveScale,
        m_parameters.recoverySeconds, m_parameters.actionInterval, m_parameters.meleeRange};
    for (float value : positiveValues)
        if (!std::isfinite(value) || value <= 0.0f) return false;
    if (m_parameters.shurikenDamage <= 0 || m_parameters.meleeDamage <= 0 ||
        m_parameters.poisonMoveScale > 1.0f ||
        m_parameters.approachStartDistance <= m_parameters.approachStopDistance) return false;
    m_nextAction = m_parameters.actionInterval;

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
    UpdateHazards(deltaTime);
    if (m_knockbackRemaining > 0.0f)
    {
        m_knockbackRemaining = (std::max)(0.0f, m_knockbackRemaining - deltaTime);
        if (m_knockbackRemaining <= 0.0f) SetVel({0.0f, GetVel().y});
    }
    else
    {
        UpdateAI(deltaTime);
        UpdateAttack(deltaTime);
    }

    // 速度コンポーネントによる直接移動とMoveAndCollideの二重移動を避ける。
    // 重力は既存GravityComponentを一度だけ更新する。
    for (auto* component : m_components)
    {
        if (component == m_velocity || component == m_gravity) continue;
        component->Update(deltaTime);
    }
    if (!IsBattleFinished())
    {
        m_gravity->Update(deltaTime);
        MoveAndCollide(deltaTime);
        if (m_isGround) SetVel({GetVel().x, 0.0f});
        // 着地したフレームから硬直を開始する。投擲中も重力を止めない。
        if (m_isGround && (m_behavior == Behavior::Shuriken || m_behavior == Behavior::Fall))
        {
            SetVel(Vector2d::Zero());
            ChangeBehavior(Behavior::Recovery);
        }
    }
    else SetVel(Vector2d::Zero());
}

void Boss_01::UpdateAI(float deltaTime)
{
    if (IsBattleFinished()) return;
    if (m_behavior != Behavior::Idle && m_behavior != Behavior::Move) return;
    m_nextAction = (std::max)(0.0f, m_nextAction - deltaTime);

    auto* player = FindPlayer();

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

    // 中心間距離ではなく、体の端同士の距離で近接の届く範囲を判定する。
    const auto* playerCollision = player->GetCollision();
    const float playerHalfWidth = playerCollision ? playerCollision->GetWidth() * 0.5f : 0.0f;
    const float edgeDistance = (std::max)(0.0f,
        distance - m_parameters.hitboxSize.x * 0.5f - playerHalfWidth);
    const bool nearPlayer = edgeDistance <= (std::min)(m_parameters.meleeRange, m_parameters.meleeSize.x);

    if (distance >= m_parameters.approachStartDistance) m_approaching = true;
    if (distance <= m_parameters.approachStopDistance) m_approaching = false;
    if (m_approaching)
    {
        ChangeBehavior(Behavior::Move);
        const float speed = (std::min)(m_moveSpeed,
            (distance - m_parameters.approachStopDistance) / deltaTime);
        SetVel({(m_dir ? 1.0f : -1.0f) * speed, GetVel().y});
        return;
    }

    if (m_nextAction <= 0.0f && m_isGround)
    {
        // 近接は届くときだけ候補にする。近距離でも手裏剣・罠を混ぜる。
        const int roll = std::rand() % 100;
        Behavior choice = nearPlayer
            ? (roll < 60 ? Behavior::Melee : (roll < 80 ? Behavior::Jump : Behavior::PoisonTrap))
            : (roll < 70 ? Behavior::Jump : Behavior::PoisonTrap);
        if (m_attacksWithoutTrap >= 2) choice = Behavior::PoisonTrap;
        else if (choice == m_lastAttack && std::rand() % 100 < 50)
            choice = choice == Behavior::PoisonTrap ? (nearPlayer ? Behavior::Melee : Behavior::Jump)
                : Behavior::PoisonTrap;
        bool started = false;
        switch (choice)
        {
        case Behavior::Melee: started = StartMeleeAttack(); break;
        case Behavior::PoisonTrap: started = StartPoisonTrap(); break;
        default: started = StartJumpAttack(); break;
        }
        if (started)
        {
            m_lastAttack = choice;
            m_attacksWithoutTrap = choice == Behavior::PoisonTrap ? 0 : m_attacksWithoutTrap + 1;
        }
        return;
    }
    if (nearPlayer)
    {
        ChangeBehavior(Behavior::Idle);
        SetVel({0.0f, GetVel().y});
        return;
    }

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

void Boss_01::UpdateAttack(float deltaTime)
{
    m_behaviorTime += deltaTime;
    switch (m_behavior)
    {
    case Behavior::Jump:
        // 頂点（または天井に接触して上昇が止まった時）で投擲する。
        if (GetVel().y >= 0.0f)
        {
            ChangeBehavior(Behavior::Shuriken);
            SpawnShuriken();
            m_attackTriggered = true;
        }
        break;
    case Behavior::Shuriken:
        if (!m_attackTriggered)
        {
            SpawnShuriken();
            m_attackTriggered = true;
        }
        if (m_behaviorTime >= 60.0f / 60.0f) ChangeBehavior(Behavior::Fall);
        break;
    case Behavior::Fall:
        if (m_isGround) ChangeBehavior(Behavior::Recovery);
        break;
    case Behavior::Melee:
        // 10fの中で4～7fを仮の有効判定とする。エディタ導入後に差し替える。
        if (!m_meleeHit && m_behaviorTime >= 4.0f / 60.0f &&
            m_behaviorTime - deltaTime <= 7.0f / 60.0f)
        {
            auto* player = FindPlayer();
            const Vector2d center = GetPos() + Vector2d{
                m_attackDirection * (m_parameters.hitboxSize.x + m_parameters.meleeSize.x) * 0.5f, 0.0f};
            if (Overlaps(center, m_parameters.meleeSize, player))
            {
                player->TakeDamage(m_parameters.meleeDamage, {m_attackDirection * 150.0f, -100.0f});
                m_meleeHit = true;
            }
        }
        if (m_behaviorTime >= 10.0f / 60.0f) ChangeBehavior(Behavior::Recovery);
        break;
    case Behavior::PoisonTrap:
        if (!m_attackTriggered && m_behaviorTime >= 6.0f / 60.0f)
        {
            PlacePoisonTrap();
            m_attackTriggered = true;
        }
        if (m_behaviorTime >= 12.0f / 60.0f) ChangeBehavior(Behavior::Recovery);
        break;
    case Behavior::Recovery:
        if (m_behaviorTime >= m_parameters.recoverySeconds)
        {
            m_nextAction = m_parameters.actionInterval;
            ChangeBehavior(Behavior::Idle);
        }
        break;
    default: break;
    }
}

void Boss_01::TakeDamage(int damage, const Vector2d& knockback)
{
    if (!m_initialized || IsBattleFinished() || damage <= 0 || m_hp->IsInvincible()) return;
    // 強い一撃でもHP600で踏みとどまり、通常敵の死亡・消去へ進ませない。
    const int remaining = m_hp->GetHP() - GetDefeatHP();
    if (remaining <= 0) { EnterDefeated(); return; }
    m_hp->Damage((std::min)(damage, remaining));
    if (!IsBattleFinished() && std::isfinite(knockback.x) && std::isfinite(knockback.y))
    {
        ChangeBehavior(Behavior::Recovery);
        m_knockbackRemaining = m_parameters.knockbackSeconds;
        // 攻撃側から渡された向きを維持し、後方への移動と浮き上がりを強める。
        const Vector2d launch{knockback.x * m_parameters.knockbackHorizontalScale,
            knockback.y * m_parameters.knockbackVerticalScale};
        SetVel(launch);
        if (launch.y < 0.0f) m_isGround = false;
    }
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
    m_shuriken.clear();
    m_traps.clear();
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
    m_behaviorTime = 0.0f;
    m_attackTriggered = false;
    PlayBehaviorAnimation();
}

void Boss_01::PlayBehaviorAnimation()
{
    if (!m_anim) return;
    const std::vector<int>* attackFrames = nullptr;
    float duration = 0.0f;
    switch (m_behavior)
    {
    case Behavior::Jump: attackFrames = &m_jumpFrames; duration = 10.0f / 60.0f; break;
    case Behavior::Shuriken: attackFrames = &m_throwFrames; duration = 1.0f; break;
    case Behavior::Melee: attackFrames = &m_meleeFrames; duration = 10.0f / 60.0f; break;
    case Behavior::PoisonTrap: attackFrames = &m_trapFrames; duration = 12.0f / 60.0f; break;
    default: break;
    }
    if (attackFrames && !attackFrames->empty())
    {
        m_sprite->SetEffectFrames(*attackFrames);
        AnimationClip clip;
        for (int i = 0; i < static_cast<int>(attackFrames->size()); ++i) clip.frames.push_back(i);
        clip.speed = duration / static_cast<float>(attackFrames->size());
        clip.loop = false;
        m_anim->AddClip("boss_01_action", clip);
        m_anim->Play("boss_01_action", true);
        return;
    }
    if (m_baseFrames.empty()) return;
    m_sprite->SetEffectFrames(m_baseFrames);
    switch (m_behavior)
    {
    case Behavior::Idle: m_anim->Play("boss_01_idle", true); break;
    case Behavior::Move: m_anim->Play("boss_01_move", true); break;
    case Behavior::Defeated: m_anim->Play("boss_01_defeated", true); break;
    default: m_anim->Play("boss_01_idle", true); break;
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

    m_baseFrames = frames;
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


PlayerEntity* Boss_01::FindPlayer() const
{
    if (!m_scene) return nullptr;
    for (auto* actor : m_scene->GetActors())
    {
        if (actor && actor->GetType() == ActorType::Player && actor->GetState() == Actor::State::Active)
        {
            auto* player = dynamic_cast<PlayerEntity*>(actor);
            if (player && player->GetHP() && !player->GetHP()->IsDead()) return player;
        }
    }
    return nullptr;
}

bool Boss_01::CanStartAttack() const
{
    return m_initialized && GetState() == Actor::State::Active && m_canMove && m_isGround &&
        (m_behavior == Behavior::Idle || m_behavior == Behavior::Move);
}

void Boss_01::BeginAttack(Behavior behavior)
{
    auto* player = FindPlayer();
    if (player && player->GetPos().x != GetPos().x) m_dir = player->GetPos().x > GetPos().x;
    m_attackDirection = m_dir ? 1.0f : -1.0f;
    m_sprite->SetFlipX(m_dir != m_sourceFacesRight);
    SetVel(Vector2d::Zero());
    m_meleeHit = false;
    ChangeBehavior(behavior);
}

bool Boss_01::StartJumpAttack()
{
    if (!CanStartAttack()) return false;
    BeginAttack(Behavior::Jump);
    // v^2 = 2gh。600px/sだけでは重力2800で約64pxしか跳べないため高さを優先。
    const float initialSpeed = (std::max)(m_parameters.jumpSpeed,
        std::sqrt(2.0f * m_gravity->GetGravity() * m_parameters.jumpHeight));
    SetVel({0.0f, -initialSpeed});
    m_isGround = false;
    return true;
}

bool Boss_01::StartMeleeAttack()
{
    if (!CanStartAttack()) return false;
    BeginAttack(Behavior::Melee);
    return true;
}

bool Boss_01::StartPoisonTrap()
{
    if (!CanStartAttack()) return false;
    BeginAttack(Behavior::PoisonTrap);
    return true;
}

void Boss_01::SpawnShuriken()
{
    for (float degrees : {30.0f, 45.0f, 60.0f})
    {
        const float radians = degrees * 3.14159265358979323846f / 180.0f;
        ShurikenData shot;
        shot.position = GetPos();
        shot.velocity = {m_attackDirection * std::cos(radians) * m_parameters.shurikenSpeed,
            std::sin(radians) * m_parameters.shurikenSpeed};
        m_shuriken.push_back(shot);
    }
}

bool Boss_01::IsVisible(const Vector2d& position, const Vector2d& size) const
{
    const auto* play = dynamic_cast<const PlayScene*>(m_scene);
    if (!play || !m_scene->GetGame()) return false;
    const auto point = play->GetCamera().WorldToScreen(position);
    const float zoom = play->GetCamera().GetZoom();
    return point.x + size.x * zoom * 0.5f >= 0.0f &&
        point.y + size.y * zoom * 0.5f >= 0.0f &&
        point.x - size.x * zoom * 0.5f <= m_scene->GetGame()->GetWidth() &&
        point.y - size.y * zoom * 0.5f <= m_scene->GetGame()->GetHeight();
}

void Boss_01::PlacePoisonTrap()
{
    // 実際に存在する地形の上面のみを候補にする。空中・画面外には置かない。
    std::vector<Vector2d> candidates;
    for (auto* actor : m_scene->GetActors())
    {
        auto* block = dynamic_cast<BlockActor*>(actor);
        if (!block || block->GetState() != Actor::State::Active || !block->GetCollision()) continue;
        const auto* col = block->GetCollision();
        if (col->GetShape() != CollisionShape::Rect || col->GetWidth() < m_parameters.trapSize.x) continue;
        const auto center = block->GetPos() + col->GetOffset();
        const float span = col->GetWidth() - m_parameters.trapSize.x;
        const float random = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        const Vector2d pos{center.x + (random - 0.5f) * span,
            center.y - col->GetHeight() * 0.5f - m_parameters.trapSize.y * 0.5f};
        if (!IsVisible(pos, m_parameters.trapSize)) continue;
        bool buried = false;
        for (auto* other : m_scene->GetActors())
        {
            auto* obstacle = dynamic_cast<BlockActor*>(other);
            if (!obstacle || obstacle == block || obstacle->GetState() != Actor::State::Active ||
                !obstacle->GetCollision()) continue;
            const auto* c = obstacle->GetCollision();
            if (c->GetShape() != CollisionShape::Rect) continue;
            const auto o = obstacle->GetPos() + c->GetOffset();
            if (std::fabs(pos.x - o.x) < (m_parameters.trapSize.x + c->GetWidth()) * 0.5f &&
                std::fabs(pos.y - o.y) < (m_parameters.trapSize.y + c->GetHeight()) * 0.5f)
            { buried = true; break; }
        }
        if (!buried) candidates.push_back(pos);
    }
    if (candidates.empty()) return;
    if (m_traps.size() >= 3) m_traps.erase(m_traps.begin());
    m_traps.push_back({candidates[static_cast<std::size_t>(std::rand()) % candidates.size()],
        m_parameters.trapLifetime});
}

void Boss_01::UpdateHazards(float deltaTime)
{
    auto* player = FindPlayer();
    if (player != m_poisonTarget || !player)
    {
        // 【死亡・ステージ変更時の解除：プレイヤー側のリセット処理に追加】
        // SetMovementMultiplier(1.0f); // 正式な関数名へ置換する。
        // この位置のm_poisonTargetは既に破棄済みの場合があるため、呼び出し禁止。
        // プレイヤーの死亡/ResetStageState/再生成時に1.0fへ戻すよう依頼する。
        // ボスだけを破棄する場合も、生存中のプレイヤーへ安全に解除する接続が必要。
        m_poisonRemaining = 0.0f;
        m_poisonFraction = 0.0;
        m_poisonTarget = nullptr;
    }
    if (m_poisonRemaining > 0.0f && player)
    {
        const float elapsed = (std::min)(m_poisonRemaining, deltaTime);
        m_poisonRemaining = (std::max)(0.0f, m_poisonRemaining - elapsed);
        // HPが整数なので端数を保持。10秒で計15ダメージ。通常被弾の硬直は発生させない。
        m_poisonFraction += static_cast<double>(elapsed) * m_parameters.poisonDamagePerSecond;
        const int damage = static_cast<int>(std::floor(m_poisonFraction + 0.00001));
        if (damage > 0)
        {
            player->GetHP()->Damage(damage);
            m_poisonFraction = (std::max)(0.0, m_poisonFraction - damage);
        }
        // 【10秒経過で解除：下のifの中に倍率変更関数を追加】
        // 接続時には下の1行ifを波括弧付きにして、次の形にする（関数名は仮）：
        // if (m_poisonRemaining <= 0.0f)
        // {
        //     player->SetMovementMultiplier(1.0f);
        //     m_poisonFraction = 0.0;
        // }
        // ifの外に解除を置くと、毎フレーム減速が消えるので注意。
        if (m_poisonRemaining <= 0.0f) m_poisonFraction = 0.0;
        if (player->GetHP()->IsDead()) player = nullptr;
    }

    for (auto it = m_shuriken.begin(); it != m_shuriken.end();)
    {
        const auto next = it->position + it->velocity * deltaTime;
        float blockTime = 2.0f, playerTime = 2.0f;
        for (auto* actor : m_scene->GetActors())
        {
            auto* block = dynamic_cast<BlockActor*>(actor);
            if (!block || block->GetState() != Actor::State::Active || !block->GetCollision()) continue;
            const auto* c = block->GetCollision();
            if (c->GetShape() != CollisionShape::Rect) continue;
            blockTime = (std::min)(blockTime, Sweep(it->position, next, m_parameters.shurikenSize,
                block->GetPos() + c->GetOffset(), {c->GetWidth(), c->GetHeight()}));
        }
        if (player && player->GetCollision() && player->GetCollision()->GetShape() != CollisionShape::None)
        {
            const auto* c = player->GetCollision();
            playerTime = Sweep(it->position, next, m_parameters.shurikenSize,
                player->GetPos() + c->GetOffset(), {c->GetWidth(), c->GetHeight()});
        }
        if (playerTime <= 1.0f && playerTime < blockTime)
            player->TakeDamage(m_parameters.shurikenDamage, {it->velocity.x > 0.0f ? 100.0f : -100.0f, 0.0f});
        it->position = next;
        if (blockTime <= 1.0f || playerTime <= 1.0f || !IsVisible(next, m_parameters.shurikenSize))
            it = m_shuriken.erase(it);
        else ++it;
    }
    const auto* playerCol = player ? player->GetCollision() : nullptr;
    const Vector2d playerCenter = playerCol ? player->GetPos() + playerCol->GetOffset() : Vector2d::Zero();
    for (auto it = m_traps.begin(); it != m_traps.end();)
    {
        it->remaining -= deltaTime;
        if (it->remaining <= 0.0f) { it = m_traps.erase(it); continue; }
        const bool crossed = playerCol && m_previousPlayer == player &&
            playerCol->GetShape() != CollisionShape::None &&
            Sweep(m_previousPlayerCenter, playerCenter, {playerCol->GetWidth(), playerCol->GetHeight()},
                it->position, m_parameters.trapSize) <= 1.0f;
        if (player && !IsBattleFinished() &&
            (crossed || Overlaps(it->position, m_parameters.trapSize, player)))
        {
            m_poisonTarget = player;
            m_poisonRemaining = m_parameters.poisonDuration;
            // 毒は重複させず残り時間だけ更新。接触した罠は消費する。
            it = m_traps.erase(it);
            // 【減速開始・再接触：ここに倍率変更関数を追加】
            // 正式な関数名が決まったら、下の仮名を置き換えて有効にする。
            // player->SetMovementMultiplier(m_parameters.poisonMoveScale); // 0.85f
            // 移動・回避に同じ倍率を設定。重ね掛けしない。10秒はボス側で管理する。
            // 下の旧通知方式と直接呼び出し方式は併用せず、接続時に一本化する。
            if (OnPoisonSlowRequested)
                OnPoisonSlowRequested(player, m_parameters.poisonDuration, m_parameters.poisonMoveScale);
        }
        else ++it;
    }
    m_previousPlayer = player;
    m_previousPlayerCenter = playerCenter;
}

void Boss_01::Draw()
{
    Actor::Draw();
    if (GetState() == Actor::State::Dead || !m_scene || !m_scene->GetGame()) return;
    auto* renderer = m_scene->GetGame()->GetRenderer();
    if (!renderer) return;
    if (!m_parameters.showDebugShapes) return;

    Color bodyColor(80, 170, 240);
    const char* label = "IDLE";
    switch (m_behavior)
    {
    case Behavior::Move: bodyColor = Color(70, 210, 130); label = "MOVE"; break;
    case Behavior::Jump: bodyColor = Color(255, 220, 60); label = "JUMP"; break;
    case Behavior::Shuriken: bodyColor = Color(230, 230, 250); label = "SHURIKEN"; break;
    case Behavior::Fall: bodyColor = Color(240, 180, 60); label = "FALL"; break;
    case Behavior::Melee: bodyColor = Color(255, 100, 60); label = "MELEE"; break;
    case Behavior::PoisonTrap: bodyColor = Color(180, 70, 220); label = "TRAP"; break;
    case Behavior::Recovery: bodyColor = Color(150, 150, 160); label = "RECOVERY"; break;
    case Behavior::Defeated: bodyColor = Color(90, 90, 100); label = "DEFEATED"; break;
    default: break;
    }
    const Vector2d bodyCenter = GetPos() + (m_collision ? m_collision->GetOffset() : Vector2d::Zero());
    const float bodyWidth = m_collision ? m_collision->GetWidth() : m_parameters.hitboxSize.x;
    const float bodyHeight = m_collision ? m_collision->GetHeight() : m_parameters.hitboxSize.y;
    renderer->DrawRectCenter(bodyCenter, bodyWidth, bodyHeight, bodyColor, true);
    renderer->DrawRectCenter(bodyCenter, bodyWidth, bodyHeight, Color(255, 255, 255), false);
    // 小さな白い四角で向きを示す。後退中にもプレイヤーを向いていることを確認できる。
    renderer->DrawRectCenter(bodyCenter + Vector2d{(m_dir ? 1.0f : -1.0f) * bodyWidth * 0.35f,
        -bodyHeight * 0.25f}, 10.0f, 10.0f, Color(255, 255, 255), true);
    renderer->DrawTextC(bodyCenter + Vector2d{0.0f, -bodyHeight * 0.5f - 28.0f},
        label, Color(255, 255, 255), "Arial", 18, true);

    for (const auto& shot : m_shuriken)
        renderer->DrawRectCenter(shot.position, m_parameters.shurikenSize.x,
            m_parameters.shurikenSize.y, Color(230, 230, 250), true);
    for (const auto& trap : m_traps)
    {
        // 判定は0.5pxのまま、表示だけ6pxにして見やすくする。
        renderer->DrawRectCenter(trap.position, m_parameters.trapSize.x,
            (std::max)(6.0f, m_parameters.trapSize.y), Color(180, 70, 220), true);
    }
    if (m_behavior == Behavior::Melee)
    {
        // 予備動作・後隙は枠のみ。実際に判定のある4～7fだけ塗りつぶす。
        const bool active = m_behaviorTime >= 4.0f / 60.0f && m_behaviorTime <= 7.0f / 60.0f;
        renderer->DrawRectCenter(GetPos() + Vector2d{
            m_attackDirection * (m_parameters.hitboxSize.x + m_parameters.meleeSize.x) * 0.5f, 0.0f},
            m_parameters.meleeSize.x, m_parameters.meleeSize.y, Color(255, 100, 60), active);
    }
}

bool Boss_01::SetAttackAnimationFrames(Behavior behavior, const std::vector<int>& frames)
{
    if (!m_initialized) return false;
    std::vector<int>* destination = nullptr;
    std::size_t count = 0;
    switch (behavior)
    {
    case Behavior::Jump: destination = &m_jumpFrames; count = 6; break;
    case Behavior::Shuriken: destination = &m_throwFrames; count = 12; break;
    case Behavior::Melee: destination = &m_meleeFrames; count = 3; break;
    case Behavior::PoisonTrap: destination = &m_trapFrames; count = 10; break;
    default: return false;
    }
    if (frames.size() != count) return false;
    for (int handle : frames)
    {
        int width = 0, height = 0;
        if (handle < 0 || GetGraphSize(handle, &width, &height) < 0 || width <= 0 || height <= 0) return false;
    }
    *destination = frames;
    if (m_behavior == behavior) PlayBehaviorAnimation();
    return true;
}
