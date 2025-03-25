#include "pch.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"

std::string MessageFilter = "";
LogType LogTypeFilter = LOG_TYPE_ALL;

/// <summary>
/// Where args are return type, log type, is logged to game console
/// </summary>
Event<void, LogType, std::string, bool> OnMessageLogged;

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

	if (Utils::HasFlagAll(logType, LogType::Log)) elementBits.push_back("Log");
	if (Utils::HasFlagAll(logType, LogType::Warning)) elementBits.push_back("Warning");
	if (Utils::HasFlagAll(logType, LogType::Error)) elementBits.push_back("Error");

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
void LogError(const std::string& str, const bool& logTime, const bool& logToGameConsole, const bool& showStackTrace)
{
	LogError<int>(nullptr, str, logTime, logToGameConsole, showStackTrace);
}

bool Assert(const bool condition, const std::string& errMessage, const bool& showStackTrace)
{
	return Assert<int>(nullptr, condition, errMessage, showStackTrace);
}