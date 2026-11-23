#pragma once
#include "BlockActor.h"

class WoodSBlock_01 : public BlockActor
{
public:
	explicit WoodSBlock_01(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104, 104 }, const Vector2d& texSize = { 104, 104 });
	~WoodSBlock_01() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/wood1.png";
	}
};

class WoodSBlock_02 : public BlockActor
{
public:
	explicit WoodSBlock_02(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104, 104 }, const Vector2d& texSize = { 104, 104 });
	~WoodSBlock_02() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/wood2.png";
	}
};

class WoodSBlock_03 : public BlockActor
{
public:
	explicit WoodSBlock_03(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104, 104 }, const Vector2d& texSize = { 104, 104 });
	~WoodSBlock_03() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/wood3.png";
	}
};

class WoodSBlock_04 : public BlockActor
{
public:
	explicit WoodSBlock_04(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104, 104 }, const Vector2d& texSize = { 104, 104 });
	~WoodSBlock_04() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override
	{
		return "assets/images/blocks/wood4.png";
	}
};
