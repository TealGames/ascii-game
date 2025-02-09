#include "pch.hpp"
#include "CommandConsole.hpp"
#include "StringUtil.hpp"
#include "HelperFunctions.hpp"
#include "CommandPromptType.hpp"
#include "Debug.hpp"

static constexpr int MAX_OUTPUT_MESSAGES = 10;
static constexpr float MESSAGE_DISPLAY_TIME_SECONDS = 4;

CommandConsole::CommandConsole() : m_prompts(), m_isEnabled(false), m_input(), m_outputMessages() 
{

}

std::string CommandConsole::FormatPromptName(const std::string& name)
{
	return Utils::StringUtil(name).Trim().ToLowerCase().ToString();
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

	it->second.push_back(prompt);
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
	std::string formattedPrompt = FormatPromptName(m_input);
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
			const std::string message = std::format("Console command: {} has been successfully executed", m_input);
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
	std::string formattedPrompt = FormatPromptName(m_input);
	auto it = TryGetIteratorForPromptName(formattedPrompt);

	for (const auto& promptOverload : it->second)
	{
		docs.push_back(promptOverload->GetDocumentation());
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
			docs.push_back(promptOverload->GetDocumentation());
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

void CommandConsole::Update()
{
	if (IsKeyPressed(KEY_ENTER))
	{
		TryInvokePrompt();
		ResetInput();
		return;
	}

	if (IsKeyPressed(KEY_BACKSPACE) && !m_input.empty())
	{
		m_input.pop_back();
	}

	int key = GetKeyPressed();
	while (key > 0)
	{
		//8 is backspace, 32-126 is the range of numers/letters
		if (key >= 32 && key <= 126)
		{
			m_input += static_cast<char>(key);
		}
		key = GetKeyPressed();
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

void CommandConsole::ResetInput()
{
	m_input.clear();
}
std::string CommandConsole::GetInput() const
{
	return m_input + "_";
}
const std::vector<ConsoleOutputMessage>& CommandConsole::GetOutputMessages() const
{
	return m_outputMessages;
}
