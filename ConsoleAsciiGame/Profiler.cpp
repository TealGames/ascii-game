#include "pch.hpp"
#include "Profiler.hpp"
#include "HelperFunctions.hpp"
#include <filesystem>
#include <fstream>
#include "StringUtil.hpp"
#include <cmath>
#include "plplot/plstream.h"
#include "plplot/plplot.h"

static constexpr bool WRITE_TO_FILE = true;
static constexpr bool DISPLAY_ALL_ROUTINE_GRAPHS = false;
static constexpr std::uint16_t MAX_GRAPH_POINTS = 1000;

static const std::string DISPLAY_GRAPH_ROUTINE_NAME = "LightSourceSystem::SystemUpdate";
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

	return std::format("{}{}{}", nonTimeSegment,
		std::string(spaceSize, ' '), GetFormattedTime(m_MicrosecondsTime));
}

Profiler::Profiler() : m_roundNumber(0), m_routines{}, m_profilerSummary{}, m_previousLogs(), m_profilerActionsTimer(){}
Profiler::~Profiler()
{
#ifndef ENABLE_PROFILER
	return;
#endif

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

	std::string url = EncodeGraphInDesmosURL(GraphColor::Green,
				Utils::GetValuesFromMap<std::string, ProfilerRoutineSummary>(
				m_profilerSummary.m_RoutineSummaries.begin(), m_profilerSummary.m_RoutineSummaries.end()));
	data += std::format("\n\nMapped Data (DESMOS url): {}", url);

	outStream << data << std::endl;
	outStream.close();

	//TODO: graphs are OK at the moment, but hard to reacd for all graphs in one and also 
	//no zooming, panning, accessiblity features
	//CHECK out PLOTLY for C++ (will require you to integrate Python code)
	if (DISPLAY_ALL_ROUTINE_GRAPHS)
	{
		ProfilerProcess slowestProcess = m_profilerSummary.m_SlowestProcess.value_or(ProfilerProcess{});
		float slowestTime = slowestProcess.m_MicrosecondsTime;

		plstream* stream = CreatePLPlotGraph(m_roundNumber, slowestTime, "ALL ROUTINE TIMES");
		GraphColorIntegralType graphColorValue = 0;
		for (const auto& routine : m_profilerSummary.m_RoutineSummaries)
		{
			graphColorValue++;
			AddPLPlotRoutine(*stream, static_cast<GraphColor>(graphColorValue), routine.first, routine.second);
		}
		delete stream;
	}
	else if (!DISPLAY_GRAPH_ROUTINE_NAME.empty())
	{
		auto graphDisplayRoutineIt = m_profilerSummary.m_RoutineSummaries.find(DISPLAY_GRAPH_ROUTINE_NAME);
		if (!Utils::Assert(this, graphDisplayRoutineIt != m_profilerSummary.m_RoutineSummaries.end(), 
			std::format("Tried to display a graph in profiler for routine: {} "
			"but a routine of that name could not be found", DISPLAY_GRAPH_ROUTINE_NAME))) return;

		plstream* stream = CreatePLPlotGraph(m_roundNumber, graphDisplayRoutineIt->second.m_SlowestTime, 
			std::format("\"{}\" Routine Times", DISPLAY_GRAPH_ROUTINE_NAME).c_str());

		AddPLPlotRoutine(*stream, GraphColor::Red, graphDisplayRoutineIt->first, graphDisplayRoutineIt->second);
		delete stream;
	}
}

plstream* Profiler::CreatePLPlotGraph(const double& roundMax, const double& timeMax, const char* name)
{
	plstream* stream = new plstream();

	plsdev("wingcc");
	stream->init();
	stream->env(0.0, roundMax, 0, timeMax, 0, 0);
	stream->lab("FRAME #", "TIME (microseconds)", name);

	return stream;
}

void Profiler::AddPLPlotRoutine(plstream& stream, const GraphColor& color, const std::string& routineName, const ProfilerRoutineSummary& graphDisplayRoutine)
{
	PLFLT x[MAX_GRAPH_POINTS], y[MAX_GRAPH_POINTS];

	//TODO: instead of having to go throguh each time maybe instead the memeber itself should be the PLFLT buffer
	for (int i = 0; i < MAX_GRAPH_POINTS && i < graphDisplayRoutine.m_RoundTimes.size(); i++)
	{
		x[i] = graphDisplayRoutine.m_RoundTimes[i].m_Round;
		y[i] = graphDisplayRoutine.m_RoundTimes[i].m_MicrosecondsTim;
	}
	
	PLINT graphColorIndex = static_cast<GraphColorIntegralType>(color);
	stream.col0(graphColorIndex);
	stream.line(MAX_GRAPH_POINTS, x, y);
	PLFLT width = 10;
	PLFLT height = 10;
	const char* title = routineName.c_str();
	//TODO: add a legend, however the real args are really long and shitty api
	/*
	* (PLFLT * p_legend_width, PLFLT * p_legend_height,
		PLINT opt, PLINT position, PLFLT x, PLFLT y, PLFLT plot_width,
		PLINT bg_color, PLINT bb_color, PLINT bb_style,
		PLINT nrow, PLINT ncolumn,
		PLINT nlegend, const PLINT * opt_array,
		PLFLT text_offset, PLFLT text_scale, PLFLT text_spacing,
		PLFLT text_justification,
		const PLINT * text_colors, const char* const* text,
		const PLINT * box_colors, const PLINT * box_patterns,
		const PLFLT * box_scales, const PLFLT * box_line_widths,
		const PLINT * line_colors, const PLINT * line_styles,
		const PLFLT * line_widths,
		const PLINT * symbol_colors, const PLFLT * symbol_scales,
		const PLINT * symbol_numbers, const char* const* symbols

		stream.legend(
		&width, &height, 0, 0, 0, 0, width,
		0, 0, 0, 1, 1, 1, nullptr,
		0, 1, 0, 0,
		&graphColorIndex, &title,
		nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	*/
}

std::string Profiler::EncodeGraphInDesmosURL(const GraphColor& color, const std::vector<ProfilerRoutineSummary>& summaries)
{
	std::string exp = "";

	std::string encodedX = "";
	std::string encodedY = "";

	const std::string columnStart = "{\"type\":\"column\",\"values\":[";
	const std::string columnEnd = "],\"color\":\"#c74440\"}";
	for (int i=0; i<summaries.size(); i++)
	{
		if (i != 0) exp += ",";

		encodedX = columnStart;
		encodedY = columnStart;
		const ProfilerRoutineSummary& summary = summaries[i];
		for (int j=0; j< summary.m_RoundTimes.size(); j++)
		{
			if (j != 0)
			{
				encodedX += ",";
				encodedY += ",";
			}
			encodedX += std::to_string(summary.m_RoundTimes[j].m_Round);
			encodedY += std::to_string(summary.m_RoundTimes[j].m_MicrosecondsTim);
		}
		encodedX += columnEnd;
		encodedY += columnEnd;
		exp += "{\"type\":\"table\",\"id\":\"1\",\"columns\":[" + encodedX + "," + encodedY + "]}";
	}

	return "https://www.desmos.com/calculator?expressions=[" + exp + "]";
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

	//To not waste extra space we do not add unneeded times
	if (DISPLAY_ALL_ROUTINE_GRAPHS || process.m_ProcessName == DISPLAY_GRAPH_ROUTINE_NAME)
	{
		summaryIt->second.m_RoundTimes.emplace_back(m_roundNumber, process.m_MicrosecondsTime);
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
