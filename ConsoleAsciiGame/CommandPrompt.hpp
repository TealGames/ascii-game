#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <typeinfo>
#include "CommandPromptType.hpp"
#include "ICommandPrompt.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

template<typename ...Args>
class CommandPrompt : public ICommandPrompt
{
private:
	/// <summary>
	/// The args neccessary for a proper command. 
	/// Example: "player", 0, 0, 0 in '/SetPos "player" 0 0 0'
	/// </summary>
	std::function<void(Args...)> m_action;

public:

private:
public:
	CommandPrompt(const std::string& commandName, const std::vector<std::string>& argNames, const std::function<void(Args...)>& action) : 
		ICommandPrompt(commandName), m_action(action)
	{
		if (!Assert(this, GetArgCount() == argNames.size(), std::format("Tried to create a command prompt "
			"but the arg count names: {} does not match total args: {}",
			std::to_string(argNames.size()), std::to_string(GetArgCount()))))
		{
			throw std::invalid_argument(std::format("ARG COUNT: {} does not match NAMES: {}", 
				std::to_string(GetArgCount()), std::to_string(argNames.size())));
		}

		size_t argNameIndex = 0;
		(m_args.emplace(argNames[argNameIndex++], &typeid(Args)), ...);
	}

	bool TryInvokeAction(const std::vector<std::string>& args) const override
	{
		if (args.size() != m_args.size()) return false;

		std::vector<std::string> thing = {};
		(thing.push_back(typeid(Args).name()), ...);
		/*Log(std::format("HERE YOU GO: args: {} doc: {} RECENT TRY: {}", 
			Utils::ToStringIterable<std::vector<std::string>, std::string>(args), GetDocumentation(),
			Utils::ToStringIterable<std::vector<std::string>, std::string>(thing)));*/

		//throw std::invalid_argument(0);

		size_t argIndex = 0;
		try
		{
			m_action(TryConvertPromptArgToType<Args>(args[argIndex++]).value()...);
		}
		catch (const std::bad_optional_access& e)
		{
			return false;
		}
		return true;
	}

	const std::string& GetCommandName() const
	{
		return m_command;
	}

	size_t GetArgCount() const
	{
		return sizeof...(Args);
	}

	/*bool DoArgsMatch(const std::vector<const std::type_info*>& targetArgs)
	{
		if (targetArgs.size() != m_args.size()) return false;

		size_t argIndex = 0;
		for (const auto& arg : m_args)
		{
			if (arg.second != targetArgs[argIndex]) return false;
			argIndex++;
		}
		return true;
	}*/
};

