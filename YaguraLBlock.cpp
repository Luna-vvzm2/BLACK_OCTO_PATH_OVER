#include "YaguraLBlock.h"
#include "SpriteComponent.h"

YaguraLBlock::YaguraLBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
    : BlockActor(scene, pos, colSize, texSize)
{
}


bool YaguraLBlock::Init()
{
    if (!BlockActor::Init()) return false;

    m_sprite->SetDrawOffset(-5.0f, -10.0f);

    return true;
}
