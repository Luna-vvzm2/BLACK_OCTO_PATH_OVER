#include "StructureBBlock.h"
#include "SpriteComponent.h"

StructureBBlock::StructureBBlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool StructureBBlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -285.0f);

	return true;
}