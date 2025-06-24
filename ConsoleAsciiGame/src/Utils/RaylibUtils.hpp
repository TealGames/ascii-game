#pragma once
#include <string>
#include <cstdint>
#include "raylib.h"
#include "Utils/HelperFunctions.hpp"
#include "Utils/Data/Point4D.hpp"
#include "Utils/Data/Point3D.hpp"
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/Vec2Int.hpp"
#include "Core/PositionConversions.hpp"
#include <optional>
#include "Utils/Data/Color.hpp"

namespace RaylibUtils
{
	std::string ToString(const Color& color);
	Color MultiplyColorsRGBA(const Color& color, const Utils::Point4D& factor);
	Color MultiplyColorsRGB(const Color& color, const Utils::Point3D& factor);
	Utils::Point4D DivideColorRGBA(const Color& divided, const Color& divisor);
	Utils::Point3D DivideColorRGB(const Color& divided, const Color& divisor);

	Utils::Point4D GetFractionalColorRGBA(const Color& color, const float& multiplier);
	Utils::Point3D GetFractionalColorRGB(const Color& color, const float& multiplier);

	Color GetColorFromPoint(const Utils::Point4D& rgba);
	Color GetColorFromPoint(const Utils::Point3D& rgb);

	Color GetColorFromHex(const std::uint32_t& hexNumber);
	Color ToRaylibColor(const Utils::Color color);
	Utils::Color FromRaylibColor(const Color color);

	bool ColorEqual(const Color& color1, const Color& color2);

	void DrawFPSCounter();
	void DrawRay2D(const ScreenPosition& startPos, Vec2 dir, const Color& color);

	std::string ToString(const Vector2& vec);
	Vector2 ToRaylibVector(const Vec2& vec);
	Vector2 ToRaylibVector(const Vec2Int& point);

	bool IsValidFont(const Font& font);
	bool FontEqual(const Font& font1, const Font& font2);
	//float GetMaxFontSizeForSpace(const Font& font, const std::string& text, const Vec2& space, const float& spacing);
	void RemoveFontExtraSpacing(Font& font);
	bool FontSupportsChar(const Font& font, const char& character);

	std::optional<KeyboardKey> TryStringToKeyboardKey(const std::string& str);
	std::optional<MouseButton> TryStringToMouseButton(const std::string& str);
	std::optional<GamepadButton> TryStringToGamepadButton(const std::string& str);

	KeyboardKey StringToKeyboardKey(const std::string& str);
	MouseButton StringToMouseButton(const std::string& str);
	GamepadButton StringToGamepadButton(const std::string& str);

	std::string KeyboardKeyToString(const KeyboardKey& key);
	std::string MouseButtonToString(const MouseButton& button);
	std::string GamepadButtonToString(const GamepadButton& button);
}

