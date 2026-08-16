#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"

Timer::Timer( float period, Clock const* clock )
	: m_clock( (clock == nullptr) ? &Clock::s_systemClock : clock )
	, m_period( period )
{
}

void Timer::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

void Timer::Stop()
{
	//m_startTime = 0.f;
	m_startTime = 0.000001f;
}

float Timer::GetElapsedTime() const
{
	if (IsStopped())
	{
		return 0.f;
	}
	float elapsedTime = m_clock->GetTotalSeconds() - m_startTime;
	return elapsedTime;
}

float Timer::GetElapsedFraction() const
{
	return GetElapsedTime() / m_period;
}

bool Timer::IsStopped() const
{
	//return m_startTime == 0.f;
	return m_startTime == 0.000001f;
}

bool Timer::HasPeriodElapsed() const
{
	return GetElapsedTime() >= m_period;
}

bool Timer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed())
	{
		m_startTime += m_period;
		return true;
	}
	return false;
}
