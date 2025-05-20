#include "pch.hpp"
#include "CommandConsole.hpp"
#include "StringUtil.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"
#include "RaylibUtils.hpp"
#include "GUISelectorManager.hpp"
#include "GUIHierarchy.hpp"

static constexpr int MAX_OUTPUT_MESSAGES = 10;
static constexpr float MESSAGE_DISPLAY_TIME_SECONDS = 4;

static constexpr KeyboardKey LAST_COMMAND_KEY = KEY_ONE;

static const Color CONSOLE_COLOR = { GRAY.r, GRAY.g, GRAY.b, 100 };

static constexpr int COMMAND_CONSOLE_HEIGHT = 20;
static constexpr int COMMAND_CONSOLE_WIDTH = SCREEN_WIDTH;
static constexpr int COMMAND_CONSOLE_FONT_SIZE = 25;
static constexpr int COMMAND_CONSOLE_SPACING = 3;
static constexpr float COMMAND_CONSOLE_OUPUT_FONT_SIZE = 10;
static constexpr int COMMAND_CONSOLE_TEXT_INDENT = 10;

CommandConsole::CommandConsole(const Input::InputManager& input, GUIHierarchy& hierarchy, GUISelectorManager& selector) :
	m_inputManager(input), m_prompts(), m_outputMessages(), 
	m_inputField(input, InputFieldType::Any, 
		InputFieldFlag::SelectOnStart | InputFieldFlag::ShowCaret | InputFieldFlag::KeepSelectedOnSubmit, GUIStyle()), 
	m_isEnabled(false)
{
	GUIStyle fieldSettings = GUIStyle(GRAY, TextGUIStyle(WHITE, FontProperties(COMMAND_CONSOLE_FONT_SIZE, COMMAND_CONSOLE_SPACING, GetGlobalFont()), 
		TextAlignment::TopLeft, GUIPadding(COMMAND_CONSOLE_TEXT_INDENT)));
	m_inputField.SetSettings(fieldSettings);
	m_inputField.SetSubmitAction([this](std::string input) -> void
		{
			TryInvokePrompt();
			ResetInput();
		});
	m_inputField.SetKeyPressAction(LAST_COMMAND_KEY, [this](std::string input) -> void
		{
			//Assert(false, std::format("Triggering stuff last input: {}", m_inputField.GetLastInput()));
			//TODO: this does not work because we override underlying input and not attempted input
			m_inputField.OverrideInput(m_inputField.GetLastInput());
		});
	selector.AddSelectable(DEFAULT_LAYER, &m_inputField);

	DebugProperties::OnMessageLogged.AddListener(
		[this](const LogType& logType, const std::string& message, const bool& logToConsole, const bool& pause)-> void
		{
			if (!logToConsole) return;

			ConsoleOutputMessageType messageType = ConsoleOutputMessageType::Default;
			if ((logType & LogType::Error)!=LogType::None) messageType = ConsoleOutputMessageType::Error;

			LogOutputMessage(message, messageType);
		});

	hierarchy.AddToRoot(DEFAULT_LAYER, this);
}

std::string CommandConsole::FormatPromptName(const std::string& name)
{
	return Utils::StringUtil(name).TrimSpaces().ToLowerCase().ToString();
}

PromptCollection::iterator CommandConsole::TryGetIteratorForPromptName(const std::string& name)
{
	return m_prompts.find(name);
}

void CommandConsole::AddPrompt(ICommandPrompt* prompt)
{
	if (prompt == nullptr) return;

	const std::string formattedName = FormatPromptName(prompt->GetCommandName());
	auto it = TryGetIteratorForPromptName(formattedName);
	if (it == m_prompts.end()) it = m_prompts.emplace(formattedName, std::vector<ICommandPrompt*>{}).first;

	it->second.emplace_back(prompt);
}

std::string CommandConsole::GetDocumentationForPrompt(PromptCollection::iterator& promptIt, const size_t& index) const
{
	if (promptIt == m_prompts.end()) return "";
	if (index <0 || index >= promptIt->second.size()) return "";

	return promptIt->second[index]->GetDocumentation();
}

bool CommandConsole::HasPrompt(const std::string& promptName)
{
	return TryGetIteratorForPromptName(promptName) != m_prompts.end();
}

void CommandConsole::DeletePrompts()
{
	for (auto& prompt : m_prompts)
	{
		for (auto& promptOverload : prompt.second)
		{
			delete promptOverload;
		}
		prompt.second.clear();
	}
}

bool CommandConsole::TryInvokePrompt()
{
	std::string formattedPrompt = FormatPromptName(m_inputField.GetInput());
	std::vector<std::string> promptSegments = Utils::Split(formattedPrompt, ' ');

	if (!Assert(this, !promptSegments.empty(), 
		std::format("Tried to check input for command prompt but found no prompt segments!")))
		return false;

	if (!Assert(this, !promptSegments[0].empty(),
		std::format("Tried to check input for command prompt but first prompt segment is empty")))
		return false;
	
	if (promptSegments[0][0] != COMMAND_CHAR)
	{
		LogOutputMessage(std::format("Command symbol: '{}' was not found", 
			Utils::ToString(COMMAND_CHAR)), ConsoleOutputMessageType::Error);
		return false;
	}

	std::string promptName = promptSegments[0].substr(1);
	promptSegments.erase(promptSegments.begin());

	auto it = TryGetIteratorForPromptName(promptName);
	if (it == m_prompts.end())
	{
		LogOutputMessage(std::format("No command matches name: {}", promptName), ConsoleOutputMessageType::Error);
		return false;
	}

	//TODO: i don't know why but for some reason for variadic template args to recgonize args provided by command
	//must be reversed. the args and types provided by templates must be in opposite orders to work- WHY?
	std::vector<std::string> promptReversed(promptSegments.size());
	std::reverse_copy(promptSegments.begin(), promptSegments.end(), promptReversed.begin());
	for (const auto& prompt : it->second)
	{
		if (prompt->TryInvokeAction(promptReversed))
		{
			const std::string message = std::format("Console command: {} has been successfully executed", m_inputField.GetInput());
			LogOutputMessage(message, ConsoleOutputMessageType::Success);
			Log(message);
			return true;
		}
	}
	LogOutputMessage(std::format("No command matches name: {} ({} overloads) and args: {}", promptName,
		std::to_string(it->second.size()),
		Utils::ToStringIterable<std::vector<std::string>, std::string>(promptSegments)), ConsoleOutputMessageType::Error);
	return false;
	//TODO: we probably want to print some errors if not prompt was found DIRECTLY in the console
}

std::vector<std::string> CommandConsole::GetPromptDocumentation(const std::string& promptName)
{
	std::vector<std::string> docs = {};
	std::string formattedPrompt = FormatPromptName(m_inputField.GetInput());
	auto it = TryGetIteratorForPromptName(formattedPrompt);

	for (const auto& promptOverload : it->second)
	{
		docs.emplace_back(promptOverload->GetDocumentation());
	}

	return docs;
}

std::vector<std::string> CommandConsole::GetPromptDocumentationAll()
{
	std::vector<std::string> docs = {};
	for (const auto& prompt : m_prompts)
	{
		for (const auto& promptOverload : prompt.second)
		{
			docs.emplace_back(promptOverload->GetDocumentation());
		}
	}
	return docs;
}

Color CommandConsole::GetColorFromMessageType(const ConsoleOutputMessageType& message)
{
	if (message == ConsoleOutputMessageType::Error) return RED;
	else if (message == ConsoleOutputMessageType::Success) return GREEN;
	else return WHITE;
}
void CommandConsole::LogOutputMessage(const std::string& message, const ConsoleOutputMessageType& messageType)
{
	m_outputMessages.emplace(m_outputMessages.begin(), message, GetColorFromMessageType(messageType), std::chrono::high_resolution_clock::now());
	while (m_outputMessages.size() > MAX_OUTPUT_MESSAGES)
	{
		m_outputMessages.pop_back();
	}
}

void CommandConsole::LogOutputMessagesUnrestricted(const std::vector<std::string>& messages, const ConsoleOutputMessageType& messageType)
{
	Color color = GetColorFromMessageType(messageType);
	for (const auto& message : messages)
	{
		m_outputMessages.emplace(m_outputMessages.begin(), 
			message, color, std::chrono::high_resolution_clock::now());
	}
}

void CommandConsole::Update(const float deltaTime)
{
	if (m_inputManager.IsKeyPressed(TOGGLE_COMMAND_CONSOLE_KEY))
	{
		m_isEnabled = !m_isEnabled;
	}

	if (!m_isEnabled)
	{
		ResetInput();
		return;
	}

	if (m_outputMessages.empty()) return;

	Time currentTime = std::chrono::high_resolution_clock::now();
	for (int i= m_outputMessages.size()-1; i>=0; i--)
	{
		float secondsPassed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>
			(currentTime - m_outputMessages[i].m_StartTime).count()) / 1000;

		if (secondsPassed >= MESSAGE_DISPLAY_TIME_SECONDS)
		{
			m_outputMessages.pop_back();
		}
	}
}

//void CommandConsole::TryRender()
//{
//	Render(RenderInfo({ 0, SCREEN_HEIGHT - COMMAND_CONSOLE_HEIGHT }, { SCREEN_WIDTH, COMMAND_CONSOLE_HEIGHT }));
//}

RenderInfo CommandConsole::Render(const RenderInfo& renderInfo)
{
	if (!m_isEnabled) return {};

	//float consoleIndent = 10;
	Vector2 currentPos = RaylibUtils::ToRaylibVector(renderInfo.m_TopLeftPos); //{ 0, SCREEN_HEIGHT - COMMAND_CONSOLE_HEIGHT };
	ScreenPosition totalSize = {};
	//Color consoleColor = ;
	//consoleColor.a = 100;
	//DrawRectangle(currentPos.x, currentPos.y, renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, consoleColor);

	//DrawTextEx(GetGlobalFont(), GetInput().c_str(), { currentPos.x + consoleIndent, currentPos.y }, COMMAND_CONSOLE_FONT_SIZE, COMMAND_CONSOLE_SPACING, WHITE);
	ScreenPosition fieldSize = m_inputField.Render(renderInfo).m_RenderSize;
	currentPos.y -= fieldSize.m_Y;
	totalSize = fieldSize;

	const auto& outputMessages = GetOutputMessages();
	Vector2 currentMessageSize = {};
	for (int i = 0; i < outputMessages.size(); i++)
	{
		currentMessageSize = MeasureTextEx(GetGlobalFont(), outputMessages[i].m_Message.c_str(), COMMAND_CONSOLE_OUPUT_FONT_SIZE, COMMAND_CONSOLE_SPACING);
		totalSize = totalSize+ ScreenPosition(static_cast<int>(currentMessageSize.x), static_cast<int>(currentMessageSize.y));

		DrawTextEx(GetGlobalFont(), outputMessages[i].m_Message.c_str(), { currentPos.x + COMMAND_CONSOLE_TEXT_INDENT, currentPos.y },
			COMMAND_CONSOLE_OUPUT_FONT_SIZE, COMMAND_CONSOLE_SPACING, outputMessages[i].m_Color);
		currentPos.y -= currentMessageSize.y;
	}

	return { renderInfo.m_RenderSize, totalSize};
}

bool CommandConsole::IsEnabled() const { return m_isEnabled; }
void CommandConsole::ResetInput() { m_inputField.ResetInput(); }
//std::string CommandConsole::GetInput() const
//{
//	return m_input + "_";
//}
const std::vector<ConsoleOutputMessage>& CommandConsole::GetOutputMessages() const
{
	return m_outputMessages;
}
