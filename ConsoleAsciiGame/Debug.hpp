#pragma once
#include "HelperFunctions.hpp"
#include "Globals.hpp"
#include "Event.hpp"
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
constexpr bool operator==(const LogType first, const LogType other)
{
	return static_cast<LogTypeIntegralType>(first) == static_cast<LogTypeIntegralType>(other);
}
constexpr bool operator!=(const LogType first, const LogType other)
{
	return !(first == other);
}

std::optional<LogType> StringToLogType(const std::string& str);
std::string LogTypeToString(const LogType& logType);

constexpr LogType LOG_TYPE_ALL = LogType::Log | LogType::Error | LogType::Warning;
constexpr bool DEFAULT_LOG_TIME = true;
constexpr bool DEFAULT_LOG_TO_GAME_CONSOLE = false;
constexpr bool DEFAULT_SHOW_STACK_TRACE = false;
constexpr bool DEFAULT_PAUSE_ON_MESSAGE = false;

namespace DebugProperties
{
	constexpr bool THROW_ON_FAILED_ASSERT = true;
	constexpr bool THROW_ON_ALL_ERROR = false;
	constexpr bool PAUSE_ON_ERROR = true;

	extern bool LogMessages;
	extern std::string MessageFilter;
	extern LogType LogTypeFilter;

	extern Event<void, LogType, std::string, bool, bool> OnMessageLogged;

	void SetLogMessages(const bool doLog);
		
	void SetLogMessageFilter(const std::string& message);
	void ClearLogMessageFilter();
	void SetLogTypeFilter(const LogType& logType);
	void AddLogTypeFilter(const LogType& logType);
	void RemoveLogTypeFilter(const LogType& logType);
	void SetAllLogTypeFilter();
	void SetNoneLogTypeFilter();
	const LogType& GetLogTypeFilter();
	void ResetLogFilters();
}

/// <summary>
/// Achieves the same as defualt log but also includes the class that called it
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="logType"></param>
/// <param name="str"></param>
template<typename T>
void LogMessage(const T* const objPtr, const LogType& logType, const std::string& message, 
	const bool& showStackTrace, const bool& logTime = DEFAULT_LOG_TIME, const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE, 
	const char* overrideANSIColor= nullptr, const bool& pauseOnMessageDone= DEFAULT_PAUSE_ON_MESSAGE)
{
	//Log(logType, std::format("{}: {}", objPtr != nullptr ? typeid(T).name() : "", str), logTime);

	if (!DebugProperties::LogMessages) return;

	//if ((LOG_MESSAGE_TYPES & logType) != LogType::None) return;
	if (!Utils::HasFlagAny(DebugProperties::LogTypeFilter, logType)) return;

	if (!DebugProperties::MessageFilter.empty() &&
		message.substr(0, DebugProperties::MessageFilter.size()) != DebugProperties::MessageFilter) return;

	std::string stackTraceMessage = message;
	if (showStackTrace) stackTraceMessage += std::format("\n-------> STACK TRACE: {}", Utils::GetCurrentStackTrace());

	std::string logTypeMessage;
	std::string timeFormatted = "";
	if (logTime) timeFormatted = std::format("{}[{}{}{}]{}", ANSI_COLOR_WHITE, ANSI_COLOR_GRAY,
		Utils::FormatTime(Utils::GetCurrentTime()), ANSI_COLOR_WHITE, ANSI_COLOR_CLEAR);

	const char* mainTextAnsiColor = nullptr;
	switch (logType)
	{
	case LogType::Error:
		mainTextAnsiColor = overrideANSIColor != nullptr ? overrideANSIColor : ANSI_COLOR_RED;
		logTypeMessage = std::format("{}[{}!{}] {} {}ERROR:", ANSI_COLOR_WHITE,
			mainTextAnsiColor, ANSI_COLOR_WHITE, timeFormatted, mainTextAnsiColor);
		break;

	case LogType::Warning:
		mainTextAnsiColor = overrideANSIColor != nullptr ? overrideANSIColor : ANSI_COLOR_YELLOW;
		logTypeMessage = std::format("{}[{}!{}] {} {}WARNING:", ANSI_COLOR_WHITE,
			mainTextAnsiColor, ANSI_COLOR_WHITE, timeFormatted, mainTextAnsiColor);
		break;

	case LogType::Log:
		mainTextAnsiColor = overrideANSIColor != nullptr ? overrideANSIColor : ANSI_COLOR_WHITE;
		logTypeMessage = std::format("{} {}LOG:", timeFormatted, mainTextAnsiColor);
		break;

	default:
		std::string errMessage = "Tried to log message of message type "
			"that is not defined: ";
		throw std::invalid_argument(errMessage);
	}
	std::string objectName = objPtr != nullptr ? std::format("{}:", typeid(T).name())  : "";
	std::string fullMessage = std::format("\n{}{}{}{}", logTypeMessage, objectName, stackTraceMessage, ANSI_COLOR_CLEAR);
	
	std::cout << fullMessage << std::endl;

	if (DebugProperties::THROW_ON_ALL_ERROR && (logType & LogType::Error) != LogType::None) throw std::invalid_argument(message);
	DebugProperties::OnMessageLogged.Invoke(logType, message, logToGameConsole, pauseOnMessageDone);
}

/// <summary>
/// Logs a message as a default LOG type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
template<typename T>
void Log(const T* const objPtr, const std::string& str, 
	const bool& logTime = DEFAULT_LOG_TIME, const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE, 
	const char* overrideAnsiColor=nullptr)
{
	LogMessage<T>(objPtr, LogType::Log, str, false, logTime, logToGameConsole, overrideAnsiColor);
}
/// <summary>
/// Logs a message as a default LOG type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void Log(const std::string& str, const bool& logTime = DEFAULT_LOG_TIME, 
	const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE, 
	const char* overrideAnsiColor = nullptr);

/// <summary>
/// Logs a message as a WARNING type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
template<typename T>
void LogWarning(const T* const objPtr, const std::string& str, const bool& logTime = DEFAULT_LOG_TIME, 
	const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE)
{
	LogMessage<T>(objPtr, LogType::Warning, str, false, logTime, logToGameConsole);
}
/// <summary>
/// Logs a message as a WARNING type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogWarning(const std::string& str, const bool& logTime = DEFAULT_LOG_TIME, 
	const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE);

/// <summary>
/// Logs a message as an ERROR type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
template<typename T>
void LogError(const T* const objPtr, const std::string& str, const bool& logTime = DEFAULT_LOG_TIME, 
	const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE, const bool& showStackTrace= DEFAULT_SHOW_STACK_TRACE, 
	const bool& pauseOnMesssageEnd= DEFAULT_PAUSE_ON_MESSAGE)
{
	LogMessage<T>(objPtr, LogType::Error, str, showStackTrace, logTime, logToGameConsole, nullptr, pauseOnMesssageEnd);
}
/// <summary>
/// Logs a message as an ERROR type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogError(const std::string& str, const bool& logTime = DEFAULT_LOG_TIME, 
	const bool& logToGameConsole = DEFAULT_LOG_TO_GAME_CONSOLE, const bool& showStackTrace= DEFAULT_SHOW_STACK_TRACE, 
	const bool& pauseOnMesssageEnd = DEFAULT_PAUSE_ON_MESSAGE);

/// <summary>
/// If condition is false will log error and return false, othersise return true and does nothing
/// </summary>
bool Assert(const bool condition, const std::string& errMessage, const bool& showStackTrace= DEFAULT_SHOW_STACK_TRACE);

template<typename T>
bool Assert(const T* const objPtr, const bool condition, const std::string& errMessage, 
	const bool& showStackTrace = DEFAULT_SHOW_STACK_TRACE)
{
	if (!condition)
	{
		LogError<T>(objPtr, errMessage, DEFAULT_LOG_TIME, true, showStackTrace);
		if (DebugProperties::THROW_ON_FAILED_ASSERT) throw std::invalid_argument(errMessage);
	}
	return condition;
}

template<typename T>
void Break(const T* const objPtr)
{
	Assert(objPtr, false, std::format("Break invoked"));
}