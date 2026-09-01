#include "YaguraMBlock.h"
#include "SpriteComponent.h"

YaguraMBlock::YaguraMBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
    : BlockActor(scene, pos, colSize, texSize)
{
}


bool YaguraMBlock::Init()
{
    if (!BlockActor::Init()) return false;

    m_sprite->SetDrawOffset(-5.0f, -20.0f);

    return true;
}
