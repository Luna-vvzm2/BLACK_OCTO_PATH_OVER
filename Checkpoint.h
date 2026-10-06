#pragma once

#include "Actor.h"
#include "Vector2d.h"

class SpriteComponent;

class Checkpoint : public Actor
{
public:
    Checkpoint(Scene* scene, const Vector2d& position);

    bool Init() override;
    void Update(float deltaTime) override;
    ActorType GetType() const override { return ActorType::Block; }

private:
    Vector2d m_position{};
    SpriteComponent* m_sprite{ nullptr };
    bool m_isActivated{ false };
};
