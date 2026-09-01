#include "StructureABlock.h"
#include "SpriteComponent.h"

StructureABlock::StructureABlock(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool StructureABlock::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -285.0f);

	return true;
}