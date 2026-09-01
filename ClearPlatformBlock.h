#pragma once
#include "BlockActor.h"

class ClearPlatformBlock : public BlockActor {
public:
	explicit ClearPlatformBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 208,20 });
	~ClearPlatformBlock() override = default;

	BlockType GetBlockType() const override { return BlockType::Platform; }
protected:
	std::string GetTexturePath() const override 
	{
		return "";
	}
};