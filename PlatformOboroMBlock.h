#pragma once
#include "BlockActor.h"

class PlatformOboroMBlock : public BlockActor
{
public:
	explicit PlatformOboroMBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 208,20 });
	~PlatformOboroMBlock() override = default;

	bool Init() override;

	BlockType GetBlockType() const override { return BlockType::Platform; }
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/platformOboroMid.png";
	}
};