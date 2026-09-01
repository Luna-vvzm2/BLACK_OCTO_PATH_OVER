#pragma once
#include "BlockActor.h"

class ClearBlock : public BlockActor {
public:
	explicit ClearBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 104,104 });
	~ClearBlock() override = default;

protected:
	std::string GetTexturePath() const override {
		return "";
	}
};
