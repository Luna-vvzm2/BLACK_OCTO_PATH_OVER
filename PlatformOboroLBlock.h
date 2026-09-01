#pragma once
#include "BlockActor.h"

class PlatformOboroLBlock : public BlockActor
{
public:
	explicit PlatformOboroLBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 208,20 });
	~PlatformOboroLBlock() override = default;

	bool Init() override;

	BlockType GetBlockType() const override { return BlockType::Platform; }
protected:
	std::string GetTexturePath() const override 
	{
		return "assets/images/blocks/platformOboroBig.png";
	}
};
