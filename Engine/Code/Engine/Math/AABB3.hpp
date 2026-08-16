#pragma once

#include <cmath>

#include "Engine/Math/Vec3.hpp"

struct AABB3
{
public:
	Vec3 m_mins;
	Vec3 m_maxs;

public:
	//Construction & Deconstruction
	AABB3() {}
	~AABB3() {}
	AABB3( AABB3 const& copyForm );
	explicit AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ );
	explicit AABB3( Vec3 const& mins, Vec3 const& maxs );

	AABB3 const GetMoved( Vec3 const& delta ) const;

	// Accessors (const methods)
// 	bool IsPointInside( Vec3 const& point ) const;
 	Vec3 const GetCenter() const;
// 	Vec3 const GetDimensions() const;
// 	Vec3 const GetNearestPoint( Vec3 const& refernencePosition ) const;
// 	Vec3 const GetPointAtUV( Vec3 const& uv ) const;
// 	Vec3 const GetUVForPoint( Vec3 const& point ) const;
	//Mutator (non-const methods)
// 	void Translate( Vec3 const& trasnlationToApply );
// 	void SetCenter( Vec3 const& newCenter );
// 	void SetDimensions( Vec3 const& newDimensions );

//	void StretchToIncludePoint( Vec3 const& point );

	static const AABB3 ZERO_TO_ONE;
};
