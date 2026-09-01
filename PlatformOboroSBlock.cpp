#include "PlatformOboroSBlock.h"
#include "SpriteComponent.h"

PlatformOboroSBlock::PlatformOboroSBlock(Scene* scene, const Vector2d& pos, const Vector2d& size)
	: BlockActor(scene, pos, size)
{
}

bool PlatformOboroSBlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, 0.0f);

	return true;
}