#pragma once
#include "Profiler.hpp"
#include "Timer.hpp"
#include <string>

class ProfilerTimer : protected Timer
{
private:
	std::string m_processName;
	
public:
	static Profiler m_Profiler;

private:
public:
	ProfilerTimer(const std::string& processName);
	~ProfilerTimer();
};

