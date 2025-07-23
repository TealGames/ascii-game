#pragma once
#include <optional>
#include <string>
//#include "raylib.h"
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/Color.hpp"
#include "Core/Rendering/Font.hpp"

namespace JsonConstants
{
	std::optional<Vec2> TryGetConstantPivot(const std::string& constant);
	std::optional<std::string> TryGetPivotConstant(const Vec2& vec);

	std::optional<Vec2> TryGetConstantDir(const std::string& constant);
	std::optional<std::string> TryGetDirConstant(const Vec2& vec);

	std::optional<Utils::Color> TryGetConstantColor(const std::string& constant);
	std::optional<std::string> TryGetColorConstant(const Utils::Color& color);

	std::optional<Rendering::Font> TryGetConstantFont(const std::string& constant);
	std::optional<std::string> TryGetFontConstant(const Rendering::Font& font);

	std::optional<Vec2> TryGetConstantFontSize(const std::string& constant);
	std::optional<std::string> TryGetFontSizeConstant(const Vec2& size);
}


