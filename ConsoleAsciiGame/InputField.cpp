#include <optional>
#include <cstdint>
#include "pch.hpp"
#include "InputField.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"
#include "Globals.hpp"
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

InputField::InputField(const Input::InputManager* manager, GUISelectorManager* selectorManager, 
	const InputFieldType& type, const InputFieldFlag& flags, const GUISettings& settings, 
	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
	: SelectableGUI(selectorManager), m_inputManager(manager), m_type(type), m_input(), m_lastInput(), m_inputFlags(flags), m_submitAction(submitAction),
	m_keyActions(keyPressActions), m_settings(settings), m_lastRenderRect()
{
	//LogError("Creating Input FIeld");
	//Assert(false, std::format("Created  gui redct: {}", GetLastFrameRect().ToString()));
	if (HasFlag(InputFieldFlag::SelectOnStart)) Select();
}

InputField::InputField() : InputField(nullptr, nullptr, InputFieldType::Any, InputFieldFlag::None, {}, nullptr, {}) 
{
	
}

InputField::InputField(const Input::InputManager& manager, GUISelectorManager& selectorManager, 
	const InputFieldType& type, const InputFieldFlag& flags, const GUISettings& settings,
	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
	:InputField(&manager, &selectorManager, type, flags, settings, submitAction, keyPressActions) {}

InputField::~InputField()
{
	//LogError("Input field destroyed");
}

const Input::InputManager& InputField::GetInputManager() const
{
	if (!Assert(this, m_inputManager != nullptr,
		std::format("Tried to retreive input manager from input field but it is NULLPTR")))
	{
		throw std::invalid_argument("Invalid input manager");
	}
	return *m_inputManager;
}

std::string InputField::CleanInput(const std::string& input) const
{
	return Utils::StringUtil(input).RemoveSpaces().ToString();
}

void InputField::Update()
{
	//TODO: increase the area of selection and add potential padding
	//TODO: add general key select and submits for a general profile so each selectable does not 
	// need to have its own defined
	
	//LogError(std::format("Is selected: {} attempted: {} real: {}", m_isSelected, m_attemptedInput, m_input));
	//If we press select key inside this area we then select or deselect
	//bool isSelectReleased = GetInputManager().GetInputKey(SELECT_KEY)->GetState().IsReleased();

	/*LogError(std::format("SELECTED FIELD. last render rect: {} mouse: {} CONTAINS: {}",
		m_lastRenderRect.ToString(), m_inputManager->GetMousePosition().ToString(), 
		std::to_string(m_lastRenderRect.ContainsPos(m_inputManager->GetMousePosition()))));*/

	/*
	if (isSelectReleased && m_lastRenderRect.ContainsPos(m_inputManager->GetMousePosition()))
	{
		if (m_isSelected)
		{
			Deselect();
		}
		else
		{
			Select();
			LogError("SELECTED");
		}
	}
	*/

	//TODO: this might bnot be the best way of doing this and having this bee called everytime on update
	//instead this should be a function called by some general system on selectables on gui selector manager
	if (!IsInit()) Init();

	if (IsSelected() && GetInputManager().GetInputKey(ESCAPE_KEY)->GetState().IsReleased())
	{
		Deselect();
		m_attemptedInput = "";
		//Assert(false, "ESCAPIGN FROMR INTPUT");
		return;
	}

	if (!IsSelected()) return;

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

	if (GetInputManager().IsKeyReleased(DELETE_KEY) && !m_attemptedInput.empty())
	{
		m_attemptedInput.pop_back();
	}

	if (!m_keyActions.empty())
	{
		for (const auto& key : m_keyActions)
		{
			if (GetInputManager().IsKeyReleased(key.first))
				key.second(GetInput());
		}
	}

	std::string keysPressed = GetInputManager().GetCharsPressedSinceLastFrame();
	//LogError(std::format("Setting input field input delta: {}", keysPressed));
	if (keysPressed.empty()) return;

	//Assert(false, std::format("Chars pressed since last frame: {}", keysPressed));
	SetAttemptedInputDelta(keysPressed);
}

void InputField::SetSubmitAction(const InputFieldAction& action)
{
	m_submitAction = action;
}

void InputField::SetAttemptedInputDelta(const std::string& input)
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
void InputField::ResetInput()
{
	m_input = "";
}

void InputField::SetInput(const std::string& newInput, const bool isAttemptedInput)
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

void InputField::OverrideInput(const std::string& input)
{
	SetInput(input, false);
}

const InputFieldType& InputField::GetFieldType() const
{
	return m_type;
}

std::string InputField::GetDisplayInput() const
{
	if (HasFlag(InputFieldFlag::ShowCaret)) return m_input + "_";
	return m_input;
}
std::string InputField::GetDisplayAttemptedInput() const
{
	if (HasFlag(InputFieldFlag::ShowCaret)) return m_attemptedInput + "_";
	return m_attemptedInput;
}

const std::string& InputField::GetInput() const
{
	return m_input;
}

const std::string& InputField::GetLastInput() const
{
	return m_lastInput;
}

int InputField::GetIntInput() const
{
	return std::stoi(m_input);
}

float InputField::GetFloatInput() const
{
	return std::stof(m_input);
}

bool InputField::HasFlag(const InputFieldFlag& flag) const
{
	return Utils::HasFlagAll(m_inputFlags, flag);
}

ScreenPosition InputField::Render(const RenderInfo& renderInfo)
{
	const int widthUsed = std::min(renderInfo.m_RenderSize.m_X, m_settings.m_Size.m_X);
	const int heightUsed = std::min(renderInfo.m_RenderSize.m_Y, m_settings.m_Size.m_Y);
	Utils::Point2DInt renderSize = { widthUsed, heightUsed };

	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y, widthUsed, heightUsed, m_settings.m_BackgroundColor);
	std::string inputStr = IsSelected() ? GetDisplayAttemptedInput() : GetDisplayInput();
	//Assert(false, std::format("Found input: {}", inputStr));
	//TODO: right now this deos not consider text overflow, or if the text is longer than input field
	//TODO: add the render settigns like font, font size, spacing as constructor args

	Vector2 textStartPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	const float fontSize = m_settings.m_TextSettings.GetFontSize(renderSize);
	/*if (m_settings.m_TextSettings.m_FontSizeType==TextGUIFontSize::ParentArea) 
		Assert(false, std::format("Font size is: {} text factor: {}", std::to_string(fontSize), std::to_string(m_settings.m_TextSettings.m_FontSize)));*/

	textStartPos.x += m_settings.m_TextSettings.m_RightIndent;
	DrawTextEx(GetGlobalFont(), inputStr.c_str(), textStartPos,
		fontSize, DEBUG_INFO_CHAR_SPACING.m_X, m_settings.m_TextSettings.m_TextColor);

	if (!IsSelected()) DrawDisabledOverlay({ renderInfo.m_TopLeftPos, renderSize });

	GetLastFrameRectMutable().SetSize(renderSize);
	GetLastFrameRectMutable().SetTopLeftPos(renderInfo.m_TopLeftPos);
	//Assert(false, std::format("On render set last rect mutable; {}", GetLastFrameRect().ToString()));

	/*m_lastRenderRect.SetSize(renderSize);
	m_lastRenderRect.SetTopLeftPos(renderInfo.m_TopLeftPos);*/
	
	//LogError(std::format("Updating input field rect to: {}", m_lastRenderRect.ToString()));
	return renderSize;
}

//const GUIRect& InputField::GetLastRenderRect() const
//{
//	return m_lastRenderRect;
//}