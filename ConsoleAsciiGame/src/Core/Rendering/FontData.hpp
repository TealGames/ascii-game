#pragma once
#include "raylib.h"

class FontAsset;
class FontProperties
{
private:
public:
	float m_Size;
	/// <summary>
	/// The horizontal space that is between all characters
	/// </summary>
	float m_Tracking;
	const FontAsset* m_FontAsset;

private:
	FontProperties(const float fontSize, const float spacing, const FontAsset* font);
public:
	FontProperties();
	FontProperties(const float fontSize, const float spacing, const FontAsset& font);

	bool HasValidFont() const;
};
