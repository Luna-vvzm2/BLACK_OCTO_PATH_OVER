#pragma once
#include <DxLib.h>

struct Color {
	UINT8 r, g, b, a = 255;

	Color(UINT8 r = 255, UINT8 g = 255, UINT8 b = 255, UINT8 a = 255)
		: r(r), g(g), b(b), a(a)
	{
	}

	//	DxLib—p‚ÌCOLORREF•ÏŠ·
	unsigned int ToDxColor() const { return GetColor(r, g, b); }
};