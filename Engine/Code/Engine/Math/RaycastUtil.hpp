#pragma once

#include <vector>

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Convex2.hpp"

struct RaycastResult2D
{
	// Basic raycast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec2	m_impactPos;
	Vec2	m_impactNormal;

	// Original raycast information (optional)
	Vec2	m_rayStartPos;
	Vec2	m_rayFwdNormal;
	float	m_rayMaxLength = 1.f;
};

struct RaycastResult3D
{
	// Basic raycast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;

	// Original raycast information (optional)
	Vec3	m_rayStartPos;
	Vec3	m_rayFwdNormal;
	float	m_rayMaxLength = 1.f;
};

bool RaycastDisc2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius );
bool RaycastSegment2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 segmentStart, Vec2 segmentFwdNormal);
bool RaycastAABB2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 aabb );
bool RaycastConvex2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2 convexHull2 );

bool RaycastGridFirst2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, void const* gridSolidity, int gridLength, int gridWidth );
bool RaycastGridAll2D( std::vector<RaycastResult2D>& results, Vec2 startPos, Vec2 fwdNormal, float maxDist, void const* gridSolidity, int gridLength, int gridWidth );

bool RaycastZCylinder3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 cylinderStart, float cylinderHeight, float cylinderRadius );
bool RaycastZPlane3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, float ZValue );
bool RaycastAABB3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, AABB3 aabb );
bool RaycastSphere3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 const& sphereCenter, float sphereRadius );
bool RaycastOBB3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, OBB3 obb );
bool RaycastCapsule3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 const& boneStart, Vec3 const& boneEnd, float capsuleRadius );