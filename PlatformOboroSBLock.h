#pragma once
#include "BlockActor.h"

class PlatformOboroSBlock : public BlockActor
{
public:
	explicit PlatformOboroSBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 208,20 });
	~PlatformOboroSBlock() override = default;

	bool Init() override;

	BlockType GetBlockType() const override { return BlockType::Platform; }
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/platformOboroSmall.png";
	}
};