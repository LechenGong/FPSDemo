#pragma once

#include "Engine/Math/Vec2.hpp"

struct Plane2
{
public:
	Vec2 m_normal = Vec2::ZERO;
	float m_distanceFromOrigin = 0.f;

public:
	Plane2() {};
	~Plane2() {};
	Plane2( Plane2 const& copyForm );
	explicit Plane2( Vec2 const& normal, float distanceFromOrigin );

	float GetDistanceOfPoint( Vec2 const& point ) const;
	bool IsPointInFrontOfPlane( Vec2 const& point ) const;
	bool IsRayInward( Vec2 const& rayFwd ) const;

	bool GetIntersection( Vec2& intersection, Vec2 const& rayStart, Vec2 const& rayForward, float rayLength, float& intersectionDist ) const;

	static bool DoPlane2Intersect( Plane2 const& planeA, Plane2 const& planeB );
	static Vec2 GetIntersectPos( Plane2 const& planeA, Plane2 const& planeB );
};
