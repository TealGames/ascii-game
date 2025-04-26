#pragma once
#include "raylib.h"

struct FontProperties
{
	float m_Size;
	/// <summary>
	/// The horizontal space that is between all characters
	/// </summary>
	float m_Tracking;
	Font m_FontType;

	FontProperties();
	FontProperties(const float fontSize, const float spacing, const Font& font);

	bool HasValidFont() const;
};
