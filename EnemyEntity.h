#pragma once
#include "EntityActor.h"
#include "Vector2d.h"
#include "DropData.h"
#include <vector>

class TransformComponent;
class VelocityComponent;
class GravityComponent;
class SpriteComponent;
class CollisionComponent;
class AnimationComponent;
class HPComponent;


class EnemyEntity : public EntityActor {


public:
	explicit EnemyEntity(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 90, 196 });
	~EnemyEntity() override = default;

	virtual bool Init() override;
	virtual void Update(float deltaTime) override;
    virtual void UpdateAI(){}
    virtual void OnDead();
	void UpdateMove(float deltaTime);
	void UpdateGravity(float deltaTime);
	void UpdateAttack(float deltaTime);
	void UpdateState();


    ActorType GetType() const override { return ActorType::Enemy; }
    CollisionComponent* GetCollision() const { return m_collision; }

    bool OnGround() const { return m_isGround; }
    virtual void SetHP(){}
    void SetCanMove(bool canMove) { m_canMove = canMove; }

    HPComponent* GetHP() const { return m_hp; }
    virtual int GetMaxHP() const { return m_hpMax;  }
    virtual void TakeDamage(int damage, const Vector2d& knockback);
    virtual void TakeMetsu(int metsu);
    bool IsMetsu() { return m_metsu; }
    void MetsuAttacked();
    int GetMetsuGauge() const { return m_metsuGauge; }
    int GetMetsuMax() const { return m_metsuMax; }

protected:
    virtual void OnDamaged(int damage, const Vector2d& knockback) {}
    virtual void OnDeadFromDamage(int damage, const Vector2d& knockback);

    HPComponent* m_hp;
    int m_hpMax;
    GravityComponent* m_gravity;
    //SpriteComponent* m_sprite;
    AnimationComponent* m_anim;

    bool m_dir;
    float m_jumpSpeed;
    float m_moveSpeed;

    int  m_guard;
    int  m_guardMax;

    int  m_metsuGauge;
    int  m_metsuMax;
    bool m_metsu;

    bool m_damageCancel;
    bool m_attackActive;

    bool m_findPlayer;

    bool m_attack;
    int m_attackType;
    float m_attackTimer;

    float m_actionTimer;
    float m_cooldownTimer;
    bool m_actionLock;

    bool m_canMove;       // ˆÚ“®‰Â”Û
    //’Ç‰Á
    std::vector<DropData> m_dropTable;
    void SpawnItem(ItemType type);
    virtual std::string GetTexturePath() const override;

};


