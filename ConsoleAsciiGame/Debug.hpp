#pragma once
#include "HelperFunctions.hpp"
#include "Globals.hpp"
#include <optional>
#include <cstdint>

using LogTypeIntegralType = std::uint8_t;
enum class LogType : LogTypeIntegralType
{
	None = 0,
	Error = 1 << 0,
	Warning = 1 << 1,
	Log = 1 << 2,
};

constexpr LogType operator&(const LogType& lhs, const LogType& rhs)
{
	return static_cast<LogType>(static_cast<LogTypeIntegralType>(lhs)
		& static_cast<LogTypeIntegralType>(rhs));
}
constexpr LogType& operator&=(LogType& lhs, const LogType& rhs)
{
	lhs = lhs & rhs;
	return lhs;
}
constexpr LogType operator|(const LogType& lhs, const LogType& rhs)
{
	return static_cast<LogType>(static_cast<LogTypeIntegralType>(lhs)
		| static_cast<LogTypeIntegralType>(rhs));
}
constexpr LogType& operator|=(LogType& lhs, const LogType& rhs)
{
	lhs = lhs | rhs;
	return lhs;
}
constexpr LogType operator~(const LogType& op)
{
	return static_cast<LogType>(~static_cast<LogTypeIntegralType>(op));
}
std::optional<LogType> StringToLogType(const std::string& str);
std::string LogTypeToString(const LogType& logType);

constexpr LogType LOG_TYPE_ALL = LogType::Log | LogType::Error | LogType::Warning;
constexpr bool DEFAULT_LOG_TIME = true;
constexpr bool THROW_ON_FAILED_ASSERT = true;
extern std::string MessageFilter;
extern LogType LogTypeFilter;

/// <summary>
/// Achieves the same as defualt log but also includes the class that called it
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="logType"></param>
/// <param name="str"></param>
template<typename T>
void LogMessage(const T* const objPtr, const LogType& logType, const std::string& message, const bool& logTime = DEFAULT_LOG_TIME)
{
	//Log(logType, std::format("{}: {}", objPtr != nullptr ? typeid(T).name() : "", str), logTime);

	//if (!DoLogMessages) return;
	//if ((LOG_MESSAGE_TYPES & logType) != LogType::None) return;
	if (!Utils::HasFlagAny(LogTypeFilter, logType)) return;

	if (!MessageFilter.empty() &&
		message.substr(0, MessageFilter.size()) != MessageFilter) return;

	std::string logTypeMessage;
	std::string timeFormatted = "";
	if (logTime) timeFormatted = std::format("{}[{}{}{}]{}", ANSI_COLOR_WHITE, ANSI_COLOR_GRAY,
		Utils::FormatTime(Utils::GetCurrentTime()), ANSI_COLOR_WHITE, ANSI_COLOR_CLEAR);

	switch (logType)
	{
	case LogType::Error:
		logTypeMessage = std::format("{}[{}!{}] {} {}ERROR:", ANSI_COLOR_WHITE,
			ANSI_COLOR_RED, ANSI_COLOR_WHITE, timeFormatted, ANSI_COLOR_RED);
		break;
	case LogType::Warning:
		logTypeMessage = std::format("{}[{}!{}] {} {}WARNING:", ANSI_COLOR_WHITE,
			ANSI_COLOR_YELLOW, ANSI_COLOR_WHITE, timeFormatted, ANSI_COLOR_YELLOW);
		break;
	case LogType::Log:
		logTypeMessage = std::format("{} {}LOG:", timeFormatted, ANSI_COLOR_WHITE);
		break;
	default:
		std::string errMessage = "Tried to log message of message type "
			"that is not defined: ";
		throw std::invalid_argument(errMessage);
	}
	std::string objectName = objPtr != nullptr ? std::format("{}:", typeid(T).name())  : "";
	std::string fullMessage = std::format("\nCan Log:{}({}) current:{} {}{}{}{}", LogTypeToString(logType), 
		std::to_string(Utils::HasFlagAny(LogTypeFilter, logType)), LogTypeToString(LogTypeFilter),
		logTypeMessage, objectName, message, ANSI_COLOR_CLEAR);
	std::cout << fullMessage << std::endl;
}

/// <summary>
/// Logs a message as a default LOG type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
template<typename T>
void Log(const T* const objPtr, const std::string& str, const bool& logTime = DEFAULT_LOG_TIME)
{
	LogMessage<T>(objPtr, LogType::Log, str, logTime);
}
/// <summary>
/// Logs a message as a default LOG type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void Log(const std::string& str, const bool& logTime = DEFAULT_LOG_TIME);

/// <summary>
/// Logs a message as a WARNING type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
template<typename T>
void LogWarning(const T* const objPtr, const std::string& str, const bool& logTime = DEFAULT_LOG_TIME)
{
	LogMessage<T>(objPtr, LogType::Warning, str, logTime);
}
/// <summary>
/// Logs a message as a WARNING type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogWarning(const std::string& str, const bool& logTime = DEFAULT_LOG_TIME);

/// <summary>
/// Logs a message as an ERROR type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
template<typename T>
void LogError(const T* const objPtr, const std::string& str, const bool& logTime = DEFAULT_LOG_TIME)
{
	LogMessage<T>(objPtr, LogType::Error, str, logTime);
}
/// <summary>
/// Logs a message as an ERROR type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogError(const std::string& str, const bool& logTime = DEFAULT_LOG_TIME);

/// <summary>
/// If condition is false will log error and return false, othersise return true and does nothing
/// </summary>
bool Assert(const bool condition, const std::string& errMessage);

template<typename T>
bool Assert(const T* const objPtr, const bool condition, const std::string& errMessage)
{
	if (!condition)
	{
		LogError<T>(objPtr, errMessage);
		if (THROW_ON_FAILED_ASSERT) throw std::invalid_argument(errMessage);
	}
	return condition;
}

inline void SetLogMessageFilter(const std::string& message)
{
	MessageFilter = message;
}
inline void ClearLogMessageFilter()
{
	MessageFilter = "";
}
inline void SetLogTypeFilter(const LogType& logType)
{
	LogTypeFilter = logType;
}
inline void AddLogTypeFilter(const LogType& logType)
{
	LogTypeFilter |= logType;
}
inline void RemoveLogTypeFilter(const LogType& logType)
{
	LogTypeFilter &= ~logType;
}
inline void SetAllLogTypeFilter()
{
	LogTypeFilter = LOG_TYPE_ALL;
}
inline void SetNoneLogTypeFilter()
{
	LogTypeFilter = LogType::None;
}
inline const LogType& GetLogTypeFilter()
{
	return LogTypeFilter;
}
inline void ResetLogFilters()
{
	ClearLogMessageFilter();
	SetAllLogTypeFilter();
}