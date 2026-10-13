#pragma once
#include "EntityActor.h"
#include "Vector2d.h"

class CollisionComponent;
class PlayerEntity;

// ステータスダウン罠: プレイヤーが踏むと攻撃力・防御力が半減(15秒)し、罠は消える。
class StatusDownTrap : public EntityActor {
public:
    StatusDownTrap(class Scene* scene, const Vector2d& pos, const Vector2d& size);
    ~StatusDownTrap() override = default;

    bool Init() override;
    void Update(float deltaTime) override;

    ActorType GetType() const override { return ActorType::Trap; }
    std::string GetTexturePath() const override { return "assets/images/blocks/burned1.png"; }

private:
    float m_duration;   // ステータスダウンの効果時間(秒)
};
