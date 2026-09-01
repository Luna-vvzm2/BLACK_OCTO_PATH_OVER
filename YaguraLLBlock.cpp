#include "YaguraLLBlock.h"
#include "SpriteComponent.h"

YaguraLLBlock::YaguraLLBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
    : BlockActor(scene, pos, colSize, texSize)
{
}


bool YaguraLLBlock::Init()
{
    if (!BlockActor::Init()) return false;

    m_sprite->SetDrawOffset(-45.0f, -20.0f);

    return true;
}
