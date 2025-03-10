#pragma once
#include <optional>
#include <string>
#include "raylib.h"
#include "Vec2.hpp"

namespace JsonConstants
{
	std::optional<Vec2> TryGetConstantPivot(const std::string& constant);
	std::optional<std::string> TryGetPivotConstant(const Vec2& vec);

	std::optional<Vec2> TryGetConstantDir(const std::string& constant);
	std::optional<std::string> TryGetDirConstant(const Vec2& vec);

	std::optional<Color> TryGetConstantColor(const std::string& constant);
	std::optional<std::string> TryGetColorConstant(const Color& color);

	std::optional<Font> TryGetConstantFont(const std::string& constant);
	std::optional<std::string> TryGetFontConstant(const Font& font);

	std::optional<float> TryGetConstantFontSize(const std::string& constant);
	std::optional<std::string> TryGetFontSizeConstant(const float& size);
}


