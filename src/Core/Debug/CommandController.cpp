#include "Core/Debug/CommandController.hpp"

namespace Engine::Debug
{
	CommandController::CommandController(bool debugLogOutputMessages) :
		m_commands(), m_doLogOutputMessages(debugLogOutputMessages)
	{
	}

	CommandController::~CommandController()
	{
		DeleteCommands();
	}

	std::string CommandController::FormatCommandName(const std::string& name)
	{
		return ::Utils::StringUtil(name).TrimSpaces().ToLowerCase().ToString();
	}

	CommandCollection::iterator CommandController::TryGetIteratorForCommandName(const std::string& name)
	{
		return m_commands.find(name);
	}

	void CommandController::AddCommand(IActionCommand* command)
	{
		if (command == nullptr) return;

		const std::string formattedName = FormatCommandName(command->GetCommandName());
		auto it = TryGetIteratorForCommandName(formattedName);
		if (it == m_commands.end()) it = m_commands.emplace(formattedName, std::vector<IActionCommand*>{}).first;

		it->second.emplace_back(command);
	}

	std::string CommandController::GetDocumentationForCommand(CommandCollection::iterator& commandIt, const size_t& index) const
	{
		if (commandIt == m_commands.end()) return "";
		if (index < 0 || index >= commandIt->second.size()) return "";

		return commandIt->second[index]->GetDocumentation();
	}

	bool CommandController::HasCommand(const std::string& commandName)
	{
		return TryGetIteratorForCommandName(commandName) != m_commands.end();
	}

	void CommandController::DeleteCommands()
	{
		for (auto& command : m_commands)
		{
			for (auto& commandOverload : command.second)
			{
				delete commandOverload;
			}
			command.second.clear();
		}
	}

	CommandInvocationResult CommandController::TryInvokeCommand(const std::string& commandStr)
	{
		std::string formattedCommand = FormatCommandName(commandStr);
		std::vector<std::string> commandSegments = ::Utils::Split(formattedCommand, ' ');

		if (commandSegments.empty())
		{
			LogOutput("[Command]: is empty", LogType::Warning);
			return CommandInvocationResult::EmptyCommand;
		}

		if (commandSegments[0].empty())
		{
			LogOutput("[Command]: contains no command name", LogType::Warning);
			return CommandInvocationResult::NoCommandName;
		}

		std::string commandName = commandSegments[0].substr(1);
		commandSegments.erase(commandSegments.begin());

		auto it = TryGetIteratorForCommandName(commandName);
		if (it == m_commands.end())
			return CommandInvocationResult::InvalidName;

		//TODO: i don't know why but for some reason for variadic template args to recgonize args provided by command
		//must be reversed. the args and types provided by templates must be in opposite orders to work- WHY?
		std::vector<std::string> commandReversed(commandSegments.size());
		std::reverse_copy(commandSegments.begin(), commandSegments.end(), commandReversed.begin());
		for (const auto& command : it->second)
		{
			if (command->TryInvokeAction(commandReversed))
			{
				const std::string message = std::format("[Command]: '{}' has been successfully executed", commandStr);
				LogOutput(message, LogType::Log);
				return CommandInvocationResult::Success;
			}
		}
		LogOutput(std::format("[Command]: No command matches name: '{}' ({} overloads) and args: '{}'", commandName, 
			std::to_string(it->second.size()), ::Utils::ToStringIterable(commandSegments)), LogType::Warning);
		return CommandInvocationResult::InvalidArgs;
	}

	void CommandController::LogOutput(const std::string& output, LogType logType)
	{
		if (!m_doLogOutputMessages)
			return;

		LogMessage(logType, CallerLogDetails::None, output, false, true, nullptr, false);
	}

	std::vector<std::string> CommandController::GetCommandDocumentation(const std::string& commandName)
	{
		std::vector<std::string> docs = {};
		std::string formattedCommand = FormatCommandName(commandName);
		auto it = TryGetIteratorForCommandName(formattedCommand);

		for (const auto& commandOverload : it->second)
		{
			docs.emplace_back(commandOverload->GetDocumentation());
		}

		return docs;
	}

	std::vector<std::string> CommandController::GetCommandDocumentationAll()
	{
		std::vector<std::string> docs = {};
		for (const auto& command : m_commands)
		{
			for (const auto& commandOverload : command.second)
			{
				docs.emplace_back(commandOverload->GetDocumentation());
			}
		}
		return docs;
	}
}