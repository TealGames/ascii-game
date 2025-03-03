#include "pch.hpp"
#include "raylib.h"
#include "RaylibUtils.hpp"
#include "Debug.hpp"
#include <limits>

namespace RaylibUtils
{
	static std::unordered_map<std::string, KeyboardKey> KeyboardStrings = 
	{
		//Letters
		{"A", KEY_A}, {"B", KEY_B}, {"C", KEY_C}, {"D", KEY_D}, {"E", KEY_E},
		{"F", KEY_F}, {"G", KEY_G}, {"H", KEY_H}, {"I", KEY_I}, {"J", KEY_J},
		{"K", KEY_K}, {"L", KEY_L}, {"M", KEY_M}, {"N", KEY_N}, {"O", KEY_O},
		{"P", KEY_P}, {"Q", KEY_Q}, {"R", KEY_R}, {"S", KEY_S}, {"T", KEY_T},
		{"U", KEY_U}, {"V", KEY_V}, {"W", KEY_W}, {"X", KEY_X}, {"Y", KEY_Y},
		{"Z", KEY_Z}, 

		//Nnumbers
		{"Zero", KEY_ZERO}, {"One", KEY_ONE}, {"Two", KEY_TWO}, {"Three", KEY_THREE}, {"Four", KEY_FOUR},
		{"Five", KEY_FIVE}, {"Six", KEY_SIX}, {"Seven", KEY_SEVEN}, {"Eight", KEY_EIGHT}, {"Nine", KEY_NINE},

		//Modifier Keys
		{"Space", KEY_SPACE}, {"Enter", KEY_ENTER}, {"Escape", KEY_ESCAPE},
		{"Tab", KEY_TAB}, {"Backspace", KEY_BACKSPACE}, {"Insert", KEY_INSERT},
		{"Delete", KEY_DELETE},{"ArrowLeft", KEY_LEFT}, {"ArrowRight", KEY_RIGHT},
		{"ArrowUp", KEY_UP}, {"ArrowDown", KEY_DOWN}, {"PageUp", KEY_PAGE_UP},
		{"PageDown", KEY_PAGE_DOWN}, {"Home", KEY_HOME}, {"End", KEY_END},
		{"CapsLock", KEY_CAPS_LOCK}, {"ScrollLock", KEY_SCROLL_LOCK},
		{"NumLock", KEY_NUM_LOCK}, {"PrintScreen", KEY_PRINT_SCREEN},
		{"Pause", KEY_PAUSE}, {"LeftShift", KEY_LEFT_SHIFT}, {"LeftCtrl", KEY_LEFT_CONTROL},
		{"LeftAlt", KEY_LEFT_ALT}, {"LeftSuper", KEY_LEFT_SUPER}, {"RightShift", KEY_RIGHT_SHIFT},
		{"RightCtrl", KEY_RIGHT_CONTROL}, {"RightAlt", KEY_RIGHT_ALT}, 
		{"RightSuper", KEY_RIGHT_SUPER}, {"KBMenu", KEY_KB_MENU},

		//Function Keys
		{"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3},
		{"F4", KEY_F4}, {"F5", KEY_F5}, {"F6", KEY_F6},
		{"F7", KEY_F7}, {"F8", KEY_F8}, {"F9", KEY_F9},
		{"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},
		
		//Special symbols
		{"Apostrophe", KEY_APOSTROPHE}, {"Comma", KEY_COMMA}, {"Minus", KEY_MINUS},
		{"Period", KEY_PERIOD}, {"Slash", KEY_SLASH}, {"Semicolon", KEY_SEMICOLON},
		{"Equal", KEY_EQUAL}, {"LeftBracket", KEY_LEFT_BRACKET}, {"Backslash", KEY_BACKSLASH},
		{"RightBracket", KEY_RIGHT_BRACKET}, {"Grave", KEY_GRAVE},
	};

	static std::unordered_map<std::string, MouseButton> MouseStrings =
	{
		{"MouseLeft", MOUSE_BUTTON_LEFT},
		{"MouseRight", MOUSE_BUTTON_RIGHT},
		{"MouseMiddle", MOUSE_BUTTON_MIDDLE},
		{"MouseSide", MOUSE_BUTTON_SIDE},
		{"MouseExtra", MOUSE_BUTTON_EXTRA},
		{"MouseForward", MOUSE_BUTTON_FORWARD},
		{"MouseBack", MOUSE_BUTTON_BACK},
	};

	static std::unordered_map<std::string, GamepadButton> GamepadStrings =
	{
		{"MouseRight", GAMEPAD_BUTTON_LEFT_FACE_UP},		// Gamepad left DPAD up button
		{"MouseMiddle", GAMEPAD_BUTTON_LEFT_FACE_RIGHT},	// Gamepad left DPAD right button
		{"MouseSide", GAMEPAD_BUTTON_LEFT_FACE_DOWN},		// Gamepad left DPAD down button
		{"MouseExtra", GAMEPAD_BUTTON_LEFT_FACE_LEFT},		// Gamepad left DPAD left button
		{"MouseForward", GAMEPAD_BUTTON_RIGHT_FACE_UP},		// Gamepad right button up(i.e.PS3: Triangle, Xbox : Y)
		{"MouseBack", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT},		// Gamepad right button right (i.e. PS3: Circle, Xbox: B)
		{"MouseBack", GAMEPAD_BUTTON_RIGHT_FACE_DOWN},      // Gamepad right button down (i.e. PS3: Cross, Xbox: A)
		{"MouseBack", GAMEPAD_BUTTON_RIGHT_FACE_LEFT},		// Gamepad right button left (i.e. PS3: Square, Xbox: X)
		{"MouseBack", GAMEPAD_BUTTON_LEFT_TRIGGER_1},		// Gamepad top/back trigger left (first), it could be a trailing button
		{"MouseBack", GAMEPAD_BUTTON_LEFT_TRIGGER_2},		// Gamepad top/back trigger left (second), it could be a trailing button
		{"MouseBack", GAMEPAD_BUTTON_RIGHT_TRIGGER_1},		// Gamepad top/back trigger right (first), it could be a trailing button
		{"MouseBack", GAMEPAD_BUTTON_RIGHT_TRIGGER_2},		// Gamepad top/back trigger right (second), it could be a trailing button
		{"MouseBack", GAMEPAD_BUTTON_MIDDLE_LEFT},			// Gamepad center buttons, left one (i.e. PS3: Select)
		{"MouseBack", GAMEPAD_BUTTON_MIDDLE},				// Gamepad center buttons, middle one (i.e. PS3: PS, Xbox: XBOX)s
		{"MouseBack", GAMEPAD_BUTTON_MIDDLE_RIGHT},			// Gamepad center buttons, right one (i.e. PS3: Start)
		{"MouseBack", GAMEPAD_BUTTON_LEFT_THUMB},			//Gamepad joystick pressed button left
		{"MouseBack", GAMEPAD_BUTTON_RIGHT_THUMB},			//Gamepad joystick pressed button right
	};
		

	std::string ToString(const Color& color)
	{
		return std::format("[R:{}, G:{}, B:{}, A:{}]",
			std::to_string(color.r), std::to_string(color.g), std::to_string(color.b), std::to_string(color.a));
	}

	Color MultiplyColorsRGBA(const Color& color, const Utils::Point4D& factor)
	{
		unsigned char newR = color.r * factor.m_X;
		unsigned char newG = color.g * factor.m_Y;
		unsigned char newB = color.b * factor.m_Z;
		unsigned char newA = color.a * factor.m_W;
		return {newR, newG, newB, newA};
	}

	Color MultiplyColorsRGB(const Color& color, const Utils::Point3D& factor)
	{
		unsigned char newR = color.r * factor.m_X;
		unsigned char newG = color.g * factor.m_Y;
		unsigned char newB = color.b * factor.m_Z;
		return { newR, newG, newB, color.a};
	}

	Utils::Point4D DivideColorRGBA(const Color& divided, const Color& divisor)
	{
		return Utils::Point4D((float)divided.r / divisor.r, (float)divided.g / divisor.g, (float)divided.b / divisor.b, (float)divided.a / divisor.a);
	}

	Utils::Point3D DivideColorRGB(const Color& divided, const Color& divisor)
	{
		return Utils::Point3D((float)divided.r / divisor.r, (float)divided.g / divisor.g, (float)divided.b / divisor.b);
	}

	Utils::Point4D GetFractionalColorRGBA(const Color& color, const float& multiplier)
	{
		return {color.r* multiplier , color.g*multiplier, color.b*multiplier, color.a*multiplier};
	}

	Utils::Point3D GetFractionalColorRGB(const Color& color, const float& multiplier)
	{
		return { color.r * multiplier , color.g * multiplier, color.b * multiplier};
	}

	Color GetColorFromPoint(const Utils::Point4D& rgba)
	{
		return { static_cast<unsigned char>(rgba.m_X),
				 static_cast<unsigned char>(rgba.m_Y),
				 static_cast<unsigned char>(rgba.m_Z),
				 static_cast<unsigned char>(rgba.m_W) };
	}

	Color GetColorFromPoint(const Utils::Point3D& rgb)
	{
		return { static_cast<unsigned char>(rgb.m_X),
				 static_cast<unsigned char>(rgb.m_Y),
				 static_cast<unsigned char>(rgb.m_Z) };
	}

	Color GetColorFromHex(const std::uint32_t& hexNumber)
	{
		return
		{
			//We can just move the corresponding rgba value and then mask it
			//to only include that segment, and convert to unsigned char as needed
			static_cast<unsigned char>((hexNumber >> 24) & 0xFF),
			static_cast<unsigned char>((hexNumber >> 16) & 0xFF),
			static_cast<unsigned char>((hexNumber >> 8) & 0xFF), 
			static_cast<unsigned char>(hexNumber & 0xFF)  
		};
	}

	bool ColorEqual(const Color& color1, const Color& color2)
	{
		return color1.r == color2.r && color1.g == color2.g && 
			color1.b == color2.b && color1.a == color2.a;
	}

	void DrawFPSCounter()
	{
		DrawText(std::format("FPS: {}", GetFPS()).c_str(), 5, 5, 24, WHITE);
	}

	std::string ToString(const Vector2& vec)
	{
		return std::format("[x:{} y:{}]", std::to_string(vec.x), std::to_string(vec.y));
	}

	Vector2 ToRaylibVector(const Vec2& vec)
	{
		return { vec.m_X, vec.m_Y };
	}

	Vector2 ToRaylibVector(const Utils::Point2D& point)
	{
		return { point.m_X, point.m_Y };
	}
	Vector2 ToRaylibVector(const Utils::Point2DInt& point)
	{
		return { static_cast<float>(point.m_X), static_cast<float>(point.m_Y )};
	}

	bool IsValidFont(const Font& font)
	{
		return font.texture.id != 0;
	}
	float GetMaxFontSizeForSpace(const Font& font, const std::string& text, const Utils::Point2D& space, const float& spacing)
	{
		Vector2 currentSpace = {std::numeric_limits<float>().max(), std::numeric_limits<float>().max() };
		float fontSize = space.m_Y;
		while (currentSpace.x <= space.m_X && currentSpace.y <= space.m_Y)
		{
			currentSpace = MeasureTextEx(font, text.c_str(), fontSize, spacing);
		}
		return fontSize;
	}

	void RemoveFontExtraSpacing(Font& font)
	{
		for (int i = 0; i < font.glyphCount; i++)
		{
			font.glyphs[i].offsetX = 0; 
			font.glyphs[i].offsetY = 0;
			font.glyphs[i].advanceX = 0;
		}
	}

	bool FontSupportsChar(const Font& font, const char& character)
	{
		return GetGlyphIndex(font, character) != -1;
	}

	std::optional<KeyboardKey> TryStringToKeyboardKey(const std::string& str)
	{
		auto it = KeyboardStrings.find(str);
		if (it!= KeyboardStrings.end()) return it->second;

		return std::nullopt;
	}
	std::optional<MouseButton> TryStringToMouseButton(const std::string& str)
	{
		auto it = MouseStrings.find(str);
		if (it != MouseStrings.end()) return it->second;

		return std::nullopt;
	}
	std::optional<GamepadButton> TryStringToGamepadButton(const std::string& str)
	{
		auto it = GamepadStrings.find(str);
		if (it != GamepadStrings.end()) return it->second;

		return std::nullopt;
	}

	KeyboardKey StringToKeyboardKey(const std::string& str)
	{
		auto maybe = TryStringToKeyboardKey(str);
		if (!Assert(maybe.has_value(), std::format("Tried to convert string: '{}' "
			"to KEYBOARD key but it does not correspond to a valid key", str)))
			throw std::invalid_argument("Invalid Device for string conversion");

		return maybe.value();
	}
	MouseButton StringToMouseButton(const std::string& str)
	{
		auto maybe = TryStringToMouseButton(str);
		if (!Assert(maybe.has_value(), std::format("Tried to convert string: '{}' "
			"to MOUSE button but it does not correspond to a valid button", str)))
			throw std::invalid_argument("Invalid Device for string conversion");

		return maybe.value();
	}
	GamepadButton StringToGamepadButton(const std::string& str)
	{
		auto maybe = TryStringToGamepadButton(str);
		if (!Assert(maybe.has_value(), std::format("Tried to convert string: '{}' "
			"to GAMEPAD button but it does not correspond to a valid button", str)))
			throw std::invalid_argument("Invalid Device for string conversion");

		return maybe.value();
	}

	std::string KeyboardKeyToString(const KeyboardKey& key)
	{
		for (const auto& keyboardKey : KeyboardStrings)
		{
			if (keyboardKey.second == key) return keyboardKey.first;
		}
		return "";
	}
	std::string MouseButtonToString(const MouseButton& button)
	{
		for (const auto& mouseButton : MouseStrings)
		{
			if (mouseButton.second == button) return mouseButton.first;
		}
		return "";
	}
	std::string GamepadButtonToString(const GamepadButton& button)
	{
		for (const auto& gamepadButton : GamepadStrings)
		{
			if (gamepadButton.second == button) return gamepadButton.first;
		}
		return "";
	}
}
