#include "PlatformOboroLBlock.h"
#include "SpriteComponent.h"

PlatformOboroLBlock::PlatformOboroLBlock(Scene* scene, const Vector2d& pos, const Vector2d& size)
    : BlockActor(scene, pos, size)
{
}

bool PlatformOboroLBlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, 0.0f);

	return true;
}