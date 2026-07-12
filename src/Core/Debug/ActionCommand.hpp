#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <functional>
#include <typeinfo>
#include "Core/Debug/IActionCommand.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/Debug.hpp"

namespace Engine::Debug
{
	std::optional<int> TryConvertPromptArgToInt(const std::string& arg);
	std::optional<float> TryConvertPromptArgToFloat(const std::string& arg);
	std::optional<bool> TryConvertPromptArgToBool(const std::string& arg);

	template<typename T>
	concept IsValidPromptType = std::is_same_v<T, int> || std::is_same_v<T, float> ||
		std::is_same_v<T, bool> || std::is_same_v<T, std::string>;

	template<typename T>
		requires IsValidPromptType<T>
	std::optional<T> TryConvertPromptArgToType(const std::string& arg)
	{
		if constexpr (std::is_same_v<T, int>)
			return static_cast<std::optional<int>>(TryConvertPromptArgToInt(arg));

		else if constexpr (std::is_same_v<T, float>)
			return static_cast<std::optional<float>>(TryConvertPromptArgToFloat(arg));

		else if constexpr (std::is_same_v<T, bool>)
			return static_cast<std::optional<bool>>(TryConvertPromptArgToBool(arg));

		else return std::optional<std::string>(arg);
	}

	template<typename ...Args>
	class ActionCommand : public IActionCommand
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
		ActionCommand(const std::string& commandName, const std::vector<std::string>& argNames, const std::function<void(Args...)>& action) :
			IActionCommand(commandName), m_action(action)
		{
			ENGINE_ASSERT(GetArgCount() == argNames.size(), "Tried to create a command prompt "
				"but the arg count names: {} does not match total args: {}",
				std::to_string(argNames.size()), std::to_string(GetArgCount()));

			size_t argNameIndex = 0;
			(m_args.emplace(argNames[argNameIndex++], &typeid(Args)), ...);
		}

		bool TryInvokeAction(const std::vector<std::string>& args) const override
		{
			if (args.size() != m_args.size()) return false;

			std::vector<std::string> thing = {};
			(thing.emplace_back(typeid(Args).name()), ...);
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
}


