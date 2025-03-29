#pragma once
#include "raylib.h"

struct FontData
{
	float m_FontSize;
	Font m_Font;

	FontData();
	FontData(const float& fontSize, const Font& font);
};
