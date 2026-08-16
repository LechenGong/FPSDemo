#pragma once

#include "Engine/Math/Vec2.hpp"

class AnalogJoysticker
{
public:
	Vec2 GetPosition() const;
	float GetMagnitude() const;
	float GetOrientationDegree() const;

	Vec2 GetRawUncorrectedPosition() const;
	float GetInnerDeadZoneFraction() const;
	float GetOuterDeadZoneFraction() const;

	void Reset();
	void SetDeadZoneThreshold( float normalizedInnerDeadZoneThreshold, float normalizrdOuterDeadZoneThreshold );
	void UpdatePosition( float rawNormalizedX, float rawNormalizedY );

protected:
	Vec2 m_rawPosition;
	Vec2 m_correctedPosition;
	float m_innerDeadZoneFraction = 0.30f;
	float m_outerDeadZoneFraction = 0.95f;
};