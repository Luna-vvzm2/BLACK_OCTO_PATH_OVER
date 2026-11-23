#include "ToraByoubuBlock.h"

namespace
{
	const char* TEXTURE =
		"assets/images/blocks/tora_byoubu.png";
}

std::string ToraByoubuBlock::GetTexturePath() const
{
	return TEXTURE;
}
