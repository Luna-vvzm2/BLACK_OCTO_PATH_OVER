#pragma once
#include "Actor.h"
#include "Vector2d.h"
#include <DxLib.h>

class TransformComponent;
class VelocityComponent;
class SpriteComponent;
class CollisionComponent;



class EntityActor : public Actor {
public:

    enum class ActionState {
        IDLE,
        RUN,
        SQUAT,

        JUMP,
        FALL,

        ATTACK,
        KUNAI,

        HIT,
        ROLL,

        CHASE,
        DEAD,
    };

    explicit EntityActor(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32, 32 });
    virtual ~EntityActor() = default;

    bool Init() override;

    void SetPos(const Vector2d& pos);
    Vector2d GetPos() const;

    void SetVel(const Vector2d& vel);
    Vector2d GetVel() const;

    void SetSize(const Vector2d& size);
    Vector2d GetSize() const;

    virtual bool IgnorePlatform() const{ return false; }

    CollisionComponent* GetCollision() const { return m_collision; }
    TransformComponent* GetTransform() const { return m_transform; }
    Scene* GetScene() const { return m_scene; }
   

    bool IsDead() const
    {
        return GetState() == State::Dead;
    }
protected:

    TransformComponent* m_transform;
    VelocityComponent* m_velocity;
    SpriteComponent* m_sprite;
    CollisionComponent* m_collision;

    Vector2d m_initialPos;
    Vector2d m_initialVel;
    Vector2d m_initialSize;

    ActionState m_state;
    bool m_isGround;
    //’Ç‰Á
    void Destroy()
    {
        SetState(State::Dead);
    }

    void MoveAndCollide(float deltaTime);
    //’Ç‰Á
    virtual std::string GetTexturePath() const = 0;
};