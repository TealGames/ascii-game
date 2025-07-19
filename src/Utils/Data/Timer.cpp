#include "pch.hpp"
#include "Timer.hpp"
#include "Utils/HelperFunctions.hpp"

Timer::Timer(const float time) : m_timeLeft() 
{
	Start(time);
}

void Timer::Start(const float time)
{
	if (time <= 0) return;
	m_timeLeft = time;
}
void Timer::Update(const float deltaTime)
{
	if (!IsRunning()) return;

	m_timeLeft -= deltaTime;
	if (m_timeLeft <= 0) Stop();
}
void Timer::Stop()
{
	m_timeLeft = 0;
}
void Timer::Reset()
{
	m_timeLeft = NO_TIMER_FLAG;
}

bool Timer::IsDone() const
{
	return Utils::ApproximateEqualsF(m_timeLeft, 0);
}
bool Timer::ResetIfDone()
{
	bool isDone = IsDone();
	if (isDone) Reset();
	return isDone;
}

bool Timer::IsRunning() const
{
	return m_timeLeft > 0;
}
float Timer::GetTimeLeft() const
{
	if (m_timeLeft <= 0) return 0;
	return m_timeLeft;
}
