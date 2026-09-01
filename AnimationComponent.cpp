#include "AnimationComponent.h"
#include "SpriteComponent.h"
#include "Actor.h"

// --------------------
// コンストラクタ: 自動で SpriteComponent を取得
// --------------------
AnimationComponent::AnimationComponent(Actor* owner)
    : Component(owner)
    , m_sprite(nullptr)
    , m_currentClip(nullptr)
    , m_timer(0.0f)
    , m_frameIndex(0)
{
}

// --------------------
// クリップ追加
// --------------------
void AnimationComponent::AddClip(const std::string& name, const AnimationClip& clip)
{
    m_clips[name] = clip;
}

// --------------------
// アニメ再生
// --------------------
void AnimationComponent::Play(const std::string& name, bool reset)
{
    auto it = m_clips.find(name);
    if (it == m_clips.end())
        return;

    if (!reset && m_currentName == name)
        return;

    m_currentName = name;
    m_currentClip = &it->second;

    m_timer = 0.0f;
    m_frameIndex = 0;

    if (m_sprite && !m_currentClip->frames.empty())
    {
        m_sprite->SetFrame(m_currentClip->frames[0]);
    }
}

bool AnimationComponent::IsFinished() const
{
    if (!m_currentClip)
        return true;

    if (m_currentClip->loop)
        return false;

    return m_frameIndex >= static_cast<int>(m_currentClip->frames.size()) - 1;
}

// --------------------
// 更新（フレーム切替）
// --------------------
void AnimationComponent::Update(float deltaTime)
{
    if (!m_currentClip || !m_sprite)
        return;

    const auto& frames = m_currentClip->frames;
    if (frames.empty())
        return;

    m_timer += deltaTime;

    while (true)
    {
        float duration = m_currentClip->speed;

        if (!m_currentClip->frameDurations.empty())
        {
            if (m_frameIndex >= static_cast<int>(m_currentClip->frameDurations.size()))
                break;

            duration = m_currentClip->frameDurations[m_frameIndex];
        }

        if (m_timer < duration)
            break;

        m_timer -= duration;
        m_frameIndex++;

        int last = static_cast<int>(frames.size()) - 1;

        if (m_frameIndex > last)
        {
            if (m_currentClip->loop)
            {
                m_frameIndex = 0;
            }
            else
            {
                m_frameIndex = last;
                break;
            }
        }
    }

    m_sprite->SetFrame(frames[m_frameIndex]);
}