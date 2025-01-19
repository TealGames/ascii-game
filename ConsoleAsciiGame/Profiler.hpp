#pragma once
#include "Timer.hpp"
#include <string>
#include <unordered_map>

#define ENABLE_PROFILER

struct ProfilerProcess
{
	std::string m_Name;
	float m_MicrosecondsTime;
};

using ProcessCollectionType = std::unordered_map<std::string, ProfilerProcess>;
class Profiler
{
private:
	ProcessCollectionType m_processes;
public:

private:
	ProcessCollectionType::iterator TryGetProcessIterator(const std::string& processName);
public:
	Profiler();

	bool HasProcess(const std::string& name);

	void SetCompletedProcess(const std::string& processName, const float& usTime);
	void LogPerformanceTimes() const;
};

