#pragma once
#include "BlockActor.h"
#include "Vector2d.h"

class CollisionComponent;

class StageExitActor : public BlockActor
{
public:
    StageExitActor(Scene* scene, const Vector2d& pos, int nextStage);

    bool Init() override;

    int GetNextStage() const { return m_nextStage; }
    ActorType GetType() const override { return ActorType::StageExit; }
    BlockType GetBlockType() const { return BlockType::StageExit; }
private:
    int m_nextStage = 0;
    std::string GetTexturePath() const override { return ""; }
};