#pragma once
#include "Timer.hpp"
#include <string>
#include <unordered_map>
#include <cstdint>
#include <optional>
#include "Timer.hpp"

#define ENABLE_PROFILER

//PROCESS is a single execution of some code 
//ROUTINE is a PROCESS that repeats multiple times
//ROUND is a sequence of multiple PROCESSES

using ProcessRoundIntegralType = std::uint32_t;
struct ProfilerProcess
{
	std::string m_ProcessName = "";
	ProcessRoundIntegralType m_ProcessNumber = 0;
	float m_MicrosecondsTime = 0;

	std::string ToString(const bool& addProcessNumber, const bool& addExtraSpacesBeforeTime=true) const;
};

struct ProfilerRoutineSummary
{
	ProcessRoundIntegralType m_TotalProcesses = 0;

	float m_SlowestTime = 0;
	ProcessRoundIntegralType m_SlowestTimeRound = 0;

	float m_FastestTime = 0;
	ProcessRoundIntegralType m_FastestTimeRound = 0;

	float m_AverageTime = 0;
};

struct ProfilerSummary
{
	std::unordered_map<std::string, ProfilerRoutineSummary> m_RoutineSummaries = {};
	std::optional<ProfilerProcess> m_SlowestProcess = std::nullopt;
	std::optional<ProfilerProcess> m_FastestProcess = std::nullopt;
	float m_ProfilerCheckTime = 0;
};

std::string GetFormattedTime(const float& usTime);

using ProcessCollectionType = std::unordered_map<std::string, ProfilerProcess>; 
class Profiler
{
private:
	/// <summary>
	/// Stores a record of all frame times for each process
	/// </summary>
	//std::unordered_map<std::string, std::vector<float>> m_allRoundTimes;
	ProcessRoundIntegralType m_roundNumber;
	ProfilerSummary m_profilerSummary;
	std::string m_previousLogs;
	/// <summary>
	/// Stores the last process data
	/// </summary>
	ProcessCollectionType m_routines;
	Timer m_profilerActionsTimer;
public:

private:
	ProcessCollectionType::iterator TryGetProcessIterator(const std::string& processName);
	void UpdateProfilerSummary(const ProfilerProcess& process);
	std::string FormatRound(const ProcessRoundIntegralType& roundNumber);

public:
	Profiler();
	~Profiler();

	bool HasProcess(const std::string& name);

	void SetCompletedProcess(const std::string& processName, const float& usTime);
	void LogCurrentRoundTimes();
};

