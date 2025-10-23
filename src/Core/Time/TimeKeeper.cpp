#include "pch.hpp"
#include "Core/Time/TimeKeeper.hpp"
#include "Utils/Debug.hpp"

TimeKeeper::TimeKeeper(const std::uint64_t frameLimit) :
	m_startTime(std::chrono::high_resolution_clock().now()),
	m_currentTime(std::chrono::high_resolution_clock().now()),
	m_lastTime(std::chrono::high_resolution_clock().now()),
	m_scaledDeltaTime(0), m_independentDeltaTime(0), m_timeScale(DEFAULT_TIME_SCALE), m_currentFPS(0),
	m_frameCount(0), m_frameLimit(frameLimit)
{}

void TimeKeeper::UpdateTimeStart()
{
	m_currentTime = GetNowHighPrecision();
	m_independentDeltaTime = GetTimeDifferenceSeconds(m_currentTime, m_lastTime);
	m_scaledDeltaTime = m_independentDeltaTime * m_timeScale;

	m_currentFPS = 1 / m_scaledDeltaTime;
}
void TimeKeeper::UpdateTimeEnd()
{
	m_lastTime = m_currentTime;
	m_frameCount++;
}

HighPrecisionTimePoint TimeKeeper::GetNowHighPrecision() const { return std::chrono::high_resolution_clock().now(); }
double TimeKeeper::GetNow(const TimeUnit unit) const { return ConvertTime(GetNowHighPrecision(), unit); }
double TimeKeeper::GetTimeSinceInit(const TimeUnit unit) const { return GetTimeDifference(GetNowHighPrecision(), m_startTime, unit); }

double TimeKeeper::GetLastScaledDeltaTime() const { return m_scaledDeltaTime; }
double TimeKeeper::GetLastIndependentDeltaTime() const { return m_independentDeltaTime; }
double TimeKeeper::GetTimeScale() const { return m_timeScale; }
std::uint16_t TimeKeeper::GetFPS() const { return m_currentFPS; }

void TimeKeeper::SetTimeScale(const double& scale)
{
	m_timeScale = scale;
	//if (m_timeScale == 0) Assert(false, std::format("Set time"));
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

double ConvertTime(const HighPrecisionTimePoint& timePoint, const TimeUnit unit)
{
	if (unit == TimeUnit::Milliseconds)
		return ConvertTime<DoubleMilliseconds>(timePoint);
	else if (unit == TimeUnit::Seconds)
		return ConvertTime<DoubleSeconds>(timePoint);
	else if (unit == TimeUnit::Minutes)
		return ConvertTime<DoubleMinutes>(timePoint);
	else if (unit == TimeUnit::Hours)
		return ConvertTime<DoubleHours>(timePoint);
	
	LogError(std::format("Attempted to convert time point to time unit but unit has no actions defined"));
	return 0;
}
double GetTimeDifference(const HighPrecisionTimePoint& largetTimePoint,
	const HighPrecisionTimePoint& smallerTimePoint, const TimeUnit unit)
{
	if (unit == TimeUnit::Milliseconds)
		return GetTimeDifferenceMilliseconds(largetTimePoint, smallerTimePoint);
	else if (unit == TimeUnit::Seconds)
		return GetTimeDifferenceSeconds(largetTimePoint, smallerTimePoint);
	else if (unit == TimeUnit::Minutes)
		return GetTimeDifference<DoubleMinutes>(largetTimePoint, smallerTimePoint);
	else if (unit == TimeUnit::Hours)
		return GetTimeDifference<DoubleHours>(largetTimePoint, smallerTimePoint);

	LogError(std::format("Attempted to get time difference but time unit has no actions defined"));
	return 0;
}
double GetTimeDifferenceMilliseconds(const HighPrecisionTimePoint& largetTimePoint,
	const HighPrecisionTimePoint& smallerTimePoint)
{
	return GetTimeDifference<DoubleMilliseconds>(largetTimePoint, smallerTimePoint);
}
double GetTimeDifferenceSeconds(const HighPrecisionTimePoint& largetTimePoint,
	const HighPrecisionTimePoint& smallerTimePoint)
{
	return GetTimeDifference<DoubleSeconds>(largetTimePoint, smallerTimePoint);
}

double GetMilliseconds(const HighPrecisionTimePoint& timePoint)
{
	return ConvertTime<DoubleMilliseconds>(timePoint);
}
double GetSeconds(const HighPrecisionTimePoint& timePoint)
{
	return ConvertTime<DoubleSeconds>(timePoint);
}