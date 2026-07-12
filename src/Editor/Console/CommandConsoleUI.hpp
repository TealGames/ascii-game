#pragma once
#include <unordered_map>
#include <string>
#include "Core/Debug/CommandController.hpp"
#include "Core/Primitives/Color.hpp"

namespace Engine::UI
{
	class UIHierarchy;
	class UITransformComponent;
	class UITextComponent;
	class UIInputFieldComponent;
	class UILayoutComponent;
}
namespace Engine::Input { class InputManager; }
namespace Engine::Editor::UI
{
	namespace MainUI = Engine::UI;
	namespace MainDebug = Engine::Debug;

	enum class ConsoleUIOutputMessageType : std::uint8_t
	{
		Default		= 0,
		Error		= 1,
		Success		= 2
	};

	constexpr int MAX_OUTPUT_MESSAGES = 10;
	class CommandConsoleUI
	{
	private:
		MainDebug::CommandController& m_commandController;

		MainUI::UITransformComponent* m_container;
		MainUI::UIInputFieldComponent* m_inputField;
		MainUI::UILayoutComponent* m_outputMessageLayout;
		std::array<MainUI::UITextComponent*, MAX_OUTPUT_MESSAGES> m_outputMessagesTextGuis;
		std::uint8_t m_nextTextGuiIndex;
		std::vector<float> m_messageCloseTimes;

		const Input::InputManager& m_inputManager;
		float m_timeSinceOpen;
		bool m_isEnabled;
	public:

	private:
		bool TryInvokeCommandFromInputField();
		ColHDR4 GetColorFromMessageType(const ConsoleUIOutputMessageType& message);
		void SetNextMessage(const std::string& message, const ColHDR4 color);
		void RemoveBackMessage();
	public:
		CommandConsoleUI(MainDebug::CommandController& commandController,
			const Input::InputManager& input);

		void CreateUI(MainUI::UIHierarchy& hierarchy);

		void Update(const float deltaTime);
		bool IsEnabled() const;
		void ResetInput();

		void LogOutputMessage(const std::string& message, const ConsoleUIOutputMessageType& messageType);
		/// <summary>
		/// Will log the messages and will show all of them even if a limit would have
		/// caused a message to be hidden
		/// </summary>
		/// <param name="messages"></param>
		void LogOutputMessages(const std::vector<std::string>& messages, const ConsoleUIOutputMessageType& messageType);
	};
}


