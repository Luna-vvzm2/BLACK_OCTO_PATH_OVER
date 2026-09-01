#include "HitEffect.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include <iostream>

HitEffect::HitEffect(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : EffectActor(scene, pos, size)
    , m_anim(nullptr)
    , m_lifeTime(0.4f)    // 0.25秒で消えるエフェクト
    , m_timer(0.0f)
{
}

bool HitEffect::Init()
{
    if (!EffectActor::Init())return false;


    std::cout << "Spawned HitEffect at: " << m_initialPos.x << ", " << m_initialPos.y << std::endl;
    m_transform->SetScale({ 3.0f, 3.0f });

    m_sprite->LoadTextureDiv("assets/images/effects/hit.png", 4, 1);
    m_sprite->SetSize(32, 32);

    m_anim = AddComponent<AnimationComponent>();
    m_anim->SetSprite(m_sprite);

    AnimationClip hit;
    hit.frames = { 0,1,2,3 };
    hit.speed = 0.1f;
    hit.loop = false;
    m_anim->AddClip("hit", hit);


    m_anim->Play("hit");

    std::cout << "Effect pos = " << GetPos().x << ", " << GetPos().y << std::endl;
    std::cout << "Sprite size = " << m_sprite->GetWidth() << ", " << m_sprite->GetHeight() << std::endl;
    std::cout << "Scale = " << m_transform->GetScale().x << ", " << m_transform->GetScale().y << std::endl;
    return true;
}

void HitEffect::Update(float deltaTime)
{
    m_timer += deltaTime;
    if (m_timer >= m_lifeTime) {
        SetState(State::Dead);
        std::cout << "エフェクト削除 " << std::endl;
    }


    if (m_anim) {

        m_anim->Update(deltaTime);
    }
}


