#include "EffectActor.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "DxLib.h"

std::unordered_map<EffectType, EffectData> EffectActor::s_effects;

EffectActor::EffectActor(Scene* scene, const Vector2d& pos, EffectType effectType, bool flipX)
	: Actor(scene)
    , m_effectType(effectType)
    , m_transform(nullptr)
    , m_sprite(nullptr)
    , m_anim(nullptr)
    , m_initialPos(pos)
    , m_flipX(flipX)
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
    int total = xNum * yNum;

    int tempHandle = LoadGraph(path.c_str());

    if (tempHandle == -1)
    {
        return false;
    }

    int texW;
    int texH;

    GetGraphSize( tempHandle, &texW, &texH );

    DeleteGraph(tempHandle);

    int frameW = texW / xNum;
    int frameH = texH / yNum;

    outHandles.resize(total);

    return LoadDivGraph(path.c_str(), total, xNum, yNum, frameW, frameH, outHandles.data()) != -1;
}

bool EffectActor::LoadEffects()
{
    std::vector<int> attackHandles;

    LoadEffectTexture("assets/images/effects/EF_musashiLightAttack.png", 4, 3, attackHandles);

    EffectData weak1;
    weak1.handles = attackHandles;
    weak1.clip.frames = { -1, -1, -1, 0, 1 };
    weak1.clip.speed = 0.1f;
    weak1.clip.loop = false;
    weak1.size = {64,64};
    s_effects[EffectType::WeakAttack1] = weak1;

    EffectData weak2;
    weak2.handles = attackHandles;
    weak2.clip.frames = { -1, -1, 2, 3, 4 };
    weak2.clip.speed = 0.1f;
    weak2.clip.loop = false;
    weak2.size = { 64,64 };
    s_effects[EffectType::WeakAttack2] = weak2;

    EffectData weak3;
    weak3.handles = attackHandles;
    weak3.clip.frames = { -1, -1, 5, 6, 7 };
    weak3.clip.speed = 0.1f;
    weak3.clip.loop = false;
    weak3.size = { 64,64 };
    s_effects[EffectType::WeakAttack3] = weak3;

    EffectData weak4;
    weak4.handles = attackHandles;
    weak4.clip.frames = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 9, 10 };
    weak4.clip.speed = 0.1f;
    weak4.clip.loop = false;
    weak4.size = { 64,64 };
    s_effects[EffectType::WeakAttack4] = weak4;


    LoadEffectTexture("assets/images/effects/EF_musashiHeavyAttack.png", 4, 3, attackHandles);

    EffectData strong1;
    strong1.handles = attackHandles;
    strong1.clip.frames = { -1, -1, -1, -1, -1, 0, 1, 2, 3 };
    strong1.clip.speed = 0.1f;
    strong1.clip.loop = false;
    strong1.size = { 64,64 };
    s_effects[EffectType::StrongAttack1] = strong1;

    EffectData strong2;
    strong2.handles = attackHandles;
    strong2.clip.frames = { -1, -1, -1, 4, 5, 6, 7, 8, 9, 10 };
    strong2.clip.speed = 0.1f;
    strong2.clip.loop = false;
    strong2.size = { 64,64 };
    s_effects[EffectType::StrongAttack2] = strong2;


    LoadEffectTexture("assets/images/effects/EF_musahiAirAttack.png", 4, 4, attackHandles);

    EffectData weakAir1;
    weakAir1.handles = attackHandles;
    weakAir1.clip.frames = { -1, -1, 0, 1, 2 };
    weakAir1.clip.speed = 0.1f;
    weakAir1.clip.loop = false;
    weakAir1.size = { 64,64 };
    s_effects[EffectType::WeakAirAttack1] = weakAir1;

    EffectData weakAir2;
    weakAir2.handles = attackHandles;
    weakAir2.clip.frames = { -1, -1, 3, 4, 5 };
    weakAir2.clip.speed = 0.1f;
    weakAir2.clip.loop = false;
    weakAir2.size = { 64,64 };
    s_effects[EffectType::WeakAirAttack2] = weakAir2;

    EffectData weakAir3;
    weakAir3.handles = attackHandles;
    weakAir3.clip.frames = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    weakAir3.clip.speed = 0.1f;
    weakAir3.clip.loop = false;
    weakAir3.size = { 64,64 };
    s_effects[EffectType::WeakAirAttack3] = weakAir3;


    LoadEffectTexture("assets/images/effects/EF_musashiKick.png", 4, 2, attackHandles);

    EffectData Hayabusa;
    Hayabusa.handles = attackHandles;
    Hayabusa.clip.frames = { -1, -1, 0, 1, 2, 3, 4 };
    Hayabusa.clip.speed = 0.1f;
    Hayabusa.clip.loop = false;
    Hayabusa.size = { 64,64 };
    s_effects[EffectType::Hayabusa] = Hayabusa;

    LoadEffectTexture("assets/images/effects/EF_musashiCrouchAttack.png", 4, 1, attackHandles);

    EffectData squat;
    squat.handles = attackHandles;
    squat.clip.frames = { -1, -1, -1, 0, 1, 2 };
    squat.clip.speed = 0.1f;
    squat.clip.loop = false;
    squat.size = { 64,64 };
    s_effects[EffectType::SquatAttack] = squat;

    LoadEffectTexture("assets/images/effects/EF_musashiExecution.png", 4, 5, attackHandles);

    EffectData executionStart;
    executionStart.handles = attackHandles;
    executionStart.clip.frames = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    executionStart.clip.speed = 0.1f;
    executionStart.clip.loop = false;
    executionStart.size = { 64,64 };
    s_effects[EffectType::ExecutionStart] = executionStart;

    EffectData executionMove;
    executionMove.handles = attackHandles;
    executionMove.clip.frames = { 11, 12, 13 };
    executionMove.clip.speed = 0.1f;
    executionMove.clip.loop = false;
    executionMove.size = { 64,64 };
    s_effects[EffectType::ExecutionMove] = executionMove;

    EffectData executionEnemy;
    executionEnemy.handles = attackHandles;
    executionEnemy.clip.frames = { 14, 15, 16, 17, 18 };
    executionEnemy.clip.speed = 0.1f;
    executionEnemy.clip.loop = false;
    executionEnemy.size = { 64,64 };
    s_effects[EffectType::ExecutionEnemy] = executionEnemy;

    return true;
}

const EffectData* EffectActor::GetEffectData( EffectType type)
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