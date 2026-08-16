#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

const AABB2 AABB2::ZERO_TO_ONE = AABB2( 0.f, 0.f, 1.f, 1.f );
const AABB2 AABB2::ZERO = AABB2( 0.f, 0.f, 0.f, 0.f );

AABB2::AABB2( AABB2 const& copyForm )
{
	m_mins = copyForm.m_mins;
	m_maxs = copyForm.m_maxs;
}

AABB2::AABB2( float minX, float minY, float maxX, float maxY )
{
	m_mins = Vec2( minX, minY );
	m_maxs = Vec2( maxX, maxY );
}

AABB2::AABB2( Vec2 const& mins, Vec2 const& maxs )
{
	m_mins = mins;
	m_maxs = maxs;
}

bool AABB2::IsPointInside( Vec2 const& point ) const
{
	bool xInside =
		point.x <= m_maxs.x && point.x >= m_mins.x;
	bool yInside = 
		point.y <= m_maxs.y && point.y >= m_mins.y;
	return xInside && yInside;
}

Vec2 const AABB2::GetCenter() const
{
	return (m_maxs - m_mins) * 0.5 + m_mins;
}

Vec2 const AABB2::GetDimensions() const
{
	return Vec2( m_maxs.x - m_mins.x, m_maxs.y - m_mins.y );
}

Vec2 const AABB2::GetNearestPoint( Vec2 const& refernencePosition ) const
{
	return Vec2(
		Clamp( refernencePosition.x, m_mins.x, m_maxs.x ),
		Clamp( refernencePosition.y, m_mins.y, m_maxs.y )
	);
}

Vec2 const AABB2::GetPointAtUV( Vec2 const& uv ) const
{
	return Vec2(
		Interpolate( m_mins.x, m_maxs.x, uv.x ),
		Interpolate( m_mins.y, m_maxs.y, uv.y )
	);
}

Vec2 const AABB2::GetUVForPoint( Vec2 const& point ) const
{
	return Vec2(
		GetFractionWithinRange( point.x, m_mins.x, m_maxs.x ),
		GetFractionWithinRange( point.y, m_mins.y, m_maxs.y )
	);
}

void AABB2::Translate( Vec2 const& trasnlationToApply )
{
	m_mins += trasnlationToApply;
	m_maxs += trasnlationToApply;
}

void AABB2::SetCenter( Vec2 const& newCenter )
{
	Vec2 disp = newCenter - GetCenter();
	m_mins += disp;
	m_maxs += disp;
}

void AABB2::SetDimensions( Vec2 const& newDimensions )
{
	Vec2 expand = newDimensions - GetDimensions();
	m_mins -= expand * 0.5f;
	m_maxs += expand * 0.5f;
}

void AABB2::StretchToIncludePoint( Vec2 const& point )
{
	if (GetNearestPoint( point ) == point)
		return;

	if (point.x > m_maxs.x)
		m_maxs.x = point.x;
	if (point.y > m_maxs.y)
		m_maxs.y = point.y;
	if (point.x < m_mins.x)
		m_mins.x = point.x;
	if (point.y < m_mins.y)
		m_mins.y = point.y;
}
