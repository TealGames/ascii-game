#pragma once
#include <string>
#include "Core/Primitives/Vector.hpp"

namespace Engine::Rendering
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