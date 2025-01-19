#include "pch.hpp"
#include "ProfilerTimer.hpp"
#include "Profiler.hpp"
#include "Timer.hpp"
#include "HelperFunctions.hpp"
#include <string>

Profiler ProfilerTimer::m_Profiler = Profiler();

ProfilerTimer::ProfilerTimer(const std::string& processName) 
	: m_processName(processName) 
{
	Start();
	Utils::Log(Utils::LogType::Warning, std::format("Starting timer: {}", m_processName));
}

ProfilerTimer::~ProfilerTimer()
{
	TimerResult result = Stop();
	Utils::Log(Utils::LogType::Warning, std::format("ENDING timer: {}", result.ToString()));
	m_Profiler.SetCompletedProcess(m_processName, result.m_Duration);
}
