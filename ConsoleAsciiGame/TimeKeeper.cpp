#include "pch.hpp"
#include "TimeKeeper.hpp"

TimeKeeper::TimeKeeper() :
	m_currentTime(std::chrono::high_resolution_clock().now()),
	m_lastTime(std::chrono::high_resolution_clock().now()),
	m_deltaTime(0), m_timeScale(1), m_currentFPS(0), 
	m_frameCount(0), m_frameLimit(NO_FRAME_LIMIT)
{}

void TimeKeeper::UpdateTimeStart()
{
	m_currentTime = std::chrono::high_resolution_clock().now();
	m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime - m_lastTime).count() / static_cast<float>(1000) * m_timeScale;
	m_currentFPS = 1 / m_deltaTime;
}
void TimeKeeper::UpdateTimeEnd()
{
	m_lastTime = m_currentTime;
	m_frameCount++;
}

double TimeKeeper::GetLastDeltaTime() const
{
	return m_deltaTime;
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
}
void TimeKeeper::ResetTimeScale()
{
	m_timeScale = 1;
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