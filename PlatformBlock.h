#pragma once
#include "BlockActor.h"

class PlatformBlock : public BlockActor {
public:
	explicit PlatformBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 208,20 });
	~PlatformBlock() override = default;

	BlockType GetBlockType() const override { return BlockType::Platform; }
protected:
	std::string GetTexturePath() const override {
		return "assets/images/blocks/platformTikurin.png";
	}
};
