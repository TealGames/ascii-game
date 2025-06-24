#include "pch.hpp"
#include "CommandPromptType.hpp"
#include "HelperFunctions.hpp"
#include "StringUtil.hpp"
#include "Debug.hpp"

const std::type_info& GetCommandPromptTypeFromEnum(const CommandPromptType& prompt)
{
	if (prompt == CommandPromptType::Integer) return typeid(int);
	else if (prompt == CommandPromptType::Boolean) return typeid(bool);
	else if (prompt == CommandPromptType::String) return typeid(std::string);

	LogError(std::format("Tried to get command prompt type from enum but no actions found!"));
	return typeid(std::string);
}

std::optional<int> TryConvertPromptArgToInt(const std::string& arg)
{
	return Utils::TryParse<int>(arg);
}

std::optional<float> TryConvertPromptArgToFloat(const std::string& arg)
{
	return Utils::TryParse<float>(arg);
}

std::optional<bool> TryConvertPromptArgToBool(const std::string& arg)
{
	if (arg == "true") return true;
	else if (arg == "false") return false;
	return std::nullopt;
}