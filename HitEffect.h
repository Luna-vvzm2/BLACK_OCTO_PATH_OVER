#pragma once
#include "EffectActor.h"
#include "Vector2d.h"

class TransformComponent;
class SpriteComponent;
class AnimationComponent;

class HitEffect : public EffectActor {

public:
    explicit HitEffect(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32, 32 });
    ~HitEffect() override = default;

    bool Init() override;
    void Update(float deltaTime) override;

    ActorType GetType() const override { return ActorType::Effect; }

private:

    AnimationComponent* m_anim;

    float m_lifeTime;   // エフェクトの寿命
    float m_timer;      // 経過時間



};