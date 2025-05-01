#include "pch.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"

namespace DebugProperties
{
	std::string MessageFilter = "";
	LogType LogTypeFilter = LOG_TYPE_ALL;
	bool LogMessages = true;

	/// <summary>
	/// Where args are return type, log type, LogToGameConsole, PauseOnMessage
	/// </summary>
	Event<void, LogType, std::string, bool, bool> OnMessageLogged;
}

std::optional<LogType> StringToLogType(const std::string& str)
{
	std::string strFormatted = Utils::StringUtil(str).ToLowerCase().ToString();
	if (strFormatted == "none") return LogType::None;
	else if (strFormatted == "error") return LogType::Error;
	else if (strFormatted == "warning") return LogType::Warning;
	else if (strFormatted == "log") return LogType::Log;
	else if (strFormatted == "all") return LOG_TYPE_ALL;
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

void Log(const std::string& str, const bool& logTime, const bool& logToGameConsole, const char* overrideAnsiColor)
{
	Log<int>(nullptr, str, logTime, logToGameConsole, overrideAnsiColor);
}
void LogWarning(const std::string& str, const bool& logTime, const bool& logToGameConsole)
{
	LogWarning<int>(nullptr, str, logTime, logToGameConsole);
}
void LogError(const std::string& str, const bool& logTime, const bool& logToGameConsole, 
	const bool& showStackTrace, const bool& pauseOnMesssageEnd)
{
	LogError<int>(nullptr, str, logTime, logToGameConsole, showStackTrace, pauseOnMesssageEnd);
}

bool Assert(const bool condition, const std::string& errMessage, const bool& showStackTrace)
{
	return Assert<int>(nullptr, condition, errMessage, showStackTrace);
}

namespace DebugProperties
{
	void SetLogMessages(const bool doLog)
	{
		LogMessages = doLog;
	}

	void SetLogMessageFilter(const std::string& message)
	{
		MessageFilter = message;
	}
	void ClearLogMessageFilter()
	{
		MessageFilter = "";
	}
	void SetLogTypeFilter(const LogType& logType)
	{
		LogTypeFilter = logType;
	}
	void AddLogTypeFilter(const LogType& logType)
	{
		LogTypeFilter |= logType;
	}
	void RemoveLogTypeFilter(const LogType& logType)
	{
		LogTypeFilter &= ~logType;
	}
	void SetAllLogTypeFilter()
	{
		LogTypeFilter = LOG_TYPE_ALL;
	}
	void SetNoneLogTypeFilter()
	{
		LogTypeFilter = LogType::None;
	}
	const LogType& GetLogTypeFilter()
	{
		return LogTypeFilter;
	}
	void ResetLogFilters()
	{
		ClearLogMessageFilter();
		SetAllLogTypeFilter();
	}
}
