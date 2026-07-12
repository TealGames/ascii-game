#include "pch.hpp"
#include "Core/Debug/IActionCommand.hpp"

namespace Engine::Debug
{
	IActionCommand::IActionCommand(const std::string& command) : m_args{}, m_command(command) {}

	const std::string& IActionCommand::GetCommandName() const
	{
		return m_command;
	}

	std::string IActionCommand::GetDocumentation() const
	{
		std::string doc = GetCommandName() + "(";
		for (const auto& arg : m_args)
		{
			std::string typeName = arg.second->name();
			if (*arg.second == typeid(std::string)) typeName = "string";

			doc += std::format(" {}:{}", arg.first, typeName);
		}
		doc += " )";
		return doc;
	}
}

