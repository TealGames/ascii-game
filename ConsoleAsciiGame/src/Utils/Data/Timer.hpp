#pragma once

class Timer
{
private:
	float m_timeLeft;

public:
	static constexpr float NO_TIMER_FLAG = -1;

private:
public:
	Timer(const float time= NO_TIMER_FLAG);

	void Start(const float time);
	void Update(const float deltaTime);
	/// <summary>
	/// Will terminate the current timer (if any) ->
	/// it will be in an 'DONE' state and isDone is TRUE
	/// Note: this is only useful for getting isDone flag afterwards.
	/// If you do not need it, use Reset() instead to fast-track setup for next run
	/// </summary>
	void Stop();
	/// <summary>
	/// Will prepare for next iteration by resetting as if it 
	/// has no time -> isDone is now FALSE
	/// NOTE: if you want to stop instantly/prepare just call Reset() and not
	/// Stop(). Use Stop() only if you need the done flag
	/// </summary>
	void Reset();

	bool IsDone() const;
	/// <summary>
	/// Will check the is done flag and reset if done.
	/// This is useful if you need the flag, but do not want to
	/// call reset() manually everytime
	/// </summary>
	/// <returns></returns>
	bool ResetIfDone();

	bool IsRunning() const;
	float GetTimeLeft() const;
};

