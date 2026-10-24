#pragma once
#include "EntityActor.h"
#include "Vector2d.h"

class CollisionComponent;
class PlayerEntity;

//”Ep••ˆóƒgƒ‰ƒbƒv
class NinjutsuSealTrap:public EntityActor
{
public:
	NinjutsuSealTrap(class Scene* scene, const Vector2d& pos, const Vector2d& size);
	~NinjutsuSealTrap() override = default;

	bool Init() override;
	void Update(float deltaTime) override;

	ActorType GetType() const override { return ActorType::Trap; }
	std::string GetTexturePath() const override { return "assets/images/blocks/burned1.png"; } //‰¼‘fŞ

private:
	float m_duration; //”Ep‚ªg‚¦‚È‚­‚È‚éŠÔ
};
