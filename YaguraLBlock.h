#pragma once
#include "BlockActor.h"

class YaguraLBlock : public BlockActor {
public:
	explicit YaguraLBlock(Scene* scene, const Vector2d& pos = Vector2d::Zero(), const Vector2d& colSize = { 104,104 }, const Vector2d& texSize = { 104, 104 });
	~YaguraLBlock() override = default;

	bool Init() override;
protected:
	std::string GetTexturePath() const override {
		return "assets/images/blocks/yagura3.png";
	}
};
