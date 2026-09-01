#include "HouseBlock.h"
#include "SpriteComponent.h"

HouseBlock::HouseBlock(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : BlockActor(scene, pos, size)
{
}

bool HouseBlock::Init()
{
    if (!BlockActor::Init()) return false;

    m_sprite->SetDrawOffset(-610.0f, -350.0f);

    return true;
}