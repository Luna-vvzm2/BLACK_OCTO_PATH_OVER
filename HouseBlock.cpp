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

namespace
{
	const char* TEXTURE_01 =
		"assets/images/blocks/home.png";

	const char* TEXTURE_02 =
		"assets/images/blocks/home2.png";
}

std::string HouseBlock_Hide_01::GetTexturePath() const
{
	return TEXTURE_01;
}

std::string HouseBlock_Hide_02::GetTexturePath() const
{
	return TEXTURE_02;
}