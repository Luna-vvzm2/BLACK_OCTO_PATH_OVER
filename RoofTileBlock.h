#pragma once

#include "EntityActor.h"
#include "Vector2d.h"
#include "HideBlock.h"

class RoofTileBlock_01 : public HideBlock
{
public:
	using HideBlock::HideBlock;
	bool Init() override;

private:
	std::string GetTexturePath() const override;
};

class RoofTileBlock_02 : public HideBlock
{
public:
	using HideBlock::HideBlock;
	bool Init() override;

private:
	std::string GetTexturePath() const override;
};
