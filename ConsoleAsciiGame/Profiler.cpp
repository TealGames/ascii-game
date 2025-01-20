#include "pch.hpp"
#include "Profiler.hpp"
#include "HelperFunctions.hpp"
#include <filesystem>
#include <fstream>
#include "StringUtil.hpp"

static constexpr bool WRITE_TO_FILE = true;
static const std::filesystem::path OUT_FILE_PATH = "profileroutput";
static constexpr std::streamsize PROCESS_TIME_PRECISION = 6;

static constexpr std::uint8_t MAX_ROUND_DIGITS = 5;
static constexpr std::uint16_t MAX_CHARS_BEFORE_PROCEDURE_TIME = 50;

std::string ProfilerProcess::ToString(const bool& addProcessNumber, const bool& addExtraSpacesBeforeTime) const
{
	const std::string nonTimeSegment = std::format("\n[{}{}]:",
		m_ProcessName, addProcessNumber ? std::format(" @Round:{}", m_ProcessNumber) : "");
	
	int spaceSize = std::max(
		static_cast<int>(MAX_CHARS_BEFORE_PROCEDURE_TIME - nonTimeSegment.size()), 0);

	Utils::LogError(std::format("LINE SPACE IS: {}", std::to_string(spaceSize)));
	return std::format("{}{}{}", nonTimeSegment,
		std::string(spaceSize, ' '), GetFormattedTime(m_MicrosecondsTime));
}

Profiler::Profiler() : m_roundNumber(0), m_routines{}, m_profilerSummary{}, m_previousLogs(), m_profilerActionsTimer(){}
Profiler::~Profiler()
{
	if (!WRITE_TO_FILE) return;

	std::string data = m_previousLogs;
	data += std::format("==========================================================="
					  "\n		 PROFILER SUMMARY"
					  "\n        Total Rounds:{}"
					  "\n===========================================================", FormatRound(m_roundNumber));

	const char* separator = "===================================================================";
	const char* nullData = "[NO DATA FOUND]";

	std::vector<ProfilerProcess> m_routineAveragesRanked;
	for (const auto& routineSummary : m_profilerSummary.m_RoutineSummaries)
	{
		m_routineAveragesRanked.push_back(ProfilerProcess{routineSummary.first, 0, routineSummary.second.m_AverageTime});
		data += std::format("\n\nROUTINE SUMMARY: \"{}\"\n{}"
			"\nTotalProcesses: {}"
			"\nSlowestTime: {}\nSlowestFrame: {}"
			"\nFastestTime: {}\nFastestFrame: {}"
			"\nAverageTime: {}",

			routineSummary.first, separator, std::to_string(routineSummary.second.m_TotalProcesses),
			GetFormattedTime(routineSummary.second.m_SlowestTime), std::to_string(routineSummary.second.m_SlowestTimeRound),
			GetFormattedTime(routineSummary.second.m_FastestTime), std::to_string(routineSummary.second.m_FastestTimeRound),
			GetFormattedTime(routineSummary.second.m_AverageTime));
	}

	data += std::format("\n\nOVERALL STATISTICS\n{}", separator);
	data += "\n\nFastestProcess:";
	if (m_profilerSummary.m_FastestProcess.has_value())
	{
		data += m_profilerSummary.m_FastestProcess.value().ToString(true);
	}
	else data += nullData;

	data += "\n\nSlowestProcess:";
	if (m_profilerSummary.m_SlowestProcess.has_value())
	{
		data += m_profilerSummary.m_SlowestProcess.value().ToString(true);
	}
	else data += nullData;

	std::sort(m_routineAveragesRanked.begin(), m_routineAveragesRanked.end(), 
		[](const ProfilerProcess& first, const ProfilerProcess& second) -> bool {
			return first.m_MicrosecondsTime > second.m_MicrosecondsTime;
		});

	data += "\n\nRanked Routine Averages:";
	for (const auto& rankedAverage : m_routineAveragesRanked)
	{
		data += std::format("{}", rankedAverage.ToString(false));
	}

	data += std::format("\n\nTotalProfilerTime: {}", GetFormattedTime(m_profilerSummary.m_ProfilerCheckTime));

	std::string fileName = Utils::FormatTime(Utils::GetCurrentTime()) + ".txt";
	std::replace(fileName.begin(), fileName.end(), ' ', '_');
	std::replace(fileName.begin(), fileName.end(), ':', '_');
	std::filesystem::path fullOutPath = OUT_FILE_PATH / fileName;

	Utils::LogWarning("Writing to file at: {}!", fullOutPath.string());
	std::ofstream outStream(fullOutPath);
	if (!Utils::Assert(this, outStream.is_open(), std::format("Tried to open a file at path: {} "
		"for writing to output profiler data but something went wrong", fullOutPath.string()))) return;

	outStream << data << std::endl;
	outStream.close();
}

ProcessCollectionType::iterator Profiler::TryGetProcessIterator(const std::string& processName)
{
	return m_routines.find(processName);
}

bool Profiler::HasProcess(const std::string& name)
{
	return m_routines.find(name) != m_routines.end();
}
void Profiler::SetCompletedProcess(const std::string& processName, const float& usTime)
{
	if (HasProcess(processName)) m_routines.at(processName).m_MicrosecondsTime = usTime;
	else m_routines.emplace(processName, ProfilerProcess{ processName, m_roundNumber, usTime });
}

std::string Profiler::FormatRound(const ProcessRoundIntegralType& roundNumber)
{
	return Utils::ToStringLeadingZeros(roundNumber, MAX_ROUND_DIGITS);
}

std::string GetFormattedTime(const float& usTime)
{
	return std::format("{}us ({}s)", Utils::ToStringDouble(usTime, PROCESS_TIME_PRECISION),
		Utils::ToStringDouble(usTime / static_cast<float>(1'000'000), PROCESS_TIME_PRECISION));
}

void Profiler::UpdateProfilerSummary(const ProfilerProcess& process)
{
	auto summaryIt = m_profilerSummary.m_RoutineSummaries.find(process.m_ProcessName);
	if (summaryIt == m_profilerSummary.m_RoutineSummaries.end())
	{
		m_profilerSummary.m_RoutineSummaries.emplace(process.m_ProcessName, ProfilerRoutineSummary{});
		summaryIt = m_profilerSummary.m_RoutineSummaries.find(process.m_ProcessName);
	}

	ProfilerRoutineSummary& routine = summaryIt->second;
	if (process.m_MicrosecondsTime > routine.m_SlowestTime)
	{
		routine.m_SlowestTime = process.m_MicrosecondsTime;
		routine.m_SlowestTimeRound = m_roundNumber;
	}
	if (process.m_MicrosecondsTime < routine.m_FastestTime || 
		Utils::ApproximateEqualsF(routine.m_FastestTime, 0))
	{
		routine.m_FastestTime = process.m_MicrosecondsTime;
		routine.m_FastestTimeRound = m_roundNumber;
	}

	routine.m_AverageTime = (routine.m_AverageTime * routine.m_TotalProcesses + 
							process.m_MicrosecondsTime) / static_cast<float>(routine.m_TotalProcesses + 1);

	if (!m_profilerSummary.m_FastestProcess.has_value() || 
		Utils::ApproximateEqualsF(m_profilerSummary.m_FastestProcess.value().m_MicrosecondsTime, 0) || 
		process.m_MicrosecondsTime < m_profilerSummary.m_FastestProcess.value().m_MicrosecondsTime)
	{
		m_profilerSummary.m_FastestProcess = process;
	}
	if (!m_profilerSummary.m_SlowestProcess.has_value() ||
		process.m_MicrosecondsTime > m_profilerSummary.m_SlowestProcess.value().m_MicrosecondsTime)
	{
		m_profilerSummary.m_SlowestProcess = process;
	}

	routine.m_TotalProcesses++;
}

void Profiler::LogCurrentRoundTimes()
{
	m_profilerActionsTimer.Start();
	std::string log = std::format("============================================"
							    "\n      PROFILER PROCESS TIMES"
								"\n      Round:{}"
							    "\n============================================", FormatRound(m_roundNumber));

	for (const auto& process : m_routines)
	{
		if (WRITE_TO_FILE) UpdateProfilerSummary(process.second);
		log += process.second.ToString(false);
	}
	float profilerActionsTime = m_profilerActionsTimer.Stop().m_Duration;
	m_profilerSummary.m_ProfilerCheckTime += profilerActionsTime;
	log += std::format("\n\n[PROFILER_ACTIONS_TIME]: {}", GetFormattedTime(profilerActionsTime));

	if (WRITE_TO_FILE)
	{
		m_roundNumber++;
		m_previousLogs += log + "\n\n";
	}
	
	std::cout << std::format("\n{}{}{}", Utils::ANSI_COLOR_GREEN, log, Utils::ANSI_COLOR_CLEAR) << std::endl;
}
