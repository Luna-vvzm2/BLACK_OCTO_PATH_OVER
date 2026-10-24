#pragma once
#include "EntityActor.h"
#include "Vector2d.h"
#include "TrapEffect.h"

class CollisionComponent;
class PlayerEntity;

// ダメージ罠: プレイヤーが踏むと爆発してダメージ(15)を与え、爆発演出のあと消える。
class DamageTrap : public EntityActor {
public:
    DamageTrap(class Scene* scene, const Vector2d& pos, const Vector2d& size);
    ~DamageTrap() override = default;

    bool Init() override;
    void Update(float deltaTime) override;

    ActorType GetType() const override { return ActorType::Trap; }
    std::string GetTexturePath() const override { return "assets/images/blocks/burned1.png"; }

private:
    void Explode(PlayerEntity* player);

    int m_damage;              // プレイヤーに与えるダメージ
    bool m_exploded;           // 爆発済みか
    float m_explosionTimer;    // 爆発してからの経過時間
    TrapEffect m_effect;
};
