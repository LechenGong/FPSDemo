#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"

struct OBB3
{
public:
	Vec3 m_center;
	Vec3 m_iBasisNormal;
	Vec3 m_jBasisNormal;
	Vec3 m_kBasisNormal;
	Vec3 m_halfDimensions;
public:

	OBB3() {}
	~OBB3() {}
	OBB3( OBB3 const& copyForm );
	explicit OBB3( Vec3 const& center, Vec3 const& iBasisNormal, Vec3 const& jBasisNormal, Vec3 const& kBasisNormal, Vec3 const& halfDimensions );

	void SetCenter( Vec3 newCenter );

	Vec3 WorldPosToLocalPos( Vec3 worldPos ) const;
	Vec3 LocalPosToWorldPos( Vec3 localPos ) const;
	Vec3 WorldVecToLocalVec( Vec3 worldVec ) const;
	Vec3 LocalVecToWorldVec( Vec3 localVec ) const;

	AABB3 GetLocalAABB3() const;

	OBB3 const GetMoved( Vec3 const& delta ) const;
};