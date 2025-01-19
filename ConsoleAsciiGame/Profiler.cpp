#include "pch.hpp"
#include "Profiler.hpp"
#include "HelperFunctions.hpp"

static constexpr std::streamsize PROCESS_TIME_PRECISION = 6;

Profiler::Profiler() : m_processes{} {}

ProcessCollectionType::iterator Profiler::TryGetProcessIterator(const std::string& processName)
{
	return m_processes.find(processName);
}

bool Profiler::HasProcess(const std::string& name)
{
	return m_processes.find(name) != m_processes.end();
}
void Profiler::SetCompletedProcess(const std::string& processName, const float& usTime)
{
	if (HasProcess(processName)) m_processes.at(processName).m_MicrosecondsTime = usTime;
	else m_processes.emplace(processName, ProfilerProcess{processName, usTime });
}
void Profiler::LogPerformanceTimes() const
{
	std::string log =   "================================"
					  "\n=    PROFILER PROCESS TIMES    ="
					  "\n================================";

	for (const auto& process : m_processes)
	{
		log += std::format("\n [{}]: {}us ({}s)", 
			process.first, Utils::ToStringDouble(process.second.m_MicrosecondsTime, PROCESS_TIME_PRECISION), 
			Utils::ToStringDouble(process.second.m_MicrosecondsTime/static_cast<float>(1'000'000), PROCESS_TIME_PRECISION));
	}
	std::cout << std::format("\n{}{}{}", Utils::ANSI_COLOR_GREEN, log, Utils::ANSI_COLOR_CLEAR) << std::endl;
}
