#pragma once
//#include "raylib.h"
#include "Utils/Data/Vec2.hpp"

class FontAsset;

/// <summary>
/// The font properties for screen rendering use 
/// (since font size is based on screen pixel size relative to window)
/// </summary>
class ScreenFontProperties
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
	ScreenFontProperties(const float fontSize, const float spacing, const FontAsset* font);
public:
	ScreenFontProperties();
	ScreenFontProperties(const float fontSize, const float spacing, const FontAsset& font);

	bool HasValidFont() const;
};

class WorldFontProperties
{
private:
public:
	/// <summary>
	/// The area that the text should fit in
	/// </summary>
	Vec2 m_RectSize;
	/// <summary>
	/// 
	/// The horizontal space that is between all characters
	/// </summary>
	float m_Tracking;
	const FontAsset* m_FontAsset;

private:
	WorldFontProperties(const Vec2& rectArea, const float spacing, const FontAsset* font);
public:
	WorldFontProperties();
	WorldFontProperties(const Vec2& rectArea, const float spacing, const FontAsset& font);

	bool HasValidFont() const;
};