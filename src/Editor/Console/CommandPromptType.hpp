#pragma once
#include <typeinfo>
#include <string>
#include <optional>
#include <type_traits>

enum class CommandPromptType
{
	Integer,
	Float,
	Boolean,
	String,
};

const std::type_info& GetCommandPromptTypeFromEnum(const CommandPromptType& prompt);

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