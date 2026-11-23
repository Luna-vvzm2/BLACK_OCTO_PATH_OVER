#include "WoodBlock.h"
#include "SpriteComponent.h"

WoodSBlock_01::WoodSBlock_01(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool WoodSBlock_01::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -50.0f);

	return true;
}

WoodSBlock_02::WoodSBlock_02(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool WoodSBlock_02::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -50.0f);

	return true;
}

WoodSBlock_03::WoodSBlock_03(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool WoodSBlock_03::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -50.0f);

	return true;
}

WoodSBlock_04::WoodSBlock_04(Scene* scene, const Vector2d& pos, const Vector2d& colSize, const Vector2d& texSize)
	: BlockActor(scene, pos, colSize, texSize)
{
}

bool WoodSBlock_04::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -50.0f);

	return true;
}