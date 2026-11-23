#include "RoofTileBlock.h"

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

std::string RoofTileBlock_02::GetTexturePath() const
{
	return TEXTURE_02;
}