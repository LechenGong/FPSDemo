#include "Engine//Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"

Vec2 AnalogJoysticker::GetPosition() const
{
	return m_correctedPosition;
}

float AnalogJoysticker::GetMagnitude() const
{
	return m_correctedPosition.GetLength();
}

float AnalogJoysticker::GetOrientationDegree() const
{
	return Atan2Degrees( m_correctedPosition.y, m_correctedPosition.x );
}

Vec2 AnalogJoysticker::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

float AnalogJoysticker::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

float AnalogJoysticker::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

void AnalogJoysticker::Reset()
{
	m_rawPosition = Vec2( 0.f, 0.f );
	m_correctedPosition = Vec2( 0.f, 0.f );
}

void AnalogJoysticker::SetDeadZoneThreshold( float normalizedInnerDeadZoneThreshold, float normalizrdOuterDeadZoneThreshold )
{
	m_innerDeadZoneFraction = normalizedInnerDeadZoneThreshold;
	m_outerDeadZoneFraction = normalizrdOuterDeadZoneThreshold;
}

void AnalogJoysticker::UpdatePosition( float rawNormalizedX, float rawNormalizedY )
{
	m_rawPosition = Vec2( rawNormalizedX, rawNormalizedY );

	float orientationDegree = m_rawPosition.GetOrientationDegrees();
	float rawLength = m_rawPosition.GetLength();
	float correctedLength = RangeMapClamped( rawLength, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f );
	m_correctedPosition.SetPolarDegrees( orientationDegree, correctedLength );
}
