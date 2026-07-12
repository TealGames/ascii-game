#include "pch.hpp"
#include "Editor/Console/CommandConsoleUI.hpp"
#include "Utils/StringUtil.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Editor/EditorStyles.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIInputFieldComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "StaticGlobals.hpp"
#include "Core/Input/InputManager.hpp"

namespace Engine::Editor::UI
{
	static constexpr float MESSAGE_DISPLAY_TIME_SECONDS = 4;
	static constexpr Input::KeyCode LAST_COMMAND_KEY = Input::KeyCode::Num1;

	static const ColHDR4 CONSOLE_COLOR = { COLOR_GRAY, 100 / 255.0f };
	static constexpr float CONSOLE_HEIGHT = 0.05;
	static const NormalizedVec2 OUTPUT_MESSAGE_AREA = { 0.6, 0.2 };

	static constexpr int COMMAND_CONSOLE_WIDTH = SCREEN_WIDTH;
	static constexpr int COMMAND_CONSOLE_FONT_SIZE = 25;
	static constexpr int COMMAND_CONSOLE_SPACING = 3;
	static constexpr float COMMAND_CONSOLE_OUPUT_FONT_SIZE = 10;
	static constexpr int COMMAND_CONSOLE_TEXT_INDENT = 10;
	static constexpr std::uint16_t MESSAGE_MAX_LENGTH = 50;

	static constexpr char COMMAND_CHAR = '/';
	static constexpr Input::KeyCode TOGGLE_COMMAND_CONSOLE_KEY = Input::KeyCode::Tab;

	CommandConsoleUI::CommandConsoleUI(MainDebug::CommandController& commandController, const Input::InputManager& input) :
		m_commandController(commandController), m_inputManager(input), m_messageCloseTimes(),
		m_inputField(nullptr), m_outputMessageLayout(nullptr), m_container(nullptr),
		m_outputMessagesTextGuis(::Utils::ConstructArray<MainUI::UITextComponent*, MAX_OUTPUT_MESSAGES>()),
		m_nextTextGuiIndex(0), m_timeSinceOpen(0), m_isEnabled(false)
	{
	}

	void CommandConsoleUI::CreateUI(MainUI::UIHierarchy& hierarchy)
	{
		ECS::EntityData* containerEntity = nullptr;
		std::tie(containerEntity, m_container) = hierarchy.CreateAtRoot(MainUI::DEFAULT_LAYER, "CommandConsoleUIContainer");
		m_container->SetMaxRelativeSize();

		ECS::EntityData* inputFieldEntity = nullptr;
		MainUI::UITransformComponent* inputFieldTransform = nullptr;
		std::tie(inputFieldEntity, inputFieldTransform, m_inputField) = containerEntity->CreateChildUI("ConsoleInput",
			MainUI::UIInputFieldComponent(MainUI::InputFieldType::Any,
				MainUI::InputFieldFlag::SelectOnStart | MainUI::InputFieldFlag::ShowCaret | MainUI::InputFieldFlag::KeepSelectedOnSubmit,
				Styles::GetInputFieldStyle(MainUI::TextAlignment::CenterLeft)));
		//GUIStyle fieldSettings = GUIStyle(GRAY, TextGUIStyle(WHITE, FontProperties(COMMAND_CONSOLE_FONT_SIZE, COMMAND_CONSOLE_SPACING, GetGlobalFont()), 
		//	TextAlignment::TopLeft, GUIPadding(COMMAND_CONSOLE_TEXT_INDENT)));
		inputFieldTransform->SetLocalBoundsTLBR({ 0, CONSOLE_HEIGHT }, MainUI::UI_RECT_BOTTOM_RIGHT);
		m_inputField->SetSubmitAction([this](std::string input) -> void
			{
				TryInvokeCommandFromInputField();
				ResetInput();
			});
		m_inputField->SetKeyPressAction(LAST_COMMAND_KEY, [this](std::string input) -> void
			{
				//Assert(false, std::format("Triggering stuff last input: {}", m_inputField.GetLastInput()));
				//TODO: this does not work because we override underlying input and not attempted input
				m_inputField->OverrideInput(m_inputField->GetLastInput());
			});

		auto [layoutEntity, layoutTransform] = containerEntity->CreateChildUI("ConsoleLayout");
		m_outputMessageLayout = &(layoutEntity->AddComponent(MainUI::UILayoutComponent(MainUI::LayoutType::Vertical, MainUI::SizingType::ExpandAndShrink)));

		const NormalizedVec2 messageLayoutTopLeft = inputFieldTransform->GetLocalRect().GetTopLeftPos() + NormalizedVec2(0, OUTPUT_MESSAGE_AREA.m_Y);
		layoutTransform->SetLocalBoundsTLBR(messageLayoutTopLeft, { messageLayoutTopLeft.m_X + OUTPUT_MESSAGE_AREA.m_X,
			inputFieldTransform->GetLocalRect().GetTopLeftPos().m_Y });

		for (size_t i = 0; i < m_outputMessagesTextGuis.size(); i++)
		{
			auto [textEntity, textTransform] = layoutEntity->CreateChildUI(std::format("OutputText{}", std::to_string(i)));
			m_outputMessagesTextGuis[i] = &(textEntity->AddComponent(MainUI::UITextComponent("", 
				Styles::GetTextStyleFactorSize(MainUI::TextAlignment::CenterLeft))));
			textTransform->SetFixed(true, false);
		}

		DebugProperties::OnMessageLogged.AddListener(
			[this](const LogType& logType, std::string message, const bool hasEventFlag)-> void
			{
				if (!hasEventFlag || logType != LogType::Error) return;

				ConsoleUIOutputMessageType messageType = ConsoleUIOutputMessageType::Default;
				if ((logType & LogType::Error) != LogType::None) messageType = ConsoleUIOutputMessageType::Error;
				if (message.size() > MESSAGE_MAX_LENGTH) message = message.substr(0, MESSAGE_MAX_LENGTH) + "...";
				LogOutputMessage(message, messageType);
			});

		//LogError(std::format("After creating ui created container: {} inputfield:{}", m_container->ToString(), inputFieldTransform->ToString()));
	}

	bool CommandConsoleUI::TryInvokeCommandFromInputField()
	{
		return m_commandController.TryInvokeCommand(m_inputField->GetInput()) 
			== Debug::CommandInvocationResult::Success;
	}

	ColHDR4 CommandConsoleUI::GetColorFromMessageType(const ConsoleUIOutputMessageType& message)
	{
		if (message == ConsoleUIOutputMessageType::Error) return COLOR_RED;
		else if (message == ConsoleUIOutputMessageType::Success) return COLOR_YELLOW;
		else return COLOR_WHITE;
	}
	void CommandConsoleUI::LogOutputMessage(const std::string& message, const ConsoleUIOutputMessageType& messageType)
	{
		while (m_nextTextGuiIndex >= MAX_OUTPUT_MESSAGES)
		{
			RemoveBackMessage();
		}

		SetNextMessage(message, GetColorFromMessageType(messageType));
	}

	void CommandConsoleUI::LogOutputMessages(const std::vector<std::string>& messages, const ConsoleUIOutputMessageType& messageType)
	{
		const ColHDR4 color = GetColorFromMessageType(messageType);

		m_nextTextGuiIndex = 0;
		for (size_t i = 0; i < messages.size() && i < MAX_OUTPUT_MESSAGES; i++)
		{
			SetNextMessage(messages[i], color);
		}
	}
	void CommandConsoleUI::SetNextMessage(const std::string& message, const ColHDR4 color)
	{
		m_messageCloseTimes.emplace(m_messageCloseTimes.begin(), m_timeSinceOpen + MESSAGE_DISPLAY_TIME_SECONDS);
		m_outputMessagesTextGuis[m_nextTextGuiIndex]->SetText(message);
		m_outputMessagesTextGuis[m_nextTextGuiIndex]->SetTextColor(color);
		m_outputMessagesTextGuis[m_nextTextGuiIndex]->GetEntityMutable().TryActivateEntity();

		m_nextTextGuiIndex++;
	}
	void CommandConsoleUI::RemoveBackMessage()
	{
		if (m_nextTextGuiIndex == 0) return;

		m_messageCloseTimes.pop_back();
		m_outputMessagesTextGuis[m_nextTextGuiIndex - 1]->SetText("");
		m_outputMessagesTextGuis[m_nextTextGuiIndex - 1]->GetEntityMutable().DeactivateEntity();

		m_nextTextGuiIndex--;
	}

	void CommandConsoleUI::Update(const float scaledDeltaTime)
	{
		if (m_inputManager.IsKeyPressed(TOGGLE_COMMAND_CONSOLE_KEY))
		{
			m_isEnabled = !m_isEnabled;
			if (m_isEnabled)
			{
				m_timeSinceOpen = 0;
				ResetInput();
			}
			m_container->GetEntityMutable().TrySetEntityActive(m_isEnabled);
		}
		if (!m_isEnabled || m_messageCloseTimes.empty()) return;

		//Time currentTime = std::chrono::high_resolution_clock::now(); 
		m_timeSinceOpen += scaledDeltaTime;
		for (int i = m_messageCloseTimes.size() - 1; i >= 0; i--)
		{
			/*float secondsPassed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>
				(currentTime - m_messageTimes[i]).count()) / 1000;*/

			if (m_timeSinceOpen < m_messageCloseTimes[i]) continue;
			RemoveBackMessage();
		}
	}

	bool CommandConsoleUI::IsEnabled() const { return m_isEnabled; }
	void CommandConsoleUI::ResetInput() { m_inputField->ResetInput(); }
}
