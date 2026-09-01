#include "PlatformOboroMBlock.h"
#include "SpriteComponent.h"

PlatformOboroMBlock::PlatformOboroMBlock(Scene* scene, const Vector2d& pos, const Vector2d& size)
	: BlockActor(scene, pos, size)
{
}

bool PlatformOboroMBlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, 0.0f);

	return true;
}