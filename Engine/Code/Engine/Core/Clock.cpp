#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"

Clock Clock::s_systemClock;

Clock::Clock()
{
	if (this != &s_systemClock)
	{
		m_parent = &s_systemClock;
		s_systemClock.AddChild( this );
	}
}

Clock::~Clock()
{
	if (m_parent != nullptr)
	{
		m_parent->RemoveChild( this );
	}
	for (Clock*& child : m_children)
	{
		if (child == nullptr)
			continue;

		child->m_parent = nullptr;
	}
	m_parent = nullptr;
}

Clock::Clock( Clock& parent )
	: m_parent( &parent )
{
	parent.AddChild( this );
}

void Clock::Reset()
{
	m_lastUpdatedTimeInSeconds = static_cast<float>(GetCurrentTimeSeconds());
	m_totalSeconds = 0.0f;
	m_deltaSeconds = 0.0f;
	m_frameCount = 0;
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::UnPause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_isPaused = false;
	m_stepSingleFrame = true;
	Tick();
	m_isPaused = true;
	m_stepSingleFrame = false;
}

void Clock::SetTimeScale( float timeScale )
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return (IsPaused()) ? 0.0f : m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

int Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock& Clock::GetSystemClock()
{
	return s_systemClock;
}

void Clock::TickSystemClock()
{
	s_systemClock.Tick();
}

void Clock::Tick()
{
	float timeNow = static_cast<float>(GetCurrentTimeSeconds());
	float deltaSeconds = timeNow - m_lastUpdatedTimeInSeconds;
	if (m_stepSingleFrame)
	{
		Advance( 0.016f * m_timeScale );
		m_lastUpdatedTimeInSeconds = timeNow;
	}
	else if (m_isPaused)
	{
		Advance( 0.f );
	}
	else
	{
		Advance( Clamp( deltaSeconds, 0.f, m_maxDeltaSeconds ) * m_timeScale );
		m_lastUpdatedTimeInSeconds = timeNow;
	}
}

void Clock::Advance( float deltaTimeSeconds )
{
	m_deltaSeconds = deltaTimeSeconds;
	m_totalSeconds += m_deltaSeconds;
	m_frameCount++;
	for (Clock* child : m_children)
	{
		if (child == nullptr)
			continue;

		child->Advance( m_deltaSeconds * child->GetTimeScale() );
	}
}

void Clock::AddChild( Clock* childClock )
{
 	childClock->m_parent = this;
	for (Clock*& child : m_children)
	{
		if (child == nullptr)
		{
			child = childClock;
			return;
		}
	}
	m_children.push_back( childClock );
}

void Clock::RemoveChild( Clock* childClock )
{
	childClock->m_parent = nullptr;
	for (Clock*& child : m_children)
	{
		if (child == childClock)
		{
			child = nullptr;
		}
	}
}
