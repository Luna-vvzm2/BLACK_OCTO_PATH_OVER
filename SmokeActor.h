#pragma once

#include "Actor.h"
#include "Vector2d.h"

class SmokeActor : public Actor
{
public:
    SmokeActor(
        Scene* scene,
        const Vector2d& position,
        const Vector2d& size
    );

    void Update(float deltaTime) override;
    void Draw() override;

    ActorType GetType() const override
    {
        return ActorType::Effect;
    }

    bool Contains(const Vector2d& position) const;
    bool IsSmokeActive() const;

    Vector2d GetPosition() const;
    Vector2d GetSize() const;

private:
    Vector2d m_position;
    Vector2d m_size;

    float m_elapsedTime;
    bool m_expiredNotified;
};