#include "StoneSBlock.h"
#include "SpriteComponent.h"

StoneSBlock::StoneSBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize , texSize)
{
}

bool StoneSBlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, 0.0f);

	return true;
}