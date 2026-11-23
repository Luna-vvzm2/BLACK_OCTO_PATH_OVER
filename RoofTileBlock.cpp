#include "RoofTileBlock.h"
#include "SpriteComponent.h"

namespace
{
	const char* TEXTURE_01 =
		"assets/images/blocks/kawara1.png";

	const char* TEXTURE_02 =
		"assets/images/blocks/kawara2.png";
}

std::string RoofTileBlock_01::GetTexturePath() const
{
	return TEXTURE_01;
}

bool RoofTileBlock_01::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -20.0f);

	return true;
}


std::string RoofTileBlock_02::GetTexturePath() const
{
	return TEXTURE_02;
}

bool RoofTileBlock_02::Init()
{
	if (!BlockActor::Init()) return false;

	m_sprite->SetDrawOffset(0.0f, -20.0f);

	return true;
}