#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include "InputManager.hpp"
#include "raylib.h"
#include "UIRect.hpp"
#include "UIStyle.hpp"
#include "UISelectableData.hpp"

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
	/// <summary>
	/// If true will keep the field selected even on submit action
	/// </summary>
	KeepSelectedOnSubmit = 1<<3,
	/// <summary>
	/// If true, this input field CAN still be selected, but 
	/// values CAN NOT be changed BY THE USER (functions still work but 
	/// clicking on it can not be changed from UI)
	/// </summary>
	UserUIReadonly= 1<<4
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

class UITextComponent;
class UIPanel;
namespace ECS { class UIInputFieldSystem; }

using InputFieldAction = std::function<void(std::string input)>;
using InputFieldKeyActions = std::unordered_map<KeyboardKey, InputFieldAction>;
class UIInputField : public UISelectableData
{
private:
	InputFieldType m_type;

	std::string m_input;
	std::string m_lastInput;
	std::string m_attemptedInput;
	UITextComponent* m_textGUI;
	UIPanel* m_background;
	InputFieldFlag m_inputFlags;

	//bool m_isSelected;
	InputFieldAction m_submitAction;
	InputFieldKeyActions m_keyActions;

	UIStyle m_settings;

	const Input::InputManager* m_inputManager;
public:
	friend class ECS::UIInputFieldSystem;

private:
	/*InputFieldGUI(const Input::InputManager* manager, const InputFieldType& type, const InputFieldFlag& flags,
		const GUISettings& settings, const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions);*/

	std::string CleanInput(const std::string& input) const;
	
	void SetAttemptedInputDelta(const std::string& input);
	const Input::InputManager& GetInputManager() const;

	/// <summary>
	/// Will get input fit for display. Should not be used for normal purposes and
	/// is only useful for rendering
	/// </summary>
	/// <returns></returns>
	std::string GetDisplayInput() const;
	std::string GetDisplayAttemptedInput() const;
	void SetInput(const std::string& newInput, const bool isAttemptedInput);

	void UpdateInput(const float deltaTime);

public:
	//InputField();
	UIInputField(const Input::InputManager& manager, const InputFieldType& type, 
		const InputFieldFlag& flags, const UIStyle& settings, 
		const InputFieldAction& submitAction=nullptr, const InputFieldKeyActions& keyPressActions = {});
	~UIInputField();

	void Update(const float deltaTime) override;

	bool HasFlag(const InputFieldFlag& flag) const;
	const InputFieldType& GetFieldType() const;

	void SetSubmitAction(const InputFieldAction& action);
	void SetKeyPressAction(const KeyboardKey key, const InputFieldAction& action);
	void SetSettings(const UIStyle& settings);

	void OverrideInput(const std::string& str);
	const std::string& GetInput() const;
	const std::string& GetLastInput() const;
	void ResetInput();

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

	//RenderInfo ElementRender(const RenderInfo& renderInfo) override;
	//const GUIRect& GetLastRenderRect() const;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

