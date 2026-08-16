#pragma once

#include <vector>

class Clock
{
public:
	Clock();
	~Clock();
	Clock( Clock const& copyFrom ) = delete;
	explicit Clock( Clock& parent );

	void Reset();

	bool IsPaused() const;
	void Pause();
	void UnPause();
	void TogglePause();

	void StepSingleFrame();

	void SetTimeScale( float );
	float GetTimeScale() const;

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	int GetFrameCount() const;

	static Clock s_systemClock;

public:
	static Clock& GetSystemClock();
	static void TickSystemClock();

protected:
	void Tick();

	void Advance( float deltaTimeSeconds );

	void AddChild( Clock* childClock );

	void RemoveChild( Clock* childClock );

protected:
	Clock* m_parent = nullptr;

	std::vector<Clock*> m_children;

	float m_lastUpdatedTimeInSeconds = 0.0f;
	float m_totalSeconds = 0.0f;
	float m_deltaSeconds = 0.0f;
	int m_frameCount = 0;

	float m_timeScale = 1.0f;

	bool m_isPaused = false;

	bool m_stepSingleFrame = false;

	float m_maxDeltaSeconds = 0.1f;
};
