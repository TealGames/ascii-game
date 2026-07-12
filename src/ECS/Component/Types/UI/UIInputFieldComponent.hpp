#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include "Core/Input/InputManager.hpp"
#include "ECS/Component/Component.hpp"
//#include "raylib.h"
#include "Core/UI/UIRect.hpp"
#include "Core/UI/UIStyle.hpp"

namespace Engine::UI
{
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
		None = 0,
		/// <summary>
		/// If true, will display the caret symbol when typing
		/// </summary>
		ShowCaret = 1 << 1,
		/// <summary>
		/// If true, the field will automatically be selected
		/// for accepting input
		/// </summary>
		SelectOnStart = 1 << 2,
		/// <summary>
		/// If true will keep the field selected even on submit action
		/// </summary>
		KeepSelectedOnSubmit = 1 << 3,
		/// <summary>
		/// If true, this input field CAN still be selected, but 
		/// values CAN NOT be changed BY THE USER (functions still work but 
		/// clicking on it can not be changed from UI)
		/// </summary>
		UserUIReadonly = 1 << 4
	};
	FLAG_ENUM_OPERATORS(InputFieldFlag)

	constexpr InputFieldFlag INPUT_FIELD_DEFAULT_FLAGS = InputFieldFlag::ShowCaret;
	constexpr InputFieldType INPUT_FIELD_DEFAULT_TYPE = InputFieldType::String;

	using InputFieldAction = std::function<void(std::string input)>;
	using InputFieldKeyActions = std::unordered_map<Input::KeyCode, InputFieldAction>;

	class UITextComponent;
	class UIPanelComponent;
	class UISelectableComponent;
	class UIInputFieldSystem;
	class UIInputFieldComponent : public ECS::Component
	{
	private:
		InputFieldType m_type;

		std::string m_input;
		std::string m_lastInput;
		std::string m_attemptedInput;
		InputFieldFlag m_inputFlags;

		UISelectableComponent* m_selectable;
		UITextComponent* m_textGUI;
		UIPanelComponent* m_background;

		//bool m_isSelected;
		InputFieldAction m_submitAction;
		InputFieldKeyActions m_keyActions;

		UIStyle m_settings;

		const Input::InputManager* m_inputManager;
	public:
		friend class UIInputFieldSystem;

	private:
		/*InputFieldGUI(const Input::InputManager* manager, const InputFieldType& type, const InputFieldFlag& flags,
			const GUISettings& settings, const InputFieldAction& submitAction, const InputFieldKeyActions& keyPressActions);*/

		void Init(const Input::InputManager& input);
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

		void UpdateInput();
		void UpdateStyle();

	public:
		UIInputFieldComponent(const InputFieldType& type = INPUT_FIELD_DEFAULT_TYPE, 
			const InputFieldFlag& flags = INPUT_FIELD_DEFAULT_FLAGS, const UIStyle& settings = UIStyle(),
			const InputFieldAction& submitAction = nullptr, const InputFieldKeyActions& keyPressActions = {});
		~UIInputFieldComponent();

		void Update();

		bool HasFlag(const InputFieldFlag& flag) const;
		const InputFieldType& GetFieldType() const;

		void SetSubmitAction(const InputFieldAction& action);
		void SetKeyPressAction(const Input::KeyCode key, const InputFieldAction& action);
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
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


