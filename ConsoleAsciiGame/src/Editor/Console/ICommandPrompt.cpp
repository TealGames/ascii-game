#include "pch.hpp"
#include "Editor/Console/ICommandPrompt.hpp"

ICommandPrompt::ICommandPrompt(const std::string& command) : m_args{}, m_command(command){}

const std::string& ICommandPrompt::GetCommandName() const
{
	return m_command;
}

std::string ICommandPrompt::GetDocumentation() const
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
