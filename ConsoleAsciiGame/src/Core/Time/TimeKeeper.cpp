#include "pch.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "Core/Analyzation/Debug.hpp"

TimeKeeper::TimeKeeper() :
	m_currentTime(std::chrono::high_resolution_clock().now()),
	m_lastTime(std::chrono::high_resolution_clock().now()),
	m_scaledDeltaTime(0), m_independentDeltaTime(0), m_timeScale(DEFAULT_TIME_SCALE), m_currentFPS(0),
	m_frameCount(0), m_frameLimit(NO_FRAME_LIMIT)
{}

void TimeKeeper::UpdateTimeStart()
{
	m_currentTime = std::chrono::high_resolution_clock().now();
	m_independentDeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime - m_lastTime).count() / static_cast<float>(1000);
	m_scaledDeltaTime = m_independentDeltaTime * m_timeScale;

	m_currentFPS = 1 / m_scaledDeltaTime;
}
void TimeKeeper::UpdateTimeEnd()
{
	m_lastTime = m_currentTime;
	m_frameCount++;
}

double TimeKeeper::GetLastScaledDeltaTime() const
{
	return m_scaledDeltaTime;
}
double TimeKeeper::GetLastIndependentDeltaTime() const
{
	return m_independentDeltaTime;
}
double TimeKeeper::GetTimeScale() const
{
	return m_timeScale;
}
std::uint16_t TimeKeeper::GetFPS() const
{
	return m_currentFPS;
}

void TimeKeeper::SetTimeScale(const double& scale)
{
	m_timeScale = scale;
	if (m_timeScale == 0) Assert(false, std::format("Set time"));
}
void TimeKeeper::ResetTimeScale()
{
	SetTimeScale(1);
}
void TimeKeeper::StopTimeScale()
{
	SetTimeScale(0);
}

void TimeKeeper::SetFrameLimit(const std::uint64_t& frameLimit)
{
	m_frameLimit = frameLimit;
}
std::uint64_t TimeKeeper::GetFrameCount() const
{
	return m_frameCount;
}
bool TimeKeeper::ReachedFrameLimit() const
{
	return m_frameLimit!= NO_FRAME_LIMIT && 
		m_frameCount >= m_frameLimit;
}