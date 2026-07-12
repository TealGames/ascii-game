#include "Core/Debug/ActionCommand.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Engine::Debug
{
	std::optional<int> TryConvertPromptArgToInt(const std::string& arg)
	{
		return ::Utils::TryParse<int>(arg);
	}

	std::optional<float> TryConvertPromptArgToFloat(const std::string& arg)
	{
		return ::Utils::TryParse<float>(arg);
	}

	std::optional<bool> TryConvertPromptArgToBool(const std::string& arg)
	{
		if (arg == "true") return true;
		else if (arg == "false") return false;
		return std::nullopt;
	}
}
