#pragma once

#include "EntityActor.h"
#include "Vector2d.h"

class SpriteComponent;

class Checkpoint : public EntityActor
{
public:
    Checkpoint(Scene* scene, const Vector2d& pos);

    bool Init() override;
    void Update(float deltaTime) override;

    ActorType GetType() const override { return ActorType::CheckPoint; }
    std::string GetTexturePath() const override { return "assets/images/blocks/checkpointPassive.png";}

private:
    bool m_isActivated{ false };
};
