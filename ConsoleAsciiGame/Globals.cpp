#include "pch.hpp"
#include "Globals.hpp"

Font& GetGlobalFont()
{
	static Font font = GetFontDefault();
	return font;
}