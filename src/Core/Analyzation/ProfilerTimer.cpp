#include "pch.hpp"
#include "Core/Analyzation/ProfilerTimer.hpp"
#include "Core/Analyzation/Profiler.hpp"
#include "Core/Time/TimerBase.hpp"
#include <string>

Profiler ProfilerTimer::m_Profiler = Profiler();

ProfilerTimer::ProfilerTimer(const std::string& processName) 
	: m_processName(processName) 
{
	Start();
}

ProfilerTimer::~ProfilerTimer()
{
	TimerResult result = Stop();
	m_Profiler.SetCompletedProcess(m_processName, result.m_Duration);
}
