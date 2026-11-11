#include "SmokeActor.h"
#include "PlayScene.h"
#include "Camera.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"

#include <DxLib.h>
#include <algorithm>

// 煙幕の持続時間
namespace
{
    constexpr float SMOKE_DURATION = 2.0f;
    constexpr float SMOKE_FADE_DURATION = 0.5f;
    constexpr float SMOKE_TOTAL_DURATION =
        SMOKE_DURATION + SMOKE_FADE_DURATION;
}

// コンストラクタ
SmokeActor::SmokeActor(
    Scene* scene,
    const Vector2d& position,
    const Vector2d& size
)
    : Actor(scene)
    , m_position(position)
    , m_transform(nullptr)
    , m_size(size)
    , m_sprite(nullptr)
    , m_anim(nullptr)
    , m_elapsedTime(0.0f)
    , m_expiredNotified(false)
    , m_state(0)
{
}

bool SmokeActor::Init()
{
    m_transform = AddComponent<TransformComponent>();
    m_transform->SetPosition(m_position);
    m_transform->SetScale({ 3.0f, 3.0f });

    m_sprite = AddComponent<SpriteComponent>();
    m_sprite->LoadTextureDiv("assets/images/effects/smoke_spritesheet.png", 6, 4);
    m_sprite->SetAlpha(200);
    m_sprite->SetDrawOffset(0.0f, -80.0f);

    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    AnimationClip start;
    start.frames = { 0, 1, 2, 3, 4, 5, 6 };
    start.speed = 0.05f;
    start.loop = false;
    m_anim->AddClip("start", start);

    AnimationClip loop;
    loop.frames = { 7, 8, 9, 10, 11, 12 };
    loop.speed = 0.4f;
    loop.loop = true;
    m_anim->AddClip("loop", loop);

    AnimationClip end;
    end.frames = { 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    end.speed = 0.08f;
    end.loop = false;
    m_anim->AddClip("end", end);

    m_anim->Play("start");

    return true;
}

// 更新処理
void SmokeActor::Update(float deltaTime)
{
    if (IsDead())
        return;

    if (m_state == 2) {
        if (m_anim->IsFinished()) SetState(State::Dead);

        Actor::Update(deltaTime);
        return;
    }

    Actor::Update(deltaTime);

    m_elapsedTime += deltaTime;

    if (m_state == 0 && m_anim->IsFinished()) {
        m_state++;
        m_anim->Play("loop");
    }

    // 持続時間が終了したら、煙幕終了処理を一度だけ呼ぶ
    if (m_elapsedTime >= SMOKE_TOTAL_DURATION)
    {
        if (!m_expiredNotified)
        {
            m_expiredNotified = true;

            // 敵が別の煙幕に入っているかなどの再判定は
            // PlayScene側に任せる
            PlayScene* playScene =
                dynamic_cast<PlayScene*>(m_scene);

            if (playScene != nullptr)
            {
                playScene->OnSmokeExpired(this);
            }
        }

        m_state++;
        m_anim->Play("end");
    }
}

// 指定座標が煙幕の範囲内にあるか
bool SmokeActor::Contains(const Vector2d& position) const
{
    const float halfWidth = m_size.x * 0.5f;
    const float halfHeight = m_size.y * 0.5f;

    return
        position.x >= m_position.x - halfWidth &&
        position.x <= m_position.x + halfWidth &&
        position.y >= m_position.y - halfHeight &&
        position.y <= m_position.y + halfHeight;
}

// 煙幕が有効か
bool SmokeActor::IsSmokeActive() const
{
    return
        !IsDead() &&
        m_elapsedTime < SMOKE_TOTAL_DURATION;
}

// 煙幕の中心座標
Vector2d SmokeActor::GetPosition() const
{
    return m_position;
}

// 煙幕の範囲
Vector2d SmokeActor::GetSize() const
{
    return m_size;
}