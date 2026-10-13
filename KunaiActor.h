#pragma once
#include "Actor.h"
#include "Vector2d.h"

class PlayerEntity;
class TransformComponent;
class VelocityComponent;
class GravityComponent;
class SpriteComponent;
class CollisionComponent;
class AnimationComponent;

class KunaiActor : public Actor
{
public:
    explicit KunaiActor(
        Scene* scene,
        PlayerEntity* player,
        const Vector2d& pos,
        bool dir
    );

    bool Init() override;
    void Update(float deltaTime) override;

    ActorType GetType() const override
    {
        return ActorType::Kunai;
    }

    void CheckBlockCollision(Actor* actor);
    void OnBlockHit();

private:
    PlayerEntity* m_player;

    TransformComponent* m_transform;
    VelocityComponent* m_velocity;
    CollisionComponent* m_collision;
    SpriteComponent* m_sprite;

    float m_lifeTimer;
    Vector2d m_spawnPos;

    bool m_dir;
    bool m_stopped;
};
