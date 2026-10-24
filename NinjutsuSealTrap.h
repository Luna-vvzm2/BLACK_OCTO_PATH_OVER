#pragma once
#include "EntityActor.h"
#include "Vector2d.h"
#include "TrapEffect.h"

class CollisionComponent;
class PlayerEntity;

//忍術封印トラップ
class NinjutsuSealTrap:public EntityActor
{
public:
	NinjutsuSealTrap(class Scene* scene, const Vector2d& pos, const Vector2d& size);
	~NinjutsuSealTrap() override = default;

	bool Init() override;
	void Update(float deltaTime) override;

	ActorType GetType() const override { return ActorType::Trap; }
	std::string GetTexturePath() const override { return "assets/images/blocks/burned1.png"; } //仮素材

private:
	float m_duration; //忍術が使えなくなる時間
	bool m_triggered; // 起動済みか(起動エフェクトの再生が終わるまで残る)
	TrapEffect m_effect;
};
