#include "pch.hpp"
#include "Debug.hpp"
#include "StringUtil.hpp"

std::string MessageFilter = "";
LogType LogTypeFilter = LOG_TYPE_ALL;

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

void Log(const std::string& str, const bool& logTime)
{
	Log<int>(nullptr, str, logTime);
}
void LogWarning(const std::string& str, const bool& logTime)
{
	LogWarning<int>(nullptr, str, logTime);
}
void LogError(const std::string& str, const bool& logTime)
{
	LogError<int>(nullptr, str, logTime);
}

bool Assert(const bool condition, const std::string& errMessage)
{
	return Assert<int>(nullptr, condition, errMessage);
}