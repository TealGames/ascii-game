#include "pch.hpp"
#include "Timer.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

TimerResult::TimerResult() : TimerResult({}, {}, 0) {}

TimerResult::TimerResult(const TimePointLowRes& startTime, const TimePointLowRes& endTime, const float& duration) :
	m_StartLocalTime(startTime), m_EndLocalTime(endTime), m_Duration(duration)
{
}

std::string TimerResult::ToString() const
{
	return std::format("[Start:{} -> End:{} Duration: {}]", 
		::ToString(m_StartLocalTime), 
		::ToString(m_EndLocalTime), std::to_string(m_Duration));
}

std::string ToString(const LocalTime& time)
{
	return Utils::FormatTime(time);
	//return std::string(std::put_time(std::localtime(&time)));

	/*std::tm* now = std::localtime_s(&time);
	std::cout << (now->tm_year + 1900) << '-'
		<< (now->tm_mon + 1) << '-'
		<< now->tm_mday
		<< "\n";*/
}

std::string ToString(const TimePointLowRes& time)
{
	return ::ToString(Utils::GetLocalTime(time));
}

Timer::Timer() : m_isRunning(false), m_systemStartTime(), 
m_systemEndTime(), m_startTimeHighRes(), m_endTimeHighRes() {}

Timer::~Timer()
{
	if (m_isRunning) Stop();
}

void Timer::Start()
{
	if (!Assert(this, !m_isRunning, 
		std::format("Tried to START a timer while it is already running!"))) 
		return;

	m_isRunning = true;
	m_systemStartTime = std::chrono::system_clock::now();
	m_startTimeHighRes = std::chrono::high_resolution_clock().now();
}

TimerResult Timer::Stop()
{
	if (!Assert(this, m_isRunning,
		std::format("Tried to STOP a timer while it is not running!")))
		return {};

	m_endTimeHighRes = std::chrono::high_resolution_clock().now();
	m_systemEndTime= std::chrono::system_clock::now();
	float duration = std::chrono::duration_cast<std::chrono::microseconds>(m_endTimeHighRes - m_startTimeHighRes).count();

	m_isRunning = false;
	return TimerResult{ m_systemStartTime, m_systemEndTime, duration };
}