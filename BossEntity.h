#pragma once
#include "EnemyEntity.h"

class HPComponent;
class GravityComponent;

class BossEntity : public EnemyEntity
{
public:
	BossEntity(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 90, 196 });
    virtual bool Init() override;
    virtual void Update(float deltaTime) override;

    HPComponent* GetHP() const { return m_hp; }
    int GetMaxHP() const override { return 500; }
protected:

    virtual void UpdateAI(float deltaTime) = 0;
    virtual void UpdateAttack(float deltaTime) = 0;

};

