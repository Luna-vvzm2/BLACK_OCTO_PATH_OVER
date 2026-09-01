#include "YaguraSBlock.h"
#include "SpriteComponent.h"

YaguraSBlock::YaguraSBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
    : BlockActor(scene, pos, colSize, texSize)
{
}

bool YaguraSBlock::Init()
{
    if (!BlockActor::Init()) return false;

    m_sprite->SetDrawOffset(0.0f, -25.0f);

    return true;
}
