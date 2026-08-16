#pragma once

#include <cmath>

#include "Engine/Math/Vec2.hpp"

struct OBB2
{
public:
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;
public:
	
	OBB2() {}
	~OBB2() {}
	OBB2( OBB2 const& copyForm );
	OBB2( Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensions );

	void Translate( Vec2 translation );
	void SetCenter( Vec2 newCenter );
	void RotateAboutCenter( float rotationDeltaDegree );
	void RotateAboutPivot( Vec2 const& pivot, float rotationDeltaDegree );
};