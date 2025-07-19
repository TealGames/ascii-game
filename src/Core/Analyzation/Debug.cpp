#include "pch.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/StringUtil.hpp"
#include "Utils/HelperFunctions.hpp"
#include "AnsiCodes.hpp"
#include <filesystem>

namespace DebugProperties
{
	std::string MessageFilter = "";
	LogType LogTypeFilter = LogType::All;
	CallerLogDetails CallerDetails = CallerLogDetails::All;
	bool LogMessages = true;

	/// <summary>
	/// Where args are return type, log type, MessageEventFlag (boolean)
	/// </summary>
	Event<void, LogType, std::string, bool> OnMessageLogged;

	void SetLogMessages(const bool doLog)
	{
		LogMessages = doLog;
	}
	void SetCallerLogDetails(CallerLogDetails logDetails)
	{
		CallerDetails = logDetails;
	}

	void SetLogMessageFilter(const std::string& message)
	{
		MessageFilter = message;
	}
	void ClearLogMessageFilter()
	{
		MessageFilter = "";
	}
	void SetLogTypeFilter(LogType logType)
	{
		LogTypeFilter = logType;
	}
	void AddLogTypeFilter(LogType logType)
	{
		LogTypeFilter |= logType;
	}
	void RemoveLogTypeFilter(LogType logType)
	{
		LogTypeFilter &= ~logType;
	}
	void SetAllLogTypeFilter()
	{
		LogTypeFilter = LogType::All;
	}
	void SetNoneLogTypeFilter()
	{
		LogTypeFilter = LogType::None;
	}
	LogType GetLogTypeFilter()
	{
		return LogTypeFilter;
	}
	void ResetLogFilters()
	{
		ClearLogMessageFilter();
		SetAllLogTypeFilter();
	}
}

std::optional<LogType> StringToLogType(const std::string& str)
{
	std::string strFormatted = Utils::StringUtil(str).ToLowerCase().ToString();
	if (strFormatted == "none") return LogType::None;
	else if (strFormatted == "error") return LogType::Error;
	else if (strFormatted == "warning") return LogType::Warning;
	else if (strFormatted == "log") return LogType::Log;
	else if (strFormatted == "all") return LogType::All;
	return std::nullopt;
}
std::string LogTypeToString(const LogType& logType)
{
	std::vector<std::string> elementBits = {};
	if (logType == LogType::None) return "[]";

	if (Utils::HasFlagAll(logType, LogType::Log)) elementBits.emplace_back("Log");
	if (Utils::HasFlagAll(logType, LogType::Warning)) elementBits.emplace_back("Warning");
	if (Utils::HasFlagAll(logType, LogType::Error)) elementBits.emplace_back("Error");

	return Utils::ToStringIterable<std::vector<std::string>, std::string>(elementBits);
}

void LogMessage(const LogType& logType, const CallerLogDetails logDetails, const std::string& message, const bool showStackTrace, const bool logTime,
	const char* overrideANSIColor, const bool setEventFlag, const std::source_location& loc)
{
	//Log(logType, std::format("{}: {}", objPtr != nullptr ? typeid(T).name() : "", str), logTime);

	if (!DebugProperties::LogMessages)
		return;

	//if ((LOG_MESSAGE_TYPES & logType) != LogType::None) return;
	if (!Utils::HasFlagAny(DebugProperties::LogTypeFilter, logType))
		return;

	if (!DebugProperties::MessageFilter.empty() &&
		message.substr(0, DebugProperties::MessageFilter.size()) != DebugProperties::MessageFilter)
		return;

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
	std::string callerDetails = logDetails != CallerLogDetails::None ? "[" : "";
	if ((logDetails & CallerLogDetails::FileName) != 0)
	{
		auto filePath = std::filesystem::path(loc.file_name());
		callerDetails += std::filesystem::relative(filePath, std::filesystem::current_path()).string();
	}
	if ((logDetails & CallerLogDetails::FunctionName) != 0)
	{
		if (callerDetails.size() > 1) callerDetails += " ";
		callerDetails += std::format("@{}", loc.function_name());
	}
	if ((logDetails & CallerLogDetails::LineNumber) != 0)
	{
		if (callerDetails.size() > 1) callerDetails += " ";
		callerDetails += "L" + std::to_string(loc.line());
	}
	if ((logDetails & CallerLogDetails::ColumnNumber) != 0)
	{
		if (callerDetails.size() > 1) callerDetails += " ";
		callerDetails += "C" + std::to_string(loc.column());
	}
	if (!callerDetails.empty()) callerDetails += "]";

	std::string fullMessage = std::format("\n{}{}{}{}", logTypeMessage, callerDetails, stackTraceMessage, ANSI_COLOR_CLEAR);
	std::cout << fullMessage << std::endl;
	DebugProperties::OnMessageLogged.Invoke(logType, message, setEventFlag);
}

void Log(const std::string& message, const bool logTime, const char* overrideAnsiColor, 
	const bool setEventFlag, const std::source_location& loc)
{
	LogMessage(LogType::Log, DebugProperties::CallerDetails, message, false, logTime, overrideAnsiColor, setEventFlag, loc);
}

/// <summary>
/// Logs a message as a WARNING type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="objPtr"></param>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogWarning(const std::string& message, const bool logTime, 
	const bool setEventFlag, const std::source_location& loc)
{
	LogMessage(LogType::Warning, DebugProperties::CallerDetails, message, false, logTime, nullptr, setEventFlag, loc);
}

/// <summary>
/// Logs a message as an ERROR type
/// </summary>
/// <param name="str"></param>
/// <param name="logTime"></param>
void LogError(const std::string& message, const bool logTime, const bool showStackTrace, const std::source_location& loc)
{
	LogMessage(LogType::Error, DebugProperties::CallerDetails, message, showStackTrace, logTime, nullptr, (DebugProperties::ERROR_LOG_BEHAVIOR & ErroneousBehavior::EventFlag)!=0, loc);
	if ((DebugProperties::ERROR_LOG_BEHAVIOR & ErroneousBehavior::Throw)!=0) throw std::invalid_argument(message);
	else if ((DebugProperties::ERROR_LOG_BEHAVIOR & ErroneousBehavior::Break)!=0) Break();
}
 
void Break()
{
#if defined(_MSC_VER)
	__debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
	raise(SIGTRAP);
#else
	std::abort();
#endif
}

bool Assert(const bool condition, const std::string& errMessage, const bool showStackTrace, const std::source_location& loc)
{
	if (!condition)
	{
		LogMessage(LogType::Error, DebugProperties::CallerDetails, errMessage, showStackTrace, true, nullptr, (DebugProperties::ASSERT_BEHAVIOR & ErroneousBehavior::EventFlag) != 0, loc);
		if ((DebugProperties::ASSERT_BEHAVIOR & ErroneousBehavior::Break)!=0) Break();
		if ((DebugProperties::ASSERT_BEHAVIOR & ErroneousBehavior::Throw)!=0) throw std::invalid_argument(errMessage);
	}
	return condition;
}
