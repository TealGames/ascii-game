#pragma once
#include "Utils/Debug.hpp"
#include <cstdint>
#include <string>
#include <optional>
#include <any>
#include <array>
#include "Core/Input/InputState.hpp"

namespace Engine::Input
{
	enum class DeviceType : std::uint8_t
	{
		Keyboard		= 0,
		Mouse			= 1,
		Gamepad			= 2
	};
	std::string ToString(const DeviceType& device);

#define KEY_CODE_ENUM_LIST \
    X(Null, 0) \
    \
    X(MouseLeft, 1)        /* Left mouse button */ \
    X(MouseRight, 2)       /* Right mouse button */ \
    X(MouseMiddle, 3)      /* Middle mouse button */ \
    X(MouseSideBack, 4)    \
    X(MouseSideForward, 5) \
    X(MouseButton6, 6)     \
    X(MouseButton7, 7)     \
    X(MouseButton8, 8)     \
    \
    X(GamepadA, 10) \
    X(GamepadB, 11) \
    X(GamepadX, 12) \
    X(GamepadY, 13) \
    X(GamepadLeftBumper, 14) \
    X(GamepadRightBumper, 15) \
    X(GamepadBack, 16) \
    X(GamepadStart, 17) \
    X(GamepadGuide, 18) \
    X(GamepadLeftStick, 19) \
    X(GamepadRightStick, 20) \
    X(GamepadDpadUp, 21) \
    X(GamepadDpadRight, 22) \
    X(GamepadDpadDown, 23) \
    X(GamepadDpadLeft, 24) \
    X(GamepadLast, 25) \
    \
    /* Printable keys */ \
    X(Space, 32) \
    X(Apostrophe, 39)      /* ' */ \
    X(Comma, 44)           /* , */ \
    X(Minus, 45)           /* - */ \
    X(Period, 46)          /* . */ \
    X(Slash, 47)           /* / */ \
    \
    /* Top row numbers */ \
    X(Num0, 48) \
    X(Num1, 49) \
    X(Num2, 50) \
    X(Num3, 51) \
    X(Num4, 52) \
    X(Num5, 53) \
    X(Num6, 54) \
    X(Num7, 55) \
    X(Num8, 56) \
    X(Num9, 57) \
    \
    X(Semicolon, 59)       /* ; */ \
    X(Equal, 61)           /* = */ \
    \
    /* Letters */ \
    X(A, 65) X(B, 66) X(C, 67) X(D, 68) X(E, 69) \
    X(F, 70) X(G, 71) X(H, 72) X(I, 73) X(J, 74) \
    X(K, 75) X(L, 76) X(M, 77) X(N, 78) X(O, 79) \
    X(P, 80) X(Q, 81) X(R, 82) X(S, 83) X(T, 84) \
    X(U, 85) X(V, 86) X(W, 87) X(X, 88) X(Y, 89) X(Z, 90) \
    \
    X(BracketLeft, 91)     /* [ */ \
    X(Backslash, 92)       /* \ */ \
    X(BracketRight, 93)    /* ] */ \
    X(GraveAccent, 96)     /* ` */ \
    \
    X(World1, 161) \
    X(World2, 162) \
    \
    /* Function keys */ \
    X(Escape, 256) \
    X(Enter, 257) \
    X(Tab, 258) \
    X(Backspace, 259) \
    X(Insert, 260) \
    X(Delete, 261) \
    X(ArrowRight, 262) \
    X(ArrowLeft, 263) \
    X(ArrowDown, 264) \
    X(ArrowUp, 265) \
    X(PageUp, 266) \
    X(PageDown, 267) \
    X(Home, 268) \
    X(End, 269) \
    \
    X(CapsLock, 280) \
    X(ScrollLock, 281) \
    X(NumLock, 282) \
    X(PrintScreen, 283) \
    X(Pause, 284) \
    \
    X(F1, 290) X(F2, 291) X(F3, 292) X(F4, 293) X(F5, 294) \
    X(F6, 295) X(F7, 296) X(F8, 297) X(F9, 298) X(F10, 299) \
    X(F11, 300) X(F12, 301) X(F13, 302) X(F14, 303) X(F15, 304) \
    X(F16, 305) X(F17, 306) X(F18, 307) X(F19, 308) X(F20, 309) \
    X(F21, 310) X(F22, 311) X(F23, 312) X(F24, 313) X(F25, 314) \
    \
    /* Keypad */ \
    X(Keypad0, 320) X(Keypad1, 321) X(Keypad2, 322) X(Keypad3, 323) \
    X(Keypad4, 324) X(Keypad5, 325) X(Keypad6, 326) X(Keypad7, 327) \
    X(Keypad8, 328) X(Keypad9, 329) \
    X(KeypadDecimal, 330) X(KeypadDivide, 331) \
    X(KeypadMultiply, 332) X(KeypadSubtract, 333) X(KeypadAdd, 334) \
    X(KeypadEnter, 335) X(KeypadEqual, 336) \
    \
    /* Modifiers */ \
    X(ShiftLeft, 340) X(ControlLeft, 341) X(AltLeft, 342) X(SuperLeft, 343) \
    X(ShiftRight, 344) X(ControlRight, 345) X(AltRight, 346) X(SuperRight, 347) \
    X(Menu, 348)

	using KeyCodeIntegralType = std::uint16_t;
	enum class KeyCode : KeyCodeIntegralType
	{
#define X(name, val) name = val,
		KEY_CODE_ENUM_LIST
#undef X
		/*
		Null=		0,

		MouseLeft = 1,
		MouseRight = 2,
		MouseMiddle = 3,
		MouseSideBack = 4,
		MouseSideForward = 5,
		MouseButton6 = 6,
		MouseButton7 = 7,
		MouseButton8 = 8,

		GamepadA = 10,
		GamepadB = 11,
		GamepadX = 12,
		GamepadY = 13,
		GamepadLeftBumper = 14,
		GamepadRightBumper = 15,
		GamepadBack = 16,
		GamepadStart = 17,
		GamepadGuide = 18,
		GamepadLeftStick = 19,
		GamepadRightStick = 20,
		GamepadDpadUp = 21,
		GamepadDpadRight = 22,
		GamepadDpadDown = 23,
		GamepadDpadLeft = 24,
		GamepadLast = 25,

		// Printable keys
		Space = 32,
		Apostrophe = 39,  // '
		Comma = 44,  // ,
		Minus = 45,  // -
		Period = 46,  // .
		Slash = 47,  // /

		//Top row numbers
		Num0 = 48,
		Num1 = 49,
		Num2 = 50,
		Num3 = 51,
		Num4 = 52,
		Num5 = 53,
		Num6 = 54,
		Num7 = 55,
		Num8 = 56,
		Num9 = 57,

		Semicolon = 59,  // ;
		Equal = 61,  // =

		A = 65, B = 66, C = 67, D = 68, E = 69,
		F = 70, G = 71, H = 72, I = 73, J = 74,
		K = 75, L = 76, M = 77, N = 78, O = 79,
		P = 80, Q = 81, R = 82, S = 83, T = 84,
		U = 85, V = 86, W = 87, X = 88, Y = 89, Z = 90,

		LeftBracket = 91,  // [
		Backslash = 92,  // '\'
		RightBracket = 93,  // ]
		GraveAccent = 96,  // `

		World1 = 161,
		World2 = 162,

		// Function keys
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		RightArrow = 262,
		LeftArrow = 263,
		DownArrow = 264,
		UpArrow = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,

		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,

		F1 = 290, F2 = 291, F3 = 292, F4 = 293, F5 = 294,
		F6 = 295, F7 = 296, F8 = 297, F9 = 298, F10 = 299,
		F11 = 300, F12 = 301, F13 = 302, F14 = 303, F15 = 304,
		F16 = 305, F17 = 306, F18 = 307, F19 = 308, F20 = 309,
		F21 = 310, F22 = 311, F23 = 312, F24 = 313, F25 = 314,

		// Keypad
		Keypad0 = 320,
		Keypad1 = 321,
		Keypad2 = 322,
		Keypad3 = 323,
		Keypad4 = 324,
		Keypad5 = 325,
		Keypad6 = 326,
		Keypad7 = 327,
		Keypad8 = 328,
		Keypad9 = 329,
		KeypadDecimal = 330,
		KeypadDivide = 331,
		KeypadMultiply = 332,
		KeypadSubtract = 333,
		KeypadAdd = 334,
		KeypadEnter = 335,
		KeypadEqual = 336,

		// Modifiers
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348,
		*/
	};

	constexpr KeyCode FIRST_MOUSE_CODE = KeyCode::MouseLeft;
	constexpr KeyCode LAST_MOUSE_CODE = KeyCode::MouseButton8;
	constexpr KeyCode FIRST_KEYBOARD_CODE = KeyCode::Space;
	constexpr KeyCode LAST_KEYBOARD_CODE = KeyCode::Menu;
	constexpr KeyCode FIRST_GAMEPAD_CODE = KeyCode::GamepadA;
	constexpr KeyCode LAST_GAMEPAD_CODE = KeyCode::GamepadLast;

	constexpr KeyCodeIntegralType MOUSE_KEY_COUNT = KeyCodeIntegralType(LAST_MOUSE_CODE)- KeyCodeIntegralType(FIRST_MOUSE_CODE) + 1;
	constexpr KeyCodeIntegralType KEYBOARD_KEY_COUNT = KeyCodeIntegralType(LAST_KEYBOARD_CODE) - KeyCodeIntegralType(FIRST_KEYBOARD_CODE) + 1;
	constexpr KeyCodeIntegralType GAMEPAD_KEY_COUNT = KeyCodeIntegralType(LAST_GAMEPAD_CODE) - KeyCodeIntegralType(FIRST_GAMEPAD_CODE) + 1;

	std::array<KeyCode, KEYBOARD_KEY_COUNT> GetAllKeyboardKeys();
	std::array<KeyCode, MOUSE_KEY_COUNT> GetAllMouseButtons();
	std::array<KeyCode, GAMEPAD_KEY_COUNT> GetAllGamepadButtons();

	KeyCode ToKeyCode(const std::string& str);
	std::string ToString(const KeyCode keyCode);

	DeviceType GetDeviceFromKeyCode(const KeyCode keyCode);
	bool IsKeyCodeTextConvertible(const KeyCode keyCode);
	char GetKeyCodeAsChar(const KeyCode code, const bool isShiftPressed, const bool isCapsLockPressed);

	class InputKeyState
	{
	private:
		DeviceType m_deviceType;
		KeyCode m_keyCode;
		InputState m_state;

	public:

	private:
	public:
		InputKeyState(const KeyCode& key, const InputState& state);

		bool IsDevice(const DeviceType& device) const;
		KeyCode GetKeyCode() const;

		const InputState& GetState() const;
		InputState& GetStateMutable();

		std::string ToString(const bool showDeviceName=true, const bool showState=true) const;
	};
}
