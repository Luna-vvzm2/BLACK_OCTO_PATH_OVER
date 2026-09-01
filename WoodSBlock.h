#pragma once
#include "BlockActor.h"

class WoodSBlock : public BlockActor
{
public:
	explicit WoodSBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104, 104 }, const Vector2d& texSize = { 104, 104 });
	~WoodSBlock() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/woodBoxA.png";
	}
};