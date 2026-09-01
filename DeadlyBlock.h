#pragma once
#include "BlockActor.h"

class DeadlyBlock : public BlockActor
{
public:
	explicit DeadlyBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& size = { 32,32 });
	virtual ~DeadlyBlock() override = default;

	bool Init() override;

private:
	virtual std::string GetTexturePath() const override;
};
