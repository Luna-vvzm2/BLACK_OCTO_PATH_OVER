#include "WoodLBlock.h"
#include "SpriteComponent.h"

WoodLBlock::WoodLBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool WoodLBlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, 0.0f);

	return true;
}