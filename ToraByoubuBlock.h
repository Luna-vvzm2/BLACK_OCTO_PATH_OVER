#pragma once

#include "EntityActor.h"
#include "Vector2d.h"
#include "HideBlock.h"

class ToraByoubuBlock : public HideBlock
{
public:
	using HideBlock::HideBlock;

private:
	std::string GetTexturePath() const override;
};

