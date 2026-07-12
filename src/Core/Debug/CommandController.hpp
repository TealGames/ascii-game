#pragma once
#include <unordered_map>
#include <string>
#include "Core/Debug/ActionCommand.hpp"
#include "Core/Primitives/Color.hpp"

namespace Engine::Debug
{
	enum class CommandInvocationResult : std::uint8_t
	{
		Success = 0,
		EmptyCommand = 1,
		NoCommandName = 2,
		InvalidName	= 3,
		InvalidArgs = 4,
	};
	using CommandCollection = std::unordered_map<std::string, std::vector<IActionCommand*>>;

	constexpr int MAX_OUTPUT_MESSAGES = 10;
	class CommandController
	{
	private:
		CommandCollection m_commands;
		bool m_doLogOutputMessages;
	public:

	private:
		std::string FormatCommandName(const std::string& name);
		CommandCollection::iterator TryGetIteratorForCommandName(const std::string& name);
		std::string GetDocumentationForCommand(CommandCollection::iterator& commandIt, const size_t& index) const;

		void LogOutput(const std::string& output, LogType logType);
		void DeleteCommands();
	public:
		CommandController(bool debugLogOutputMessages);
		~CommandController();

		void AddCommand(IActionCommand* command);
		bool HasCommand(const std::string& commandName);
		
		CommandInvocationResult TryInvokeCommand(const std::string& commandStr);

		std::vector<std::string> GetCommandDocumentation(const std::string& commandName);
		std::vector<std::string> GetCommandDocumentationAll();
	};
}