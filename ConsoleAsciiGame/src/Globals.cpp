#include "pch.hpp"
#include "Globals.hpp"
#include "RaylibUtils.hpp"

Font GetGlobalFont()
{
	return GetFontDefault();
	/*static bool removedSpacing = false;
	if (!removedSpacing)
	{
		RaylibUtils::RemoveFontExtraSpacing(font);
		removedSpacing = true;
	}*/

	//return font;
}