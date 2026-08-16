#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/EngineCommon.hpp"

AABB3::AABB3( AABB3 const& copyForm )
	: m_mins( copyForm.m_mins )
	, m_maxs( copyForm.m_maxs )
{
}

AABB3::AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ )
{
	m_mins = Vec3( minX, minY, minZ );
	m_maxs = Vec3( maxX, maxY, maxZ );
}

AABB3::AABB3( Vec3 const& mins, Vec3 const& maxs )
	: m_mins( mins )
	, m_maxs( maxs )
{
}

AABB3 const AABB3::GetMoved( Vec3 const& delta ) const
{
	return AABB3( m_mins + delta, m_maxs + delta );
}

Vec3 const AABB3::GetCenter() const
{
	return (m_maxs + m_mins) * 0.5f;
}
/*

void AABB3::StretchToIncludePoint( Vec3 const& point )
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
}*/