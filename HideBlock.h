    #pragma once

#include "BlockActor.h"

#include <string>

class PlayerEntity;
class Scene;

class HideBlock : public BlockActor
{
public:
    explicit HideBlock(
        Scene* scene,
        const Vector2d& pos = Vector2d::Zero(),
        const Vector2d& colSize = { 104, 104 },
        const Vector2d& texSize = { 104, 104 });
    ~HideBlock() override = default;

    bool Init() override;

    BlockType GetBlockType() const override { return BlockType::Hide; }

private:
    std::string GetTexturePath() const override;
};