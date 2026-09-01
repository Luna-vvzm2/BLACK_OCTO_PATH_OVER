#pragma once
#include "BlockActor.h"

class HouseBlock : public BlockActor {
public:
	explicit HouseBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 104,104 });
	~HouseBlock() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override {
		return "assets/images/blocks/house.png";
	}
};
