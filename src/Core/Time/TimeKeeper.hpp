#pragma once
#include <chrono>
#include <cstdint>

enum class TimeUnit : std::uint8_t
{
	Hours = 0,
	Minutes = 1,
	Seconds = 2,
	Milliseconds = 3,
};

using DoubleNanoseconds = std::chrono::duration<double, std::nano>;
using DoubleMicroseconds = std::chrono::duration<double, std::micro>;
using DoubleMilliseconds = std::chrono::duration<double, std::milli>;
using DoubleSeconds = std::chrono::duration<double>;
using DoubleMinutes = std::chrono::duration<double, std::ratio<60>>;
using DoubleHours = std::chrono::duration<double, std::ratio<3600>>;

using HighPrecisionTimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
class TimeKeeper
{
private:
	HighPrecisionTimePoint m_startTime;
	HighPrecisionTimePoint m_lastTime;
	HighPrecisionTimePoint m_currentTime;
	/// <summary>
	/// Delta time that is scaled to the time scale.
	/// Most useful for in-game time dependent on pauses/slow downs
	/// </summary>
	double m_scaledDeltaTime;
	/// <summary>
	/// Delta time that is indepedent of scale and is most uself for 
	/// core systems taht should always be running
	/// </summary>
	double m_independentDeltaTime;

	/// <summary>
	/// The scale applied to the delta time
	/// =1 is default, <1 will slow down >1 speed up, =0 is paused with main systems not updated
	/// </summary>
	double m_timeScale;
	std::uint16_t m_currentFPS;

	/// <summary>
	/// The total number of frames that have occured in the application runtime
	/// </summary>
	std::uint64_t m_frameCount;
	std::uint64_t m_frameLimit;

public:
	static constexpr std::uint64_t NO_FRAME_LIMIT = 0;
	static constexpr double DEFAULT_TIME_SCALE = 1;

private:
public:
	TimeKeeper(const std::uint64_t frameLimit= NO_FRAME_LIMIT);

	void UpdateTimeStart();
	void UpdateTimeEnd();

	HighPrecisionTimePoint GetNowHighPrecision() const;
	double GetNow(const TimeUnit unit) const;
	double GetTimeSinceInit(const TimeUnit unit) const;

	double GetLastScaledDeltaTime() const;
	double GetLastIndependentDeltaTime() const;
	double GetTimeScale() const;
	std::uint16_t GetFPS() const;

	void SetTimeScale(const double& scale);
	void StopTimeScale();
	void ResetTimeScale();

	void SetFrameLimit(const std::uint64_t& frameLimit);
	std::uint64_t GetFramesSinceInit() const;

	bool ReachedFrameLimit() const;
};

template<typename T>
double ConvertTime(const HighPrecisionTimePoint& timePoint)
{
	return T(timePoint.time_since_epoch()).count();
}

template<typename T>
double GetTimeDifference(const HighPrecisionTimePoint& largetTimePoint,
	const HighPrecisionTimePoint& smallerTimePoint)
{
	return T(largetTimePoint - smallerTimePoint).count();
}

double ConvertTime(const HighPrecisionTimePoint& timePoint, const TimeUnit unit);
double GetTimeDifference(const HighPrecisionTimePoint& largetTimePoint, 
	const HighPrecisionTimePoint& smallerTimePoint, const TimeUnit unit);

double GetTimeDifferenceMilliseconds(const HighPrecisionTimePoint& largetTimePoint,
	const HighPrecisionTimePoint& smallerTimePoint);
double GetTimeDifferenceSeconds(const HighPrecisionTimePoint& largetTimePoint,
	const HighPrecisionTimePoint& smallerTimePoint);

