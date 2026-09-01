#pragma once

#include "BlockActor.h"

class StageBackActor : public BlockActor
{
public:
    StageBackActor(Scene* scene,
        const Vector2d& pos,
        int prevStage,
        int spawnIndex);

    bool Init() override;

    ActorType GetType() const override { return ActorType::StageBack; }
    BlockType GetBlockType() const override { return BlockType::StageExit; }

    int GetPrevStage() const { return m_prevStage; }
    int GetSpawnIndex() const { return m_spawnIndex; }

protected:
    std::string GetTexturePath() const override { return ""; }

private:
    int m_prevStage;
    int m_spawnIndex;
};