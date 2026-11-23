#include "EffectActor.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "DxLib.h"

std::unordered_map<EffectType, EffectData> EffectActor::s_effects;

EffectActor::EffectActor(Scene* scene, const Vector2d& pos, EffectType effectType, bool flipX, SpriteBlendMode blendMode)
    : Actor(scene)
    , m_effectType(effectType)
    , m_transform(nullptr)
    , m_sprite(nullptr)
    , m_anim(nullptr)
    , m_initialPos(pos)
    , m_flipX(flipX)
    , m_blendMode(blendMode)
{
}

bool EffectActor::Init() {

    m_transform = AddComponent<TransformComponent>();
    m_sprite = AddComponent<SpriteComponent>();
    m_anim = AddComponent<AnimationComponent>();

    m_transform->SetPosition(m_initialPos);
    m_sprite->SetSize(m_initialSize.x, m_initialSize.y);
    m_anim->SetSprite(m_sprite);

    const auto& data = s_effects[m_effectType];

    m_sprite->SetEffectFrames(data.handles);
    m_sprite->SetFlipX(m_flipX);
    m_sprite->SetBlendMode(m_blendMode);

    m_sprite->SetSize(data.size.x, data.size.y);
    m_anim->AddClip("effect", data.clip);
    m_anim->Play("effect");

    return true;
}

void EffectActor::Update(float deltaTime) {
    Actor::Update(deltaTime);
    if (m_followTarget)
    {
        auto transform =
            m_followTarget->GetComponent<TransformComponent>();

        if (transform)
        {
            m_transform->SetPosition(
                transform->GetPosition()
                + m_followOffset
            );
        }
    }

    if (m_anim->IsFinished())
    {
        SetState(State::Dead);
    }
    if (m_anim) {
        m_anim->Update(deltaTime);
    }
}

void EffectActor::SetPos(const Vector2d& pos) {
    if (m_transform) m_transform->SetPosition(pos);
}

Vector2d EffectActor::GetPos() const {
    return m_transform ? m_transform->GetPosition() : Vector2d::Zero();
}

void EffectActor::SetSize(const Vector2d& size) {
    if (m_sprite) m_sprite->SetSize(size.x, size.y);
}

Vector2d EffectActor::GetSize() const {
    return m_sprite ? Vector2d(m_sprite->GetWidth(), m_sprite->GetHeight()) : Vector2d::Zero();
}

static bool LoadEffectTexture(const std::string& path, int xNum, int yNum, std::vector<int>& outHandles)
{
    if (xNum <= 0 || yNum <= 0) return false;
    int total = xNum * yNum;

    int tempHandle = LoadGraph(path.c_str());

    if (tempHandle == -1)
    {
        return false;
    }

    int texW = 0;
    int texH = 0;

    GetGraphSize(tempHandle, &texW, &texH);


    int frameW = texW / xNum;
    int frameH = texH / yNum;

    std::vector<int> frames(total, -1);
    bool loaded = frameW > 0 && frameH > 0;
    for (int i = 0; loaded && i < total; ++i)
    {
        frames[i] = DerivationGraph(
            (i % xNum) * frameW, (i / xNum) * frameH,
            frameW, frameH, tempHandle);
        loaded = frames[i] != -1;
    }
    DeleteGraph(tempHandle);
    if (!loaded)
    {
        for (int handle : frames)
        {
            if (handle != -1) DeleteGraph(handle);
        }
        outHandles.clear();
        return false;
    }
    outHandles = std::move(frames);
    return true;
}

bool EffectActor::LoadEffects()
{
    std::vector<int> attackHandles;

    LoadEffectTexture("assets/images/effects/slash1_sheet.png", 3, 5, attackHandles);

    EffectData weak1;
    weak1.handles = attackHandles;
    weak1.clip.frames = { -1, -1, -1, 0, 1 , 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    weak1.clip.speed = 0.1f;
    weak1.clip.loop = false;
    weak1.size = { 64,64 };
    s_effects[EffectType::WeakAttack1] = weak1;

    LoadEffectTexture("assets/images/effects/slash2_sheet.png", 3, 5, attackHandles);

    EffectData weak2;
    weak2.handles = attackHandles;
    weak2.clip.frames = { -1, -1, -1, 0, 1 , 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    weak2.clip.speed = 0.1f;
    weak2.clip.loop = false;
    weak2.size = { 64,64 };
    s_effects[EffectType::WeakAttack2] = weak2;

    LoadEffectTexture("assets/images/effects/slash3_sheet.png", 3, 5, attackHandles);

    EffectData weak3;
    weak3.handles = attackHandles;
    weak3.clip.frames = { -1, -1, -1, 0, 1 , 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    weak3.clip.speed = 0.1f;
    weak3.clip.loop = false;
    weak3.size = { 64,64 };
    s_effects[EffectType::WeakAttack3] = weak3;

    s_effects[EffectType::WeakAirAttack1] = weak3;

    s_effects[EffectType::SquatAttack] = weak2;

    LoadEffectTexture("assets/images/effects/octo.png", 4, 1, attackHandles);

    EffectData octo;
    octo.handles = attackHandles;
    octo.clip.frames = { 0, 1, 2, 3};
    octo.clip.speed = 0.5f;
    octo.clip.loop = false;
    octo.size = { 100, 100 };
    s_effects[EffectType::Octo] = octo;

    LoadEffectTexture("assets/images/effects/TORA_sheet.png", 3, 24, attackHandles);

    EffectData tiger1;
    tiger1.handles = attackHandles;
    tiger1.clip.frames = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };
    tiger1.clip.speed = 0.07f;
    tiger1.clip.loop = false;
    tiger1.size = { 100, 100 };
    s_effects[EffectType::Tiger1] = tiger1;

    EffectData tiger2;
    tiger2.handles = attackHandles;
    tiger2.clip.frames = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71 };
    tiger2.clip.speed = 0.05f;
    tiger2.clip.loop = false;
    tiger2.size = { 100, 100 };
    s_effects[EffectType::Tiger2] = tiger2;



    return true;
}

const EffectData* EffectActor::GetEffectData(EffectType type)
{
    auto it = s_effects.find(type);

    if (it == s_effects.end())
    {
        return nullptr;
    }

    return &it->second;
}

void EffectActor::SetFollowTarget(Actor* target, const Vector2d& offset)
{
    m_followTarget = target;
    m_followOffset = offset;
}