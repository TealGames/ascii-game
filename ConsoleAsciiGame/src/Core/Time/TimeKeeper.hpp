#pragma once
#include <chrono>
#include <cstdint>

class TimeKeeper
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_currentTime;
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
	TimeKeeper();

	void UpdateTimeStart();
	void UpdateTimeEnd();

	double GetLastScaledDeltaTime() const;
	double GetLastIndependentDeltaTime() const;
	double GetTimeScale() const;
	std::uint16_t GetFPS() const;

	void SetTimeScale(const double& scale);
	void StopTimeScale();
	void ResetTimeScale();

	void SetFrameLimit(const std::uint64_t& frameLimit);
	std::uint64_t GetFrameCount() const;

	bool ReachedFrameLimit() const;
};

