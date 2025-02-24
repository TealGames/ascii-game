#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include "InputManager.hpp"
#include "raylib.h"
#include "IRenderable.hpp"
#include "GUIRect.hpp"
#include "ISelectable.hpp"

enum class InputFieldType
{
	Any,
	Integer,
	Float,
	String,
};
std::string ToString(const InputFieldType& type);

using InputFieldFlagIntegralType = std::uint8_t;
enum class InputFieldFlag : InputFieldFlagIntegralType
{
	None= 0,
	/// <summary>
	/// If true, will display the caret symbol when typing
	/// </summary>
	ShowCaret= 1 <<1,
	/// <summary>
	/// If true, the field will automatically be selected
	/// for accepting input
	/// </summary>
	SelectOnStart= 1<<2,
};

constexpr InputFieldFlag operator&(const InputFieldFlag& lhs, const InputFieldFlag& rhs)
{
	return static_cast<InputFieldFlag>(static_cast<InputFieldFlagIntegralType>(lhs)
		& static_cast<InputFieldFlagIntegralType>(rhs));
}
constexpr InputFieldFlag& operator&=(InputFieldFlag& lhs, const InputFieldFlag& rhs)
{
	lhs = lhs & rhs;
	return lhs;
}
constexpr InputFieldFlag operator|(const InputFieldFlag& lhs, const InputFieldFlag& rhs)
{
	return static_cast<InputFieldFlag>(static_cast<InputFieldFlagIntegralType>(lhs)
		| static_cast<InputFieldFlagIntegralType>(rhs));
}
constexpr InputFieldFlag& operator|=(InputFieldFlag& lhs, const InputFieldFlag& rhs)
{
	lhs = lhs | rhs;
	return lhs;
}
constexpr InputFieldFlag operator~(const InputFieldFlag& op)
{
	return static_cast<InputFieldFlag>(~static_cast<InputFieldFlagIntegralType>(op));
}

using InputFieldAction = std::function<void(std::string input)>;
using InputFieldKeyActions = std::unordered_map<KeyboardKey, InputFieldAction>;
class InputField : IRenderable, ISelectable
{
private:
	InputFieldType m_type;
	std::string m_input;
	std::string m_attemptedInput;
	InputFieldFlag m_inputFlags;

	bool m_isSelected;
	GUIRect m_lastRenderRect;

	InputFieldAction m_submitAction;
	InputFieldKeyActions m_keyActions;

	ScreenPosition m_maxSize;

	const Input::InputManager* m_inputManager;

public:

private:
	InputField(const Input::InputManager* manager, const InputFieldType& type, const InputFieldFlag& flags,
		const ScreenPosition& maxSize, const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions);

	std::string CleanInput(const std::string& input) const;
	void ResetInput();
	void SetAttemptedInputDelta(const std::string& input);

	const Input::InputManager& GetInputManager() const;

	void SetInput(const std::string& newInput, const bool isAttemptedInput);

public:
	InputField();
	InputField(const Input::InputManager& manager, const InputFieldType& type, const InputFieldFlag& flags, 
		const ScreenPosition& maxSize,
		const InputFieldAction& submitAction=nullptr, const InputFieldKeyActions& keyPressActions = {});
	~InputField();

	void Update();

	bool ShowCaret() const;
	bool IsSelectedOnStart() const;
	const InputFieldType& GetFieldType() const;

	void SetSubmitAction(const InputFieldAction& action);

	void OverrideInput(const std::string& str);
	std::string GetInput() const;
	/// <summary>
	/// Will return input as integer.
	/// Note: only use unless you are sure the input is an integer (otherwise will throw)
	/// </summary>
	/// <returns></returns>
	int GetIntInput() const;

	/// <summary>
	/// Will return input as float.
	/// Note: only use unless you are sure the input is a float (otherwise will throw)
	/// </summary>
	/// <returns></returns>
	float GetFloatInput() const;

	ScreenPosition Render(const RenderInfo& renderInfo) override;
	void Select() override;
	void Deselect() override;
};

