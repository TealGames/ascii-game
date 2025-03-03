#pragma once
#include <string>
#include <format>
#include <cstdint>
#include <limits>
#include "raylib.h"
#include "HelperFunctions.hpp"
#include "Point4D.hpp"
#include "Point3D.hpp"
#include "Vec2.hpp"
#include <optional>

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

	bool ColorEqual(const Color& color1, const Color& color2);

	void DrawFPSCounter();

	std::string ToString(const Vector2& vec);
	Vector2 ToRaylibVector(const Vec2& vec);
	Vector2 ToRaylibVector(const Utils::Point2D& point);
	Vector2 ToRaylibVector(const Utils::Point2DInt& point);

	bool IsValidFont(const Font& font);
	float GetMaxFontSizeForSpace(const Font& font, const std::string& text, const Utils::Point2D& space, const float& spacing);
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

