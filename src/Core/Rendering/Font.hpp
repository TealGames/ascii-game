#pragma once
#include <string>
#include "Utils/Data/Vec2.hpp"

namespace Rendering
{
	class Font
	{
	private:
	public:

	private:
	public:
		Font();
	};

	float GetBestFontSize(const Font& font, const float spacing, const Vec2& area, const std::string& text);
}