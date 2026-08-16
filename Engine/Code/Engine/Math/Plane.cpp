#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane::Plane( Plane const& copyForm )
{
	m_normal = copyForm.m_normal;
	m_distanceFromOrigin = copyForm.m_distanceFromOrigin;
}

Plane::Plane( Vec3 const& normal, float distanceFromOrigin )
{
	m_normal = normal;
	m_distanceFromOrigin = distanceFromOrigin;
}

float Plane::GetDistanceOfPoint( Vec3 const& point ) const
{
	return DotProduct3D( point, m_normal ) - m_distanceFromOrigin;
}

bool Plane::IsPointInFrontOfPlane( Vec3 const& point ) const
{
	return DotProduct3D( point, m_normal ) > m_distanceFromOrigin;
}
