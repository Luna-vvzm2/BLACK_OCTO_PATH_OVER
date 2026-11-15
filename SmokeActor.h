#pragma once

#include "Actor.h"
#include "Vector2d.h"

class TransformComponent;
class SpriteComponent;
class AnimationComponent;

class SmokeActor : public Actor
{
public:
    SmokeActor(
        Scene* scene,
        const Vector2d& position,
        const Vector2d& size
    );

    bool Init() override;
    void Update(float deltaTime) override;

    ActorType GetType() const override
    {
        return ActorType::Smoke;
    }

    bool Contains(const Vector2d& position) const;
    bool IsSmokeActive() const;

    Vector2d GetPosition() const;
    Vector2d GetSize() const;

private:
    Vector2d m_position;
    TransformComponent* m_transform;
    Vector2d m_size;
    SpriteComponent* m_sprite;
    AnimationComponent* m_anim;

    float m_elapsedTime;
    bool m_expiredNotified;
    int m_state;
};