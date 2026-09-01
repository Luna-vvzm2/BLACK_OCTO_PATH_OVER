#pragma once
#include "BlockActor.h"

class GroundBlock : public BlockActor {
public:
	explicit GroundBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 104,104 });
	~GroundBlock() override = default;

protected:
	std::string GetTexturePath() const override {
		return "assets/images/blocks/ground.png";
	}
};

