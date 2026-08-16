#pragma once

#include "Engine/Math/Vec3.hpp"

struct Plane
{
public:
	Vec3 m_normal = Vec3::ZERO;
	float m_distanceFromOrigin = 0.f;

public:
	Plane() {};
	~Plane() {};
	Plane( Plane const& copyForm );
	explicit Plane( Vec3 const& normal, float distanceFromOrigin );

	float GetDistanceOfPoint( Vec3 const& point ) const;
	bool IsPointInFrontOfPlane( Vec3 const& point ) const;
};