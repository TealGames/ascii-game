#pragma once
#include <optional>
#include <string>
#include "Core/Primitives/Vector.hpp"
#include "Core/Primitives/Color.hpp"
#include "Core/Rendering/Font.hpp"

namespace Engine::Serialization
{
	std::optional<Vec2> TryGetConstantPivot(const std::string& constant);
	std::optional<std::string> TryGetPivotConstant(const Vec2& vec);

	std::optional<ColHDR4> TryGetConstantColor(const std::string& constant);
	std::optional<std::string> TryGetColorConstant(const ColHDR4& color);

	std::optional<Rendering::Font> TryGetConstantFont(const std::string& constant);
	std::optional<std::string> TryGetFontConstant(const Rendering::Font& font);

	std::optional<Vec2> TryGetConstantFontSize(const std::string& constant);
	std::optional<std::string> TryGetFontSizeConstant(const Vec2& size);
}


