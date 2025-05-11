#include <optional>
#include <cstdint>
#include "pch.hpp"
#include "InputFieldGUI.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"
#include "Globals.hpp"
#include "Vec2Int.hpp"
#include "RaylibUtils.hpp"

static constexpr KeyboardKey SUBMIT_KEY = KEY_ENTER;
static constexpr KeyboardKey ESCAPE_KEY = KEY_COMMA;
static constexpr KeyboardKey DELETE_KEY = KEY_BACKSPACE;
static constexpr MouseButton SELECT_KEY = MOUSE_BUTTON_LEFT;
static constexpr float FONT_SIZE = 10;

static constexpr std::uint8_t MAX_DECIMAL_PLCES = 3;

std::string ToString(const InputFieldType& type)
{
	if (type == InputFieldType::Any) return "Any";
	else if (type == InputFieldType::Float) return "Float";
	else if (type == InputFieldType::Integer) return "Integer";
	else if (type == InputFieldType::String) return "String";
	
	LogError(std::format("Tried to convert input field type to string but no actions were found"));
	return "";
}

InputFieldGUI::InputFieldGUI(const Input::InputManager& manager, const InputFieldType& type, 
	const InputFieldFlag& flags, const GUIStyle& settings, 
	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
	: SelectableGUI(), m_inputManager(&manager), m_type(type), m_input(), m_lastInput(), 
	m_textGUI("", settings.m_TextSettings), m_inputFlags(flags), m_submitAction(submitAction),
	m_keyActions(keyPressActions), m_settings(settings), m_lastRenderRect()
{
	//LogError("Creating Input FIeld");
	//Assert(false, std::format("Created  gui redct: {}", GetLastFrameRect().ToString()));
	if (HasFlag(InputFieldFlag::SelectOnStart)) Select();
}

//InputField::InputField() : InputField(nullptr, nullptr, InputFieldType::Any, InputFieldFlag::None, {}, nullptr, {}) 
//{
//	
//}

//InputFieldGUI::InputFieldGUI(const Input::InputManager& manager,
//	const InputFieldType& type, const InputFieldFlag& flags, const GUISettings& settings,
//	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
//	:InputFieldGUI(&manager, type, flags, settings, submitAction, keyPressActions) {}

InputFieldGUI::~InputFieldGUI()
{
	//LogError("Input field destroyed");
}

const Input::InputManager& InputFieldGUI::GetInputManager() const
{
	if (!Assert(this, m_inputManager != nullptr,
		std::format("Tried to retreive input manager from input field but it is NULLPTR")))
	{
		throw std::invalid_argument("Invalid input manager");
	}
	return *m_inputManager;
}

std::string InputFieldGUI::CleanInput(const std::string& input) const
{
	return Utils::StringUtil(input).RemoveSpaces().ToString();
}

void InputFieldGUI::Update(const float deltaTime)
{
	if (IsSelected() && GetInputManager().GetInputKey(ESCAPE_KEY)->GetState().IsReleased())
	{
		Deselect();
		m_attemptedInput = "";
		//Assert(false, "ESCAPIGN FROMR INTPUT");
		return;
	}

	if (!IsSelected()) return;

	if (!m_keyActions.empty())
	{
		for (const auto& key : m_keyActions)
		{
			if (GetInputManager().IsKeyReleased(key.first))
				key.second(GetInput());
		}
	}

	if (HasFlag(InputFieldFlag::UserUIReadonly)) return;

	//THE FOLLOWING LOGIC IS FOR SELECTING/WRITING TO SELECTABLE FROM UI BASED ON PLAYER INPUT:
	if (GetInputManager().IsKeyReleased(SUBMIT_KEY))
	{
		//If we have an integral field and we have only stored a negative (we need to do this to still allow negatives)
		//we must clear it so that it does not cause any errors
		if ((m_type == InputFieldType::Float || m_type == InputFieldType::Integer) 
			&& m_attemptedInput == "-") m_attemptedInput = "";

		SetInput(m_attemptedInput, false);
		m_attemptedInput = "";

		if (!HasFlag(InputFieldFlag::KeepSelectedOnSubmit)) Deselect();
		
		LogError(std::format("Before submit action input field type is: {}", ToString(GetFieldType())));
		if (m_submitAction != nullptr) m_submitAction(GetInput());
		
		LogError(std::format("AFTER submit action input field type is: {}", ToString(GetFieldType())));
		return;
	}
	else if (GetInputManager().IsKeyReleased(DELETE_KEY) && !m_attemptedInput.empty())
	{
		m_attemptedInput.pop_back();
	}

	std::string keysPressed = GetInputManager().GetCharsPressedSinceLastFrame();
	//LogError(std::format("Setting input field input delta: {}", keysPressed));
	if (keysPressed.empty()) return;

	//Assert(false, std::format("Chars pressed since last frame: {}", keysPressed));
	SetAttemptedInputDelta(keysPressed);
}

void InputFieldGUI::SetSubmitAction(const InputFieldAction& action) { m_submitAction = action; }
void InputFieldGUI::SetKeyPressAction(const KeyboardKey key, const InputFieldAction& action)
{
	m_keyActions.emplace(key, action);
}

void InputFieldGUI::SetSettings(const GUIStyle& settings) { m_settings = settings; }

void InputFieldGUI::SetAttemptedInputDelta(const std::string& input)
{
	std::string cleanedInput = CleanInput(input);

	if (m_type == InputFieldType::Integer)
	{
		SetInput(m_attemptedInput + Utils::TryExtractInt(input), true);
	}
	else if (m_type == InputFieldType::Float)
	{
		//LogError(std::format("Extracting float from input: {} is: {}", input, Utils::TryExtractFloat(input)));
		/*Assert(false, std::format("Extracting float from input: {} is: {} (OLD: {}) new: {}", 
			input, Utils::TryExtractFloat(input), m_input, m_input + Utils::TryExtractFloat(input)));*/
		//if (Utils::TryExtractFloat(input) == "-") Assert(false, std::format("Places neg"));
		//Assert(false, std::format("Chars pressed since last frame: {} extracted: {}", input, Utils::TryExtractFloat(input)));
		SetInput(m_attemptedInput + Utils::TryExtractFloat(input), true);
	}
	else if (m_type == InputFieldType::String)
	{
		SetInput(m_attemptedInput + Utils::TryExtractNonIntegralValues(input), true);
	}
	else SetInput(m_attemptedInput + input, true);
}
void InputFieldGUI::ResetInput() { m_input = ""; }

void InputFieldGUI::SetInput(const std::string& newInput, const bool isAttemptedInput)
{
	if (newInput.empty()) return;
	if (newInput == m_input) return;

	std::string correctedInput = newInput;
	if (m_type == InputFieldType::Float)
	{
		size_t decimalPos = correctedInput.find('.');
		if (decimalPos != std::string::npos && correctedInput.size() - 1 - decimalPos > MAX_DECIMAL_PLCES)
		{
			correctedInput = correctedInput.substr(0, decimalPos + 1 + MAX_DECIMAL_PLCES);
		}
		//if (newInput== "-") Assert(false, std::format("Corrected float input: {} to {}", newInput, correctedInput));
	}
	 
	if (isAttemptedInput) m_attemptedInput = correctedInput;
	else
	{
		m_input = correctedInput;
		m_lastInput = m_input;
	}
	//Assert(false, std::format("Override input with; {} newinput: {}", m_input, newInput));
}

void InputFieldGUI::OverrideInput(const std::string& input)
{
	SetInput(input, false);
}

const InputFieldType& InputFieldGUI::GetFieldType() const { return m_type; }

std::string InputFieldGUI::GetDisplayInput() const
{
	if (HasFlag(InputFieldFlag::ShowCaret)) return m_input + "_";
	return m_input;
}
std::string InputFieldGUI::GetDisplayAttemptedInput() const
{
	if (HasFlag(InputFieldFlag::ShowCaret)) return m_attemptedInput + "_";
	return m_attemptedInput;
}

const std::string& InputFieldGUI::GetInput() const { return m_input; }
const std::string& InputFieldGUI::GetLastInput() const { return m_lastInput; }
int InputFieldGUI::GetIntInput() const { return std::stoi(m_input); }
float InputFieldGUI::GetFloatInput() const { return std::stof(m_input); }

bool InputFieldGUI::HasFlag(const InputFieldFlag& flag) const
{
	return Utils::HasFlagAll(m_inputFlags, flag);
}

RenderInfo InputFieldGUI::Render(const RenderInfo& renderInfo)
{
	const int widthUsed = std::min(renderInfo.m_RenderSize.m_X, m_settings.m_Size.m_X);
	const int heightUsed = std::min(renderInfo.m_RenderSize.m_Y, m_settings.m_Size.m_Y);
	Vec2Int renderSize = { widthUsed, heightUsed };

	Assert(false, std::format("drawing field gui at:{}", renderInfo.ToString()));
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y, widthUsed, heightUsed, m_settings.m_BackgroundColor);
	std::string inputStr = IsSelected() && !HasFlag(InputFieldFlag::UserUIReadonly) ? GetDisplayAttemptedInput() : GetDisplayInput();
	//Assert(false, std::format("Found input: {}", inputStr));
	
	m_textGUI.SetText(inputStr);
	m_textGUI.Render(RenderInfo(renderInfo.m_TopLeftPos, renderSize));

	//Vector2 textStartPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	//const float fontSize = m_settings.m_TextSettings.GetFontSize(renderSize);
	////if (m_settings.m_TextSettings.m_FontSizeType==TextGUIFontSize::ParentArea) 
	////Assert(false, std::format("Font size is: {} text factor: {}", std::to_string(fontSize), std::to_string(m_settings.m_TextSettings.m_FontSize)));*/

	//textStartPos.x += m_settings.m_TextSettings.m_RightIndent;
	//DrawTextEx(GetGlobalFont(), inputStr.c_str(), textStartPos,
	//	fontSize, DEBUG_INFO_CHAR_SPACING.m_X, m_settings.m_TextSettings.m_TextColor);

	if (!IsSelected() || HasFlag(InputFieldFlag::UserUIReadonly)) DrawDisabledOverlay({ renderInfo.m_TopLeftPos, renderSize });

	GetLastFrameRectMutable().SetSize(renderSize);
	GetLastFrameRectMutable().SetTopLeftPos(renderInfo.m_TopLeftPos);
	//Assert(false, std::format("On render set last rect mutable; {}", GetLastFrameRect().ToString()));

	/*m_lastRenderRect.SetSize(renderSize);
	m_lastRenderRect.SetTopLeftPos(renderInfo.m_TopLeftPos);*/
	
	//LogError(std::format("Updating input field rect to: {}", m_lastRenderRect.ToString()));
	return { renderInfo.m_TopLeftPos, renderSize };
}

//const GUIRect& InputField::GetLastRenderRect() const
//{
//	return m_lastRenderRect;
//}