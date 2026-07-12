#include <optional>
#include <cstdint>
#include "pch.hpp"
#include "Utils/Debug.hpp"
#include "Utils/StringUtil.hpp"
#include "StaticGlobals.hpp"
#include "Core/Primitives/Vector.hpp"
#include "ECS/Component/Types/UI/UIInputFieldComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	static constexpr Input::KeyCode SUBMIT_KEY = Input::KeyCode::Enter;
	static constexpr Input::KeyCode ESCAPE_KEY = Input::KeyCode::Escape;
	static constexpr Input::KeyCode DELETE_KEY = Input::KeyCode::Delete;
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

	UIInputFieldComponent::UIInputFieldComponent(const InputFieldType& type, const InputFieldFlag& flags, const UIStyle& settings,
		const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
		: m_inputManager(nullptr), m_type(type), m_input(), m_lastInput(),
		m_textGUI(nullptr), m_background(nullptr), m_selectable(nullptr), m_inputFlags(flags), m_submitAction(submitAction),
		m_keyActions(keyPressActions), m_settings(settings)
	{
		//LogError("Creating Input FIeld");
		//Assert(false, std::format("Created  gui redct: {}", GetLastFrameRect().ToString()));

	}

	//InputField::InputField() : InputField(nullptr, nullptr, InputFieldType::Any, InputFieldFlag::None, {}, nullptr, {}) 
	//{
	//	
	//}

	//InputFieldGUI::InputFieldGUI(const Input::InputManager& manager,
	//	const InputFieldType& type, const InputFieldFlag& flags, const GUISettings& settings,
	//	const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions)
	//	:InputFieldGUI(&manager, type, flags, settings, submitAction, keyPressActions) {}

	UIInputFieldComponent::~UIInputFieldComponent()
	{
		//LogError("Input field destroyed");
	}

	void UIInputFieldComponent::Init(const Input::InputManager& input)
	{
		m_inputManager = &input;
		if (HasFlag(InputFieldFlag::SelectOnStart)) m_selectable->Select();
		if (HasFlag(InputFieldFlag::UserUIReadonly)) m_selectable->AddRenderFlags(InteractionRenderFlags::DrawDisabledOverlay);
		//We have to call this during init when we have background panel already set
		UpdateStyle();
	}

	const Input::InputManager& UIInputFieldComponent::GetInputManager() const
	{
		ENGINE_ASSERT(m_inputManager != nullptr,
			"Tried to retreive input manager from input field but it is NULLPTR");
		return *m_inputManager;
	}

	std::string UIInputFieldComponent::CleanInput(const std::string& input) const
	{
		return ::Utils::StringUtil(input).RemoveSpaces().ToString();
	}

	void UIInputFieldComponent::UpdateInput()
	{
		if (m_selectable->IsSelected() && GetInputManager().GetInputKey(ESCAPE_KEY)->GetState().IsReleased())
		{
			m_selectable->Deselect();
			m_attemptedInput = "";
			//Assert(false, "ESCAPIGN FROMR INTPUT");
			return;
		}

		if (!m_selectable->IsSelected()) return;

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

			if (!HasFlag(InputFieldFlag::KeepSelectedOnSubmit)) m_selectable->Deselect();

			//LogError(std::format("Before submit action input field type is: {}", ::ToString(GetFieldType())));
			if (m_submitAction != nullptr) m_submitAction(GetInput());

			//LogError(std::format("AFTER submit action input field type is: {}", ::ToString(GetFieldType())));
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
	void UIInputFieldComponent::Update()
	{
		if (m_textGUI == nullptr) return;
		UpdateInput();

		std::string inputStr = m_selectable->IsSelected() && !HasFlag(InputFieldFlag::UserUIReadonly) ? GetDisplayAttemptedInput() : GetDisplayInput();
		m_textGUI->SetText(inputStr);
	}

	void UIInputFieldComponent::SetSubmitAction(const InputFieldAction& action) { m_submitAction = action; }
	void UIInputFieldComponent::SetKeyPressAction(const Input::KeyCode key, const InputFieldAction& action)
	{
		m_keyActions.emplace(key, action);
	}

	void UIInputFieldComponent::SetSettings(const UIStyle& settings)
	{
		m_settings = settings;
		UpdateStyle();
	}

	void UIInputFieldComponent::UpdateStyle()
	{
		if (m_textGUI != nullptr) m_textGUI->SetSettings(m_settings.m_TextSettings);
		if (m_background != nullptr) m_background->SetColor(m_settings.m_BackgroundColor);
	}

	void UIInputFieldComponent::SetAttemptedInputDelta(const std::string& input)
	{
		std::string cleanedInput = CleanInput(input);

		if (m_type == InputFieldType::Integer)
		{
			if (input.size() == 1) SetInput(m_attemptedInput + (std::isdigit(input[0]) ? input : ""), true);
			else SetInput(m_attemptedInput + ::Utils::TryExtractInt(input), true);
		}
		else if (m_type == InputFieldType::Float)
		{
			//LogError(std::format("Extracting float from input: {} is: {}", input, ::Utils::TryExtractFloat(input)));
			/*Assert(false, std::format("Extracting float from input: {} is: {} (OLD: {}) new: {}",
				input, ::Utils::TryExtractFloat(input), m_input, m_input + ::Utils::TryExtractFloat(input)));*/
				//if (::Utils::TryExtractFloat(input) == "-") Assert(false, std::format("Places neg"));
				//Assert(false, std::format("Chars pressed since last frame: {} extracted: {}", input, ::Utils::TryExtractFloat(input)));
				//if (input == "." && m_type == InputFieldType::Float) Assert(false, std::format("Attempt extracted float:{}", ::Utils::TryExtractFloat(".")));

			if (input.size() == 1) SetInput(m_attemptedInput + (std::isdigit(input[0]) || input[0] == '.' ? input : ""), true);
			else SetInput(m_attemptedInput + ::Utils::TryExtractFloat(input), true);
		}
		else if (m_type == InputFieldType::String)
		{
			//Assert(false, std::format("Reached here: input:{}", input));
			SetInput(m_attemptedInput + ::Utils::TryExtractNonIntegralValues(input), true);
			//Assert(false, std::format("Finished setting input"));
		}
		else SetInput(m_attemptedInput + input, true);
	}
	void UIInputFieldComponent::ResetInput()
	{
		m_input = "";
		m_attemptedInput = "";
	}

	void UIInputFieldComponent::SetInput(const std::string& newInput, const bool isAttemptedInput)
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

	void UIInputFieldComponent::OverrideInput(const std::string& input)
	{
		SetInput(input, false);
	}

	const InputFieldType& UIInputFieldComponent::GetFieldType() const { return m_type; }

	std::string UIInputFieldComponent::GetDisplayInput() const
	{
		if (HasFlag(InputFieldFlag::ShowCaret)) return m_input + "_";
		return m_input;
	}
	std::string UIInputFieldComponent::GetDisplayAttemptedInput() const
	{
		if (HasFlag(InputFieldFlag::ShowCaret)) return m_attemptedInput + "_";
		return m_attemptedInput;
	}

	const std::string& UIInputFieldComponent::GetInput() const { return m_input; }
	const std::string& UIInputFieldComponent::GetLastInput() const { return m_lastInput; }
	int UIInputFieldComponent::GetIntInput() const { return std::stoi(m_input); }
	float UIInputFieldComponent::GetFloatInput() const { return std::stof(m_input); }

	bool UIInputFieldComponent::HasFlag(const InputFieldFlag& flag) const
	{
		return ::Utils::HasFlagAll(m_inputFlags, flag);
	}

	void UIInputFieldComponent::InitFields()
	{
		m_Fields = {};
	}
	void UIInputFieldComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UIInputFieldComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}

	std::string UIInputFieldComponent::ToString() const
	{
		return std::format("[UIInputField]");
	}
}
