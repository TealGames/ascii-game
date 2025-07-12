#pragma once
#include <chrono>
#include <ctime>
#include <string>

using TimePointHighRes = std::chrono::time_point<std::chrono::high_resolution_clock>;
using TimePointLowRes = std::chrono::time_point<std::chrono::system_clock>;
using LocalTime = std::chrono::zoned_time<std::chrono::system_clock::duration>;
struct TimerResult
{
	TimePointLowRes m_StartLocalTime;
	TimePointLowRes m_EndLocalTime;
	float m_Duration;

	TimerResult();
	TimerResult(const TimePointLowRes& startTime, const TimePointLowRes& endTime, const float& duration);
	
	std::string ToString() const;
};

std::string ToString(const LocalTime& time);
std::string ToString(const TimePointLowRes& time);

class TimerBase
{
protected:
	TimePointHighRes m_startTimeHighRes;
	TimePointHighRes m_endTimeHighRes;
	
	TimePointLowRes m_systemStartTime;
	TimePointLowRes m_systemEndTime;
	bool m_isRunning;

public:

private:
public:
	TimerBase();
	~TimerBase();

	/// <summary>
	/// Will start the timer
	/// </summary>
	virtual void Start();

	/// <summary>
	/// Returns the total time elapsed from Start() call in SECONDS
	/// </summary>
	/// <returns></returns>
	virtual TimerResult Stop();
};

