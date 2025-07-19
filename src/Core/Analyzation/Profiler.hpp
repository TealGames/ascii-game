#pragma once
#include "Core/Time/TimerBase.hpp"
#include <string>
#include <cstdint>
#include <unordered_map>
#include <optional>

//#define ENABLE_PROFILER

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

struct RoundTime
{
	ProcessRoundIntegralType m_Round = 0;
	float m_MicrosecondsTim = 0;
};

struct ProfilerRoutineSummary
{
	std::vector<RoundTime> m_RoundTimes;
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

using GraphColorIntegralType = std::uint8_t;
enum class GraphColor : GraphColorIntegralType
{
	None = 0,
	Black= 1,
	Red= 2,
	Green= 3,
	Blue= 4,
	Yellow= 5,
	Magenta= 6,
	Cyan= 7,
	White= 8
};

class plstream;
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
	TimerBase m_profilerActionsTimer;
public:

private:
	ProcessCollectionType::iterator TryGetProcessIterator(const std::string& processName);
	void UpdateProfilerSummary(const ProfilerProcess& process);
	std::string FormatRound(const ProcessRoundIntegralType& roundNumber);
	plstream* CreatePLPlotGraph(const double& roundMax, const double& timeMax, const char* name);
	void AddPLPlotRoutine(plstream& stream, const GraphColor& color, 
		const std::string& routineName, const ProfilerRoutineSummary& summary);

	std::string EncodeGraphInDesmosURL(const GraphColor& color, const std::vector<ProfilerRoutineSummary>& summary);

public:
	Profiler();
	~Profiler();

	bool HasProcess(const std::string& name);

	void SetCompletedProcess(const std::string& processName, const float& usTime);
	void LogCurrentRoundTimes();
};

