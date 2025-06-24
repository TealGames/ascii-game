#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <typeinfo>

class ICommandPrompt
{
private:
protected:
	/// <summary>
	/// The main command name. 
	/// Example: "SetPos" in '/SetPos "player" 0 0 0'
	/// </summary>
	std::string m_command;
	std::unordered_map<std::string, const std::type_info*> m_args;
public:

private:
public:
	ICommandPrompt(const std::string& command);
	virtual ~ICommandPrompt() = default;

	virtual bool TryInvokeAction(const std::vector<std::string>& args) const= 0;

	const std::string& GetCommandName() const;
	std::string GetDocumentation() const;
};

