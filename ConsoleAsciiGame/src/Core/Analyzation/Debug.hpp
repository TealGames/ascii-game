#pragma once
#include <string>
#include "Utils/Data/Event.hpp"
#include <optional>
#include <cstdint>
#include <source_location>
#include "Utils/HelperMacros.hpp"

using LogTypeIntegralType = std::uint8_t;
enum class LogType : LogTypeIntegralType
{
	None = 0,
	Error = 1 << 0,
	Warning = 1 << 1,
	Log = 1 << 2,
	All= 0xFF
};

FLAG_ENUM_OPERATORS(LogType)
//constexpr LogType operator&(const LogType& lhs, const LogType& rhs)
//{
//	return static_cast<LogType>(static_cast<LogTypeIntegralType>(lhs)
//		& static_cast<LogTypeIntegralType>(rhs));
//}
//constexpr LogType& operator&=(LogType& lhs, const LogType& rhs)
//{
//	lhs = lhs & rhs;
//	return lhs;
//}
//constexpr LogType operator|(const LogType& lhs, const LogType& rhs)
//{
//	return static_cast<LogType>(static_cast<LogTypeIntegralType>(lhs)
//		| static_cast<LogTypeIntegralType>(rhs));
//}
//constexpr LogType& operator|=(LogType& lhs, const LogType& rhs)
//{
//	lhs = lhs | rhs;
//	return lhs;
//}
//constexpr LogType operator~(const LogType& op)
//{
//	return static_cast<LogType>(~static_cast<LogTypeIntegralType>(op));
//}
//constexpr bool operator==(const LogType first, const LogType other)
//{
//	return static_cast<LogTypeIntegralType>(first) == static_cast<LogTypeIntegralType>(other);
//}
//constexpr bool operator!=(const LogType first, const LogType other)
//{
//	return !(first == other);
//}

std::optional<LogType> StringToLogType(const std::string& str);
std::string LogTypeToString(const LogType& logType);

enum class CallerLogDetails : std::uint8_t
{
	None = 0,
	FileName = 1,
	FunctionName = 1 << 1,
	LineNumber = 1 << 2,
	ColumnNumber = 1 << 3,
	All= 0xFF
};
FLAG_ENUM_OPERATORS(CallerLogDetails)

constexpr bool DEFAULT_LOG_TIME = true;
constexpr bool DEFAULT_MESSAGE_EVENT_FLAG = true;
constexpr bool DEFAULT_SHOW_STACK_TRACE = false;

enum class ErroneousBehavior: std::uint8_t
{
	None = 0,
	/// <summary>
	/// Will throw an exception with the same contents as the message
	/// </summary>
	Throw = 1,
	/// <summary>
	/// [ONLY IN DEBUG BUILDS] Will cause a debug break to be triggered 
	/// in the editor
	/// </summary>
	Break = 1 << 2,
	/// <summary>
	/// Special event flag will be set in log message end event
	/// which can be used to trigger any other behavior in the program
	/// </summary>
	EventFlag = 1 << 3,
	All = 0xFF
};
FLAG_ENUM_OPERATORS(ErroneousBehavior)

namespace DebugProperties
{
	constexpr ErroneousBehavior ASSERT_BEHAVIOR = ErroneousBehavior::Break;
	constexpr ErroneousBehavior ERROR_LOG_BEHAVIOR = ErroneousBehavior::Break;

	extern Event<void, LogType, std::string, bool> OnMessageLogged;

	void SetLogMessages(const bool doLog);
	void SetCallerLogDetails(CallerLogDetails logDetails);
		
	void SetLogMessageFilter(const std::string& message);
	void ClearLogMessageFilter();
	void SetLogTypeFilter(LogType logType);
	void AddLogTypeFilter(LogType logType);
	void RemoveLogTypeFilter(LogType logType);
	void SetAllLogTypeFilter();
	void SetNoneLogTypeFilter();
	LogType GetLogTypeFilter();
	void ResetLogFilters();
}

/// <summary>
/// Achieves the same as defualt log but also includes the class that called it
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="logType"></param>
/// <param name="str"></param>
void LogMessage(const LogType& logType, const CallerLogDetails callerDetails, const std::string& message, const bool showStackTrace, const bool logTime,
	const char* overrideANSIColor, const bool setEventFlag, const std::source_location& loc = std::source_location::current());

/// <summary>
/// Logs a message as a default LOG type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
void Log(const std::string& message, const bool logTime = DEFAULT_LOG_TIME, 
	const char* overrideAnsiColor = nullptr, const bool setEventFlag = DEFAULT_MESSAGE_EVENT_FLAG, const std::source_location& loc = std::source_location::current());

/// <summary>
/// Logs a message as a WARNING type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogWarning(const std::string& message, const bool logTime = DEFAULT_LOG_TIME,
	const bool setEventFlag = DEFAULT_MESSAGE_EVENT_FLAG, const std::source_location& loc = std::source_location::current());

/// <summary>
/// Logs a message as an ERROR type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogError(const std::string& message, const bool logTime = DEFAULT_LOG_TIME,
	const bool showStackTrace = DEFAULT_SHOW_STACK_TRACE, const std::source_location& loc = std::source_location::current());
void Break();

bool Assert(const bool condition, const std::string& errMessage, const bool showStackTrace = DEFAULT_SHOW_STACK_TRACE, 
	const std::source_location& loc = std::source_location::current());