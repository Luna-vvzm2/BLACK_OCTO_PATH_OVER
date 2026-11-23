#pragma once
#include "BlockActor.h"
#include "HideBlock.h"

class HouseBlock : public BlockActor {
public:
	explicit HouseBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 104,104 });
	~HouseBlock() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override {
		return "assets/images/blocks/house.png";
	}
};

class HouseBlock_Hide_01 : public HideBlock
{
public:
	using HideBlock::HideBlock;

private:
	std::string GetTexturePath() const override;
};

class HouseBlock_Hide_02 : public HideBlock
{
public:
	using HideBlock::HideBlock;

private:
	std::string GetTexturePath() const override;
};