#include <optional>
#include <cstdint>
#include "pch.hpp"
#include "UIInputField.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"
#include "Globals.hpp"
#include "Vec2Int.hpp"
#include "RaylibUtils.hpp"
#include "UITextComponent.hpp"
#include "UIPanel.hpp"

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

UIInputField::UIInputField(const Input::InputManager& manager, const InputFieldType& type, 
	const InputFieldFlag& flags, const UIStyle& settings, 
	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
	: UISelectableData(), m_inputManager(&manager), m_type(type), m_input(), m_lastInput(), 
	m_textGUI(nullptr), m_background(nullptr), m_inputFlags(flags), m_submitAction(submitAction),
	m_keyActions(keyPressActions), m_settings(settings)
{
	//LogError("Creating Input FIeld");
	//Assert(false, std::format("Created  gui redct: {}", GetLastFrameRect().ToString()));
	if (HasFlag(InputFieldFlag::SelectOnStart)) Select();
	if (HasFlag(InputFieldFlag::UserUIReadonly)) AddRenderFlags(InteractionRenderFlags::DrawDisabledOverlay);
}

//InputField::InputField() : InputField(nullptr, nullptr, InputFieldType::Any, InputFieldFlag::None, {}, nullptr, {}) 
//{
//	
//}

//InputFieldGUI::InputFieldGUI(const Input::InputManager& manager,
//	const InputFieldType& type, const InputFieldFlag& flags, const GUISettings& settings,
//	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
//	:InputFieldGUI(&manager, type, flags, settings, submitAction, keyPressActions) {}

UIInputField::~UIInputField()
{
	//LogError("Input field destroyed");
}

const Input::InputManager& UIInputField::GetInputManager() const
{
	if (!Assert(this, m_inputManager != nullptr,
		std::format("Tried to retreive input manager from input field but it is NULLPTR")))
	{
		throw std::invalid_argument("Invalid input manager");
	}
	return *m_inputManager;
}

std::string UIInputField::CleanInput(const std::string& input) const
{
	return Utils::StringUtil(input).RemoveSpaces().ToString();
}

void UIInputField::UpdateInput(const float deltaTime)
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
		
		LogError(std::format("Before submit action input field type is: {}", ::ToString(GetFieldType())));
		if (m_submitAction != nullptr) m_submitAction(GetInput());
		
		LogError(std::format("AFTER submit action input field type is: {}", ::ToString(GetFieldType())));
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
	//if (keysPressed == "." && m_type == InputFieldType::Float) Assert(false, std::format("FOUND DOT"));
	SetAttemptedInputDelta(keysPressed);
}
void UIInputField::Update(const float deltaTime)
{
	if (m_textGUI == nullptr) return;
	UpdateInput(deltaTime);
	
	std::string inputStr = IsSelected() && !HasFlag(InputFieldFlag::UserUIReadonly) ? GetDisplayAttemptedInput() : GetDisplayInput();
	m_textGUI->SetText(inputStr);
}

void UIInputField::SetSubmitAction(const InputFieldAction& action) { m_submitAction = action; }
void UIInputField::SetKeyPressAction(const KeyboardKey key, const InputFieldAction& action)
{
	m_keyActions.emplace(key, action);
}

void UIInputField::SetSettings(const UIStyle& settings) 
{ 
	m_settings = settings; 
	if (m_textGUI != nullptr) m_textGUI->SetSettings(settings.m_TextSettings);
	if (m_background != nullptr) m_background->SetColor(settings.m_BackgroundColor);
}

void UIInputField::SetAttemptedInputDelta(const std::string& input)
{
	std::string cleanedInput = CleanInput(input);

	if (m_type == InputFieldType::Integer)
	{
		if (input.size() == 1) SetInput(m_attemptedInput + (std::isdigit(input[0])? input : ""), true);
		else SetInput(m_attemptedInput + Utils::TryExtractInt(input), true);
	}
	else if (m_type == InputFieldType::Float)
	{
		//LogError(std::format("Extracting float from input: {} is: {}", input, Utils::TryExtractFloat(input)));
		/*Assert(false, std::format("Extracting float from input: {} is: {} (OLD: {}) new: {}", 
			input, Utils::TryExtractFloat(input), m_input, m_input + Utils::TryExtractFloat(input)));*/
		//if (Utils::TryExtractFloat(input) == "-") Assert(false, std::format("Places neg"));
		//Assert(false, std::format("Chars pressed since last frame: {} extracted: {}", input, Utils::TryExtractFloat(input)));
		//if (input == "." && m_type == InputFieldType::Float) Assert(false, std::format("Attempt extracted float:{}", Utils::TryExtractFloat(".")));

		if (input.size() == 1) SetInput(m_attemptedInput + (std::isdigit(input[0]) || input[0]=='.'? input : ""), true);
		else SetInput(m_attemptedInput + Utils::TryExtractFloat(input), true);
	}
	else if (m_type == InputFieldType::String)
	{
		//Assert(false, std::format("Reached here: input:{}", input));
		SetInput(m_attemptedInput + Utils::TryExtractNonIntegralValues(input), true);
		//Assert(false, std::format("Finished setting input"));
	}
	else SetInput(m_attemptedInput + input, true);
}
void UIInputField::ResetInput() 
{ 
	m_input = ""; 
	m_attemptedInput = "";
}

void UIInputField::SetInput(const std::string& newInput, const bool isAttemptedInput)
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

void UIInputField::OverrideInput(const std::string& input)
{
	SetInput(input, false);
}

const InputFieldType& UIInputField::GetFieldType() const { return m_type; }

std::string UIInputField::GetDisplayInput() const
{
	if (HasFlag(InputFieldFlag::ShowCaret)) return m_input + "_";
	return m_input;
}
std::string UIInputField::GetDisplayAttemptedInput() const
{
	if (HasFlag(InputFieldFlag::ShowCaret)) return m_attemptedInput + "_";
	return m_attemptedInput;
}

const std::string& UIInputField::GetInput() const { return m_input; }
const std::string& UIInputField::GetLastInput() const { return m_lastInput; }
int UIInputField::GetIntInput() const { return std::stoi(m_input); }
float UIInputField::GetFloatInput() const { return std::stof(m_input); }

bool UIInputField::HasFlag(const InputFieldFlag& flag) const
{
	return Utils::HasFlagAll(m_inputFlags, flag);
}

/*
RenderInfo InputFieldGUI::ElementRender(const RenderInfo& renderInfo)
{
	//Assert(false, std::format("drawing field gui at:{}", renderInfo.ToString()));
	DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y, renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, m_settings.m_BackgroundColor);
	
	//LogError(std::format("Input str for render:{}", inputStr));
	//Assert(false, std::format("Found input: {}", inputStr));


	LogError("hello");
	//m_textGUI.Render(renderInfo);

	//Vector2 textStartPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos);
	//const float fontSize = m_settings.m_TextSettings.GetFontSize(renderSize);
	////if (m_settings.m_TextSettings.m_FontSizeType==TextGUIFontSize::ParentArea) 
	////Assert(false, std::format("Font size is: {} text factor: {}", std::to_string(fontSize), std::to_string(m_settings.m_TextSettings.m_FontSize)));

	//textStartPos.x += m_settings.m_TextSettings.m_RightIndent;
	//DrawTextEx(GetGlobalFont(), inputStr.c_str(), textStartPos,
	//	fontSize, DEBUG_INFO_CHAR_SPACING.m_X, m_settings.m_TextSettings.m_TextColor);

	//Note: if this object is deselected (even if is readonly) we do not draw disabled overlay
	//if (IsSelected() && HasFlag(InputFieldFlag::UserUIReadonly)) DrawDisabledOverlay(renderInfo);
	//LogError(std::format("reached end here yaya"));

	//SetLastFramneRect(GUIRect(renderInfo.m_TopLeftPos, renderInfo.m_RenderSize));
	//Assert(false, std::format("On render set last rect mutable; {}", GetLastFrameRect().ToString()));

	//m_lastRenderRect.SetSize(renderSize);
	//m_lastRenderRect.SetTopLeftPos(renderInfo.m_TopLeftPos);
	
	//LogError(std::format("Updating input field rect to: {}", m_lastRenderRect.ToString()));
	return renderInfo;
}
*/

void UIInputField::InitFields()
{
	m_Fields = {};
}

std::string UIInputField::ToString() const
{
	return std::format("[UIInputField]");
}

void UIInputField::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIInputField::Serialize()
{
	//TOD: implement
	return {};
}

//const GUIRect& InputField::GetLastRenderRect() const
//{
//	return m_lastRenderRect;
//}