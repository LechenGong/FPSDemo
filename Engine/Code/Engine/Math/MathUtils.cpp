#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

unsigned int RoundToPrevPower2( unsigned int base )
{
	base--;
	base |= base >> 1;
	base |= base >> 2;
	base |= base >> 4;
	base |= base >> 8;
	base |= base >> 16;
	base = base >> 1;
	base++;
	return base;
}

unsigned int RoundToNextPower2( unsigned int base )
{
	base--;
	base |= base >> 1;
	base |= base >> 2;
	base |= base >> 4;
	base |= base >> 8;
	base |= base >> 16;
	base++;
	return base;
}

float Power( float base, unsigned int exp )
{
	float result = 1.f;
	for (;;)
	{
		if (exp & 1)
		{
			result *= base;
		}
		exp >>= 1;

		if (!exp)
			break;

		base *= base;
	}

	return result;
}

float GetUnitDegree( float degrees )
{
	while (degrees > 180.f)
		degrees -= 360.f;
	while (degrees < -180.f)
		degrees += 360.f;
	return degrees;
}

float ConvertDegreesToRadians( float degrees )
{
	return degrees * PI / 180.f;
}

float ConvertRadiansToDegrees( float radians )
{
	return radians * 180.f / PI;
}

float CosDegrees( float degrees )
{
	return cosf( ConvertDegreesToRadians( degrees ) );
}

float ACosDegrees( float value )
{
	return ConvertRadiansToDegrees( acosf( value ) );
}

float SinDegrees( float degrees )
{
	return sinf( ConvertDegreesToRadians( degrees ) );
}

float ASinDegrees( float value )
{
	return ConvertRadiansToDegrees( asinf( value ) );
}

float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y, x ) );
}

float GetShortestAngularDispDegrees( float startDegrees, float endDegrees )
{
	float rawDisp = endDegrees - startDegrees;
	return GetUnitDegree( rawDisp );
}

float GetTurnedTowardDegrees( float currentDegrees, float goalDegrees, float maxDeltaDegrees )
{
	float targetDisp = GetShortestAngularDispDegrees( currentDegrees, goalDegrees );
	if (targetDisp < 0)
		return GetUnitDegree( currentDegrees - MIN( maxDeltaDegrees, -targetDisp ) );
	return GetUnitDegree( currentDegrees + MIN( maxDeltaDegrees, targetDisp ) );
}

float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b )
{
	float dotP2D = DotProduct2D( a, b );
	float thetaRadians = acosf( dotP2D / (a.GetLength() * b.GetLength()) );
	return ConvertRadiansToDegrees( thetaRadians );
}

float GetDistance2D( Vec2 const& positionA, Vec2 const& positionB )
{
	return sqrtf( GetDistanceSquared2D( positionA, positionB ) );
}

float GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	return deltaX * deltaX + deltaY * deltaY;
}

float GetDistance3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return sqrtf( GetDistanceSquared3D( positionA, positionB ) );
}

float GetDistanceSquared3D( Vec3 const& positionA, Vec3 const& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	float deltaZ = positionA.z - positionB.z;
	return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}

float GetDistanceXY3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return sqrtf( GetDistanceXYSquared3D( positionA, positionB ) );
}

float GetDistanceXYSquared3D( Vec3 const& positionA, Vec3 const& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	return deltaX * deltaX + deltaY * deltaY;
}

int GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB )
{
	return abs( pointA.x - pointB.x ) + abs( pointA.y - pointB.y );
 }

float GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& vecToProjectOnto )
{
	return DotProduct2D( vectorToProject, vecToProjectOnto ) / vecToProjectOnto.GetLength();
}

Vec2 const GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vecToProjectOnto )
{
	return vecToProjectOnto.GetNormalized() * GetProjectedLength2D( vectorToProject, vecToProjectOnto );
}

float GetProjectedLength3D( Vec3 const& vectorToProject, Vec3 const& vecToProjectOnto )
{
	return DotProduct3D( vectorToProject, vecToProjectOnto ) / vecToProjectOnto.GetLength();
}

Vec3 const GetProjectedOnto3D( Vec3 const& vectorToProject, Vec3 const& vecToProjectOnto )
{
	return vecToProjectOnto.GetNormalized() * GetProjectedLength3D( vectorToProject, vecToProjectOnto );
}

float GetLineSegmentsDistanceSquared3D( Vec3 const& startA, Vec3 const& endA, Vec3 const& startB, Vec3 const& endB )
{
	Vec3 closestPointOnA;
	Vec3 closestPointOnB;
	return GetLineSegmentsDistanceSquared3D( startA, endA, startB, endB, closestPointOnA, closestPointOnB );
}

float GetLineSegmentsDistanceSquared3D( Vec3 const& startA, Vec3 const& endA, Vec3 const& startB, Vec3 const& endB, Vec3& closestPointOnA, Vec3& closestPointOnB )
{
	Vec3 directionA = endA - startA;
	Vec3 directionB = endB - startB;
	float const squaredLengthA = DotProduct3D( directionA, directionA );
	float const squaredLengthB = DotProduct3D( directionB, directionB );
	constexpr float minSegmentLengthSquared = 0.00000001f;

	// Degenerate segments (zero-length bones / spheres) must not divide by zero.
	if (squaredLengthA <= minSegmentLengthSquared && squaredLengthB <= minSegmentLengthSquared)
	{
		closestPointOnA = startA;
		closestPointOnB = startB;
		Vec3 const diff = closestPointOnA - closestPointOnB;
		return diff.GetLengthSquared();
	}
	if (squaredLengthA <= minSegmentLengthSquared)
	{
		closestPointOnA = startA;
		closestPointOnB = GetNearestPointOnLineSegment3D( startA, startB, endB );
		Vec3 const diff = closestPointOnA - closestPointOnB;
		return diff.GetLengthSquared();
	}
	if (squaredLengthB <= minSegmentLengthSquared)
	{
		closestPointOnB = startB;
		closestPointOnA = GetNearestPointOnLineSegment3D( startB, startA, endA );
		Vec3 const diff = closestPointOnA - closestPointOnB;
		return diff.GetLengthSquared();
	}

	Vec3 AtoB = startA - startB;
	float f = DotProduct3D( directionB, AtoB );
	float s = 0.0f, t = 0.0f;
	float c = DotProduct3D( directionA, AtoB );
	float b = DotProduct3D( directionA, directionB );
	float denom = squaredLengthA * squaredLengthB - b * b;

	if (denom != 0.0f)
	{
		s = Clamp( (b * f - c * squaredLengthB) / denom, 0.0f, 1.0f );
	}

	t = (b * s + f) / squaredLengthB;

	if (t < 0.0f)
	{
		t = 0.0f;
		s = Clamp( -c / squaredLengthA, 0.0f, 1.0f );
	}
	else if (t > 1.0f)
	{
		t = 1.0f;
		s = Clamp( (b - c) / squaredLengthA, 0.0f, 1.0f );
	}

	closestPointOnA = startA + directionA * s;
	closestPointOnB = startB + directionB * t;
	Vec3 diff = closestPointOnA - closestPointOnB;

	return diff.GetLengthSquared();
}

bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius )
{
	float toCenterVectorLengthSquared = (point - discCenter).GetLengthSquared();
	return toCenterVectorLengthSquared <= discRadius * discRadius;
}

bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& box )
{
	if (point.x >= box.m_maxs.x)
		return false;
	if (point.x <= box.m_mins.x)
		return false;
	if (point.y >= box.m_maxs.y)
		return false;
	if (point.y <= box.m_mins.y)
		return false;
	return true;
}

bool IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	Vec2 boneVector = boneEnd - boneStart;

	Vec2 startToPointVector = point - boneStart;
	if (DotProduct2D( boneVector, startToPointVector ) <= 0)
		return (startToPointVector.GetLengthSquared() >= radius * radius) ? false : true;

	Vec2 endToPointVector = point - boneEnd;
	if (DotProduct2D( boneVector, endToPointVector ) >= 0)
		return (endToPointVector.GetLengthSquared() >= radius * radius) ? false : true;

	Vec2 boneVectorNormal = boneVector.GetNormalized();
	float distToLine = DotProduct2D( startToPointVector, Vec2( -boneVectorNormal.y, boneVectorNormal.x ) );
	return (distToLine >= radius || distToLine <= -radius) ? false : true;
}

bool IsPointInsideOBB2D( Vec2 const& point, OBB2 const& box )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );

	Vec2 centerToPoint = point - box.m_center;
	float xLocal = DotProduct2D( centerToPoint, iBasis );
	if (xLocal >= box.m_halfDimensions.x || xLocal <= -box.m_halfDimensions.x)
		return false;
	float yLocal = DotProduct2D( centerToPoint, jBasis );
	if (yLocal >= box.m_halfDimensions.y || yLocal <= -box.m_halfDimensions.y)
		return false;
	return true;
}

bool IsPointInsideOBB3D( Vec3 const& point, OBB3 const& box )
{
	Vec3 localPos = box.WorldPosToLocalPos( point );

	return IsPointInsideAABB3D( localPos, box.GetLocalAABB3() );
}

bool IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 tipToPointVector = point - sectorTip;
	float tipToPointLengthSquared = tipToPointVector.GetLengthSquared();
	if (tipToPointLengthSquared > sectorRadius * sectorRadius)
		return false;
	float tipToPointDegree = tipToPointVector.GetOrientationDegrees();
	float shortestDegreeToForward = GetShortestAngularDispDegrees( sectorForwardDegrees, tipToPointDegree );
	return shortestDegreeToForward <= sectorApertureDegrees * 0.5f && shortestDegreeToForward >= -sectorApertureDegrees * 0.5f;
}

bool IsPointInsideDirectedSector2D( Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 tipToPointVector = point - sectorTip;
	float tipToPointLengthSquared = tipToPointVector.GetLengthSquared();
	if (tipToPointLengthSquared > sectorRadius * sectorRadius)
		return false;
	Vec2 tipToPointNormal = tipToPointVector.GetNormalized();
	return ConvertRadiansToDegrees( acosf( DotProduct2D( tipToPointNormal, sectorForwardNormal ) ) ) < sectorApertureDegrees * 0.5f;
}

bool IsPointInsideSphere3D( Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius )
{
	Vec3 centerToPointVec = point - sphereCenter;
	return centerToPointVec.GetLength() < sphereRadius;
}

bool IsPointInsideAABB3D( Vec3 const& point, AABB3 const& box )
{
	if (point.x >= box.m_maxs.x)
		return false;
	if (point.x <= box.m_mins.x)
		return false;
	if (point.y >= box.m_maxs.y)
		return false;
	if (point.y <= box.m_mins.y)
		return false;
	if (point.z >= box.m_maxs.z)
		return false;
	if (point.z <= box.m_mins.z)
		return false;
	return true;
}

bool IsPointInsideCapsule3D( Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius )
{
	Vec3 boneVector = boneEnd - boneStart;

	Vec3 startToPointVector = point - boneStart;
	if (DotProduct3D( boneVector, startToPointVector ) <= 0)
		return (startToPointVector.GetLengthSquared() >= radius * radius) ? false : true;

	Vec3 endToPointVector = point - boneEnd;
	if (DotProduct3D( boneVector, endToPointVector ) >= 0)
		return (endToPointVector.GetLengthSquared() >= radius * radius) ? false : true;

	Vec3 boneVectorNormal = boneVector.GetNormalized();
	Vec3 projectionOnBone = DotProduct3D( startToPointVector, boneVectorNormal ) * boneVectorNormal;
	Vec3 distToBone = startToPointVector - projectionOnBone;
	
	return (distToBone.GetLengthSquared() <= radius * radius);
}

bool IsPointInsideHexigon3D( Vec3 const& point, Vec3 const& center, float radius )
{
	float dx = point.x - center.x;
	float dy = point.y - center.y;

	float dist = std::sqrt( dx * dx + dy * dy );
	float angle = std::atan2( dy, dx );

	if (dist > radius) 
		return false;

	if (angle < 0) 
	{
		angle += 2.0f * static_cast<float>(PI);
	}
	
	float sectorAngle = static_cast<float>(PI) / 3.0f;
	float normalizedAngle = std::fmod( angle, sectorAngle );

	float maxDist = radius * std::cos( normalizedAngle );

	return dist <= maxDist;
}

bool DoDiscsOverlap2D( Vec2 const& centerA, float const& radiusA, Vec2 const& centerB, float const& radiusB )
{
	return GetDistanceSquared2D( centerA, centerB ) < ((radiusA + radiusB) * (radiusA + radiusB));
}

bool DoAABBsOverlap2D( AABB2 const& aabbA, AABB2 const& aabbB )
{
	Vec2 AX = Vec2( aabbA.m_mins.x, aabbA.m_maxs.x );
	Vec2 AY = Vec2( aabbA.m_mins.y, aabbA.m_maxs.y );
	Vec2 BX = Vec2( aabbB.m_mins.x, aabbB.m_maxs.x );
	Vec2 BY = Vec2( aabbB.m_mins.y, aabbB.m_maxs.y );

	float a = MAX( AX.x, BX.x );
	float b = MIN( AX.y, BX.y );
	bool xOverlap = (a < b);
	a = MAX( AY.x, BY.x );
	b = MIN( AY.y, BY.y );
	bool yOverlap = (a < b);
	return xOverlap && yOverlap;
}

bool DoDiscOverlapAABB2D( AABB2 const& aabb, Vec2 const& center, float const& radius )
{
	if (IsPointInsideAABB2D( center, aabb ))
		return true;

	Vec2 point = GetNearestPointOnAABB2D( center, aabb );
	return (point - center).GetLengthSquared() < radius * radius;
}

bool DoDiscOverlapOBB2D( Vec2 const& discCenter, float discRadius, OBB2 const& box )
{
	OBB2 fatOBB = box;
	fatOBB.m_halfDimensions += Vec2( discRadius, discRadius );
	return IsPointInsideOBB2D( discCenter, fatOBB );
}

bool DoDiscOverlapCapsule2D( Vec2 const& discCenter, float discRadius, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	return IsPointInsideCapsule2D( discCenter, boneStart, boneEnd, radius + discRadius );
}

bool DoDiscOverlapDirectedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	return IsPointInsideOrientedSector2D( discCenter, sectorTip, sectorForwardDegrees, sectorApertureDegrees, sectorRadius + discRadius );
}

bool DoDiscOverlapOrientedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	return IsPointInsideDirectedSector2D( discCenter, sectorTip, sectorForwardNormal, sectorApertureDegrees, sectorRadius + discRadius );
}

bool DoSpheresOverlap3D( Vec3 const& centerA, float const& radiusA, Vec3 const& centerB, float const& radiusB )
{
	return GetDistanceSquared3D( centerA, centerB ) < ((radiusA + radiusB) * (radiusA + radiusB));
}

bool DoSpheresOverlap3D( Vec3 const& centerA, float const& radiusA, Vec3 const& centerB, float const& radiusB, Vec3& out_mtv_XY )
{
	Vec3 direction = centerB - centerA;

	float distanceSquaredXY = (direction.x * direction.x) + (direction.y * direction.y);
	float radiusSum = radiusA + radiusB;

	if (distanceSquaredXY < (radiusSum * radiusSum))
	{
		float distanceXY = std::sqrt( distanceSquaredXY );

		if (distanceXY == 0.0f)
		{
			out_mtv_XY = Vec3( radiusSum, 0.0f, 0.0f );
		}
		else
		{
			float overlap = radiusSum - distanceXY;

			Vec3 directionXY( direction.x, direction.y, 0.f );

			out_mtv_XY = directionXY * overlap;
		}

		return true;
	}

	out_mtv_XY = Vec3( 0.0f, 0.0f, 0.0f );
	return false;
}

bool DoAABBsOverlap3D( AABB3 const& aabbA, AABB3 const& aabbB )
{
	return (aabbA.m_mins.x <= aabbB.m_maxs.x && aabbA.m_maxs.x >= aabbB.m_mins.x) &&
		(aabbA.m_mins.y <= aabbB.m_maxs.y && aabbA.m_maxs.y >= aabbB.m_mins.y) &&
		(aabbA.m_mins.z <= aabbB.m_maxs.z && aabbA.m_maxs.z >= aabbB.m_mins.z);
}

bool DoZCylindersOverlap3D( Vec3 const& cylinderAStart, float cylinderAHeight, float cylinderARadius, Vec3 const& cylinderBStart, float cylinderBHeight, float cylinderBRadius )
{
	float actorABottom = cylinderAStart.z;
	float actorATop = actorABottom + cylinderAHeight;
	float actorBBottom = cylinderBStart.z;
	float actorBTop = actorBBottom + cylinderBHeight;

	Vec2 actorACenter = cylinderAStart.GetXY();
	Vec2 actorBCenter = cylinderBStart.GetXY();

	return actorATop > actorBBottom &&
		actorABottom < actorBTop&&
		DoDiscsOverlap2D( actorACenter, cylinderARadius, actorBCenter, cylinderBRadius );
}

bool DoSphereAndAABBOverlap3D( Vec3 const& center, float const& radius, AABB3 const& aabb )
{
	Vec3 point = GetNearestPointOnAABB3D( center, aabb );
	return (point - center).GetLengthSquared() < radius * radius;
}

bool DoZCylinderAndAABBOverlap3D( Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius, AABB3 const& aabb )
{
	Vec2 discPos = cylinderStart.GetXY();
	FloatRange cylinderZRange( cylinderStart.z, cylinderStart.z + cylinderHeight );
	FloatRange aabbZRange( aabb.m_mins.z, aabb.m_maxs.z );
	AABB2 aabb2( aabb.m_mins.GetXY(), aabb.m_maxs.GetXY() );
	return DoFloatRangeOverlap( cylinderZRange, aabbZRange ) && DoDiscOverlapAABB2D( aabb2, discPos, cylinderRadius );
}

bool DoZCylinderAndSphereOverlap3D( Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius, Vec3 const& center, float const& radius )
{
	Vec3 point = GetNearestPointOnZCylinder3D( center, cylinderStart, cylinderHeight, cylinderRadius );
	return (point - center).GetLengthSquared() < radius * radius;
}

bool DoPlaneOverlapZCylinder3D( Plane const& plane, Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius )
{
	Vec3 planeNormalXY = Vec3( plane.m_normal.GetXY().GetNormalized() );

	std::vector<Vec3>intercepts;
	intercepts.resize( 4 );
	intercepts[0] = planeNormalXY * cylinderRadius + cylinderStart;
	intercepts[1] = -planeNormalXY * cylinderRadius + cylinderStart;
	intercepts[2] = planeNormalXY * cylinderRadius + cylinderStart + Vec3( 0.f, 0.f, cylinderHeight );
	intercepts[3] = -planeNormalXY * cylinderRadius + cylinderStart + Vec3( 0.f, 0.f, cylinderHeight );
	float dist = plane.GetDistanceOfPoint( intercepts[0] );
	for (int i = 1; i < 4; i++)
	{
		float thisDist = plane.GetDistanceOfPoint( intercepts[i] );
		if (dist > 0.f && thisDist < 0.f ||
			dist < 0.f && thisDist > 0.f)
		{
			return true;
		}
	}
	return false;
}

bool DoPlaneOverlapAABB3D( Plane const& plane, AABB3 const& aabb )
{
	std::vector<Vec3>intercepts;
	intercepts.resize( 8 );
	intercepts[0] = Vec3( aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_maxs.z );
	intercepts[1] = Vec3( aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z );
	intercepts[2] = Vec3( aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z );
	intercepts[3] = Vec3( aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z );
	intercepts[4] = Vec3( aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z );
	intercepts[5] = Vec3( aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z );
	intercepts[6] = Vec3( aabb.m_mins.x, aabb.m_mins.y, aabb.m_mins.z );
	intercepts[7] = Vec3( aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z );
	float dist = plane.GetDistanceOfPoint( intercepts[0] );
	for (int i = 1; i < 8; i++)
	{
		float thisDist = plane.GetDistanceOfPoint( intercepts[i] );
		if (dist > 0.f && thisDist < 0.f ||
			dist < 0.f && thisDist > 0.f)
		{
			return true;
		}
	}
	return false;
}

bool DoPlaneOverlapSphere3D( Plane const& plane, Vec3 const& center, float radius )
{
	return abs( plane.GetDistanceOfPoint( center ) ) < radius;
}

bool DoPlaneOverlapOBB3D( Plane const& plane, OBB3 const& obb )
{
	std::vector<Vec3>intercepts;
	intercepts.resize( 8 );
	intercepts[0] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[1] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[2] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[3] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[4] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[5] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[6] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[7] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	float dist = plane.GetDistanceOfPoint( intercepts[0] );
	for (int i = 1; i < 8; i++)
	{
		float thisDist = plane.GetDistanceOfPoint( intercepts[i] );
		if (dist > 0.f && thisDist < 0.f ||
			dist < 0.f && thisDist > 0.f)
		{
			return true;
		}
	}
	return false;
}

bool DoOBBsOverlap3D( OBB3 const& obbA, OBB3 const& obbB )
{
	Mat44 rotationMatrix;
	Mat44 absRotationMatrix;

	rotationMatrix.SetIJK3D(
		Vec3(
			DotProduct3D( obbA.m_iBasisNormal, obbB.m_iBasisNormal ),
			DotProduct3D( obbA.m_jBasisNormal, obbB.m_iBasisNormal ),
			DotProduct3D( obbA.m_kBasisNormal, obbB.m_iBasisNormal )
		),
		Vec3(
			DotProduct3D( obbA.m_iBasisNormal, obbB.m_jBasisNormal ),
			DotProduct3D( obbA.m_jBasisNormal, obbB.m_jBasisNormal ),
			DotProduct3D( obbA.m_kBasisNormal, obbB.m_jBasisNormal )
		),
		Vec3(
			DotProduct3D( obbA.m_iBasisNormal, obbB.m_kBasisNormal ),
			DotProduct3D( obbA.m_jBasisNormal, obbB.m_kBasisNormal ),
			DotProduct3D( obbA.m_kBasisNormal, obbB.m_kBasisNormal )
		)
	);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			absRotationMatrix.SetElement( i, j, abs( rotationMatrix.GetElement( i, j ) ) );
			if (absRotationMatrix.GetElement( i, j ) < 0.0001f)
			{
				absRotationMatrix.SetElement( i, j, 0.f );
			}
		}
	}

	Vec3 translation = obbB.m_center - obbA.m_center;

	Vec3 translationInAFrame = Vec3(
		DotProduct3D( translation, obbA.m_iBasisNormal ),
		DotProduct3D( translation, obbA.m_jBasisNormal ),
		DotProduct3D( translation, obbA.m_kBasisNormal )
	);

	// Test OBBA local axes
	for (int i = 0; i < 3; i++)
	{
		float halfExtentA = obbA.m_halfDimensions[i];
		float halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( i, 0 ) +
			obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( i, 1 ) +
			obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( i, 2 );

		if (abs( translationInAFrame[i] ) > halfExtentA + halfExtentB)
			return false;
	}

	// Test OBBB local axes
	for (int i = 0; i < 3; i++)
	{
		float halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 0, i ) +
			obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 1, i ) +
			obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, i );
		float halfExtentB = obbB.m_halfDimensions[i];
		float projectedTranslation = translationInAFrame[0] * rotationMatrix.GetElement( 0, i ) +
			translationInAFrame[1] * rotationMatrix.GetElement( 1, i ) +
			translationInAFrame[2] * rotationMatrix.GetElement( 2, i );

		if (abs( projectedTranslation ) > halfExtentA + halfExtentB)
			return false;
	}

	// Test cross products of axes for separation (using SAT)
	// Break if no overlap on axis (overlapping requires projections also overlap on all axes)
	float halfExtentA, halfExtentB;

	// A0 x B0
	halfExtentA = obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 1, 0 );
	halfExtentB = obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 1 );
	if (abs( translationInAFrame[2] * rotationMatrix.GetElement( 1, 0 ) - translationInAFrame[1] * rotationMatrix.GetElement( 2, 0 ) ) > halfExtentA + halfExtentB) return false;

	// A0 x B1
	halfExtentA = obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 1 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 1, 1 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 0, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 0 );
	if (abs( translationInAFrame[2] * rotationMatrix.GetElement( 1, 1 ) - translationInAFrame[1] * rotationMatrix.GetElement( 2, 1 ) ) > halfExtentA + halfExtentB) return false;

	// A0 x B2
	halfExtentA = obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 2 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 1, 2 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 0, 1 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 0 );
	if (abs( translationInAFrame[2] * rotationMatrix.GetElement( 1, 2 ) - translationInAFrame[1] * rotationMatrix.GetElement( 2, 2 ) ) > halfExtentA + halfExtentB) return false;

	// A1 x B0
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 0 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 0 );
	halfExtentB = obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, 1 );
	//if (abs( translationInAFrame[0] * rotationMatrix.GetElement( 2, 0 ) - translationInAFrame[2] * rotationMatrix.GetElement( 0, 0 ) ) > halfExtentA + halfExtentB) return false;

	// A1 x B1
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 1 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 1 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[0] * rotationMatrix.GetElement( 2, 1 ) - translationInAFrame[2] * rotationMatrix.GetElement( 0, 1 ) ) > halfExtentA + halfExtentB) return false;

	// A1 x B2
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 2 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 2 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 1 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[0] * rotationMatrix.GetElement( 2, 2 ) - translationInAFrame[2] * rotationMatrix.GetElement( 0, 2 ) ) > halfExtentA + halfExtentB) return false;

	// A2 x B0
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 1, 0 ) + obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 0 );
	halfExtentB = obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, 1 );
	if (abs( translationInAFrame[1] * rotationMatrix.GetElement( 0, 0 ) - translationInAFrame[0] * rotationMatrix.GetElement( 1, 0 ) ) > halfExtentA + halfExtentB) return false;

	// A2 x B1
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 1, 1 ) + obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 1 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[1] * rotationMatrix.GetElement( 0, 1 ) - translationInAFrame[0] * rotationMatrix.GetElement( 1, 1 ) ) > halfExtentA + halfExtentB) return false;

	// A2 x B2
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 1, 2 ) + obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 2 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 1 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[1] * rotationMatrix.GetElement( 0, 2 ) - translationInAFrame[0] * rotationMatrix.GetElement( 1, 2 ) ) > halfExtentA + halfExtentB) return false;

	return true;
}

bool DoOBBsOverlap3D( OBB3 const& obbA, OBB3 const& obbB, Vec3& out_mtv )
{
	Mat44 rotationMatrix;
	Mat44 absRotationMatrix;

	rotationMatrix.SetIJK3D(
		Vec3(
			DotProduct3D( obbA.m_iBasisNormal, obbB.m_iBasisNormal ),
			DotProduct3D( obbA.m_jBasisNormal, obbB.m_iBasisNormal ),
			DotProduct3D( obbA.m_kBasisNormal, obbB.m_iBasisNormal )
		),
		Vec3(
			DotProduct3D( obbA.m_iBasisNormal, obbB.m_jBasisNormal ),
			DotProduct3D( obbA.m_jBasisNormal, obbB.m_jBasisNormal ),
			DotProduct3D( obbA.m_kBasisNormal, obbB.m_jBasisNormal )
		),
		Vec3(
			DotProduct3D( obbA.m_iBasisNormal, obbB.m_kBasisNormal ),
			DotProduct3D( obbA.m_jBasisNormal, obbB.m_kBasisNormal ),
			DotProduct3D( obbA.m_kBasisNormal, obbB.m_kBasisNormal )
		)
	);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			absRotationMatrix.SetElement( i, j, abs( rotationMatrix.GetElement( i, j ) ) );
			if (absRotationMatrix.GetElement( i, j ) < 0.0001f)
			{
				absRotationMatrix.SetElement( i, j, 0.f );
			}
		}
	}

	Vec3 translation = obbB.m_center - obbA.m_center;

	Vec3 translationInAFrame = Vec3(
		DotProduct3D( translation, obbA.m_iBasisNormal ),
		DotProduct3D( translation, obbA.m_jBasisNormal ),
		DotProduct3D( translation, obbA.m_kBasisNormal )
	);

	float minOverlap = 999999.f;
	Vec3 minTranslationAxis;

	// Test OBBA local axes
	for (int i = 0; i < 3; i++)
	{
		float halfExtentA = obbA.m_halfDimensions[i];
		float halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( i, 0 ) +
			obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( i, 1 ) +
			obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( i, 2 );

		float overlap = halfExtentA + halfExtentB - abs( translationInAFrame[i] );

		if (overlap < 0.f)
			return false;

		if (overlap < minOverlap)
		{
			minOverlap = overlap;
			Vec3 thisAxis;
			switch (i)
			{
			case 0: thisAxis = obbA.m_iBasisNormal; break;
			case 1: thisAxis = obbA.m_jBasisNormal; break;
			case 2: thisAxis = obbA.m_kBasisNormal; break;
			default: break;
			}
			minTranslationAxis = thisAxis * (translationInAFrame[i] > 0 ? 1.0f : -1.0f);
		}
	}

	// Test OBBB local axes
	for (int i = 0; i < 3; i++)
	{
		float halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 0, i ) +
			obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 1, i ) +
			obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, i );
		float halfExtentB = obbB.m_halfDimensions[i];
		float projectedTranslation = translationInAFrame[0] * rotationMatrix.GetElement( 0, i ) +
			translationInAFrame[1] * rotationMatrix.GetElement( 1, i ) +
			translationInAFrame[2] * rotationMatrix.GetElement( 2, i );

		// 		if (abs( projectedTranslation ) > halfExtentA + halfExtentB)
		// 			return false;
		float overlap = halfExtentA + halfExtentB - abs( projectedTranslation );

		if (overlap < 0.f)
			return false;

		if (overlap < minOverlap)
		{
			minOverlap = overlap;
			Vec3 thisAxis;
			switch (i)
			{
			case 0: thisAxis = obbB.m_iBasisNormal; break;
			case 1: thisAxis = obbB.m_jBasisNormal; break;
			case 2: thisAxis = obbB.m_kBasisNormal; break;
			default: break;
			}
			minTranslationAxis = thisAxis * (projectedTranslation > 0 ? 1.0f : -1.0f);
		}
	}

	// Test cross products of axes for separation (using SAT)
	// Break if no overlap on axis (overlapping requires projections also overlap on all axes)
	float halfExtentA, halfExtentB;

	// A0 x B0
	halfExtentA = obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 1, 0 );
	halfExtentB = obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 1 );
	if (abs( translationInAFrame[2] * rotationMatrix.GetElement( 1, 0 ) - translationInAFrame[1] * rotationMatrix.GetElement( 2, 0 ) ) > halfExtentA + halfExtentB) 
		return false;

	// A0 x B1
	halfExtentA = obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 1 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 1, 1 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 0, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 0 );
	if (abs( translationInAFrame[2] * rotationMatrix.GetElement( 1, 1 ) - translationInAFrame[1] * rotationMatrix.GetElement( 2, 1 ) ) > halfExtentA + halfExtentB) 
		return false;

	// A0 x B2
	halfExtentA = obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 2 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 1, 2 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 0, 1 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 0 );
	if (abs( translationInAFrame[2] * rotationMatrix.GetElement( 1, 2 ) - translationInAFrame[1] * rotationMatrix.GetElement( 2, 2 ) ) > halfExtentA + halfExtentB)
		return false;

	// A1 x B0
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 0 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 0 );
	halfExtentB = obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, 1 );
	float a = abs( translationInAFrame[0] * rotationMatrix.GetElement( 2, 0 ) - translationInAFrame[2] * rotationMatrix.GetElement( 0, 0 ) );
	if ( a > halfExtentA + halfExtentB) 
		//return false;

	// A1 x B1
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 1 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 1 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[0] * rotationMatrix.GetElement( 2, 1 ) - translationInAFrame[2] * rotationMatrix.GetElement( 0, 1 ) ) > halfExtentA + halfExtentB) 
		return false;

	// A1 x B2
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 2 ) + obbA.m_halfDimensions[2] * absRotationMatrix.GetElement( 0, 2 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 1 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[0] * rotationMatrix.GetElement( 2, 2 ) - translationInAFrame[2] * rotationMatrix.GetElement( 0, 2 ) ) > halfExtentA + halfExtentB) 
		return false;

	// A2 x B0
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 1, 0 ) + obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 0 );
	halfExtentB = obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[2] * absRotationMatrix.GetElement( 2, 1 );
	if (abs( translationInAFrame[1] * rotationMatrix.GetElement( 0, 0 ) - translationInAFrame[0] * rotationMatrix.GetElement( 1, 0 ) ) > halfExtentA + halfExtentB) 
		return false;

	// A2 x B1
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 1, 1 ) + obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 1 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 2 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[1] * rotationMatrix.GetElement( 0, 1 ) - translationInAFrame[0] * rotationMatrix.GetElement( 1, 1 ) ) > halfExtentA + halfExtentB) 
		return false;

	// A2 x B2
	halfExtentA = obbA.m_halfDimensions[0] * absRotationMatrix.GetElement( 1, 2 ) + obbA.m_halfDimensions[1] * absRotationMatrix.GetElement( 0, 2 );
	halfExtentB = obbB.m_halfDimensions[0] * absRotationMatrix.GetElement( 2, 1 ) + obbB.m_halfDimensions[1] * absRotationMatrix.GetElement( 2, 0 );
	if (abs( translationInAFrame[1] * rotationMatrix.GetElement( 0, 2 ) - translationInAFrame[0] * rotationMatrix.GetElement( 1, 2 ) ) > halfExtentA + halfExtentB) 
		return false;

	out_mtv = minTranslationAxis * minOverlap;

	return true;
}

bool DoCapsuleOverlapOBB3D( Vec3 const& boneStart, Vec3 const& boneEnd, float radius, OBB3 const& obb )
{
	Vec3 closestPointOnOBB = GetNearestPointOnOBBToLineSegment3D( boneStart, boneEnd, obb );
	Vec3 cloestPointOnCapsuleBone = GetNearestPointOnLineSegment3D( closestPointOnOBB, boneStart, boneEnd );
	float distanceToCapsuleBone = (closestPointOnOBB - cloestPointOnCapsuleBone).GetLength();

	if (distanceToCapsuleBone < radius)
		return true;

	return false;
}

// out_mtv guarantee pointing from capsule to obb
bool DoCapsuleOverlapOBB3D( Vec3 const& boneStart, Vec3 const& boneEnd, float radius, OBB3 const& obb, Vec3& out_mtv )
{
	Vec3 closestPointOnOBB = GetNearestPointOnOBBToLineSegment3D( boneStart, boneEnd, obb );
	Vec3 cloestPointOnCapsuleBone = GetNearestPointOnLineSegment3D( closestPointOnOBB, boneStart, boneEnd );
	Vec3 direction = closestPointOnOBB - cloestPointOnCapsuleBone;
	float distanceToCapsuleBone = direction.GetLength();

	if (distanceToCapsuleBone < radius)
	{
		if (distanceToCapsuleBone <= 0.000001f)
		{
			// Bone sample lies inside the OBB; use center-to-bone as a fallback axis.
			Vec3 fallback = cloestPointOnCapsuleBone - obb.m_center;
			if (fallback.GetLengthSquared() <= 0.000001f)
			{
				fallback = Vec3( 0.f, 0.f, 1.f );
			}
			out_mtv = fallback.GetNormalized() * -radius;
		}
		else
		{
			out_mtv = direction.GetNormalized() * (distanceToCapsuleBone - radius);
		}
		return true;
	}

	return false;
}

bool DoCapsulesOverlap3D( Vec3 const& boneStartA, Vec3 const& boneEndA, float radiusA, Vec3 const& boneStartB, Vec3 const& boneEndB, float radiusB )
{
	float radiusSum = radiusA + radiusB;
	float radiusSumSquared = radiusSum * radiusSum;

	float distanceSquared = GetLineSegmentsDistanceSquared3D( boneStartA, boneEndA, boneStartB, boneEndB );

	return distanceSquared <= radiusSumSquared;
}

// mtv direction is from A to B
bool DoCapsulesOverlap3D( Vec3 const& boneStartA, Vec3 const& boneEndA, float radiusA, Vec3 const& boneStartB, Vec3 const& boneEndB, float radiusB, Vec3& out_mtv_XY )
{
	Vec3 closestPointOnA, closestPointOnB;
	float distanceSquared = GetLineSegmentsDistanceSquared3D( boneStartA, boneEndA, boneStartB, boneEndB, closestPointOnA, closestPointOnB );

	float radiusSum = radiusA + radiusB;
	float radiusSumSquared = radiusSum * radiusSum;

	if (distanceSquared > radiusSumSquared) 
	{
		out_mtv_XY = Vec3( 0.f, 0.f, 0.f );
		return false;
	}

	float overlapDistance = radiusSum - sqrt( distanceSquared );

	Vec3 direction = closestPointOnA - closestPointOnB;

	Vec3 directionXY = Vec3( direction.x, direction.y, 0.f );

// 	if (directionXY.GetLengthSquared() > 0.0f) 
// 	{
// 		directionXY = directionXY.GetNormalized();
// 	}
// 	else 
// 	{
// 		directionXY = Vec3( 1.0f, 0.0f, 0.0f );
// 	}

	out_mtv_XY = directionXY * overlapDistance;

	return true;
}

bool DoCapsuleOverlapSphere3D( Vec3 const& boneStart, Vec3 const& boneEnd, float capsuleRadius, Vec3 const& center, float sphereRadius )
{
	Vec3 closestPoint = GetNearestPointOnLineSegment3D( center, boneStart, boneEnd );
	Vec3 sphereToLineSeg = closestPoint - center;
	float distanceSquared = sphereToLineSeg.GetLengthSquared();

	float radiusSum = capsuleRadius + sphereRadius;
	float radiusSumSquared = radiusSum * radiusSum;

	return distanceSquared <= radiusSumSquared;
}

// mtv direction is from A to B
bool DoCapsuleOverlapSphere3D( Vec3 const& boneStart, Vec3 const& boneEnd, float capsuleRadius, Vec3 const& center, float sphereRadius, Vec3& out_mtv_XY )
{
	Vec3 closestPoint = GetNearestPointOnLineSegment3D( center, boneStart, boneEnd );
	Vec3 sphereToLineSeg = closestPoint - center;
	float distanceSquared = sphereToLineSeg.GetLengthSquared();

	float radiusSum = capsuleRadius + sphereRadius;
	float radiusSumSquared = radiusSum * radiusSum;

	if (distanceSquared > radiusSumSquared)
	{
		out_mtv_XY = Vec3( 0.f, 0.f, 0.f );
		return false;
	}

	float overlapDistance = radiusSum - sqrt( distanceSquared );

	Vec3 directionXY = Vec3( sphereToLineSeg.x, sphereToLineSeg.y, 0.f );

// 	if (directionXY.GetLengthSquared() > 0.f)
// 	{
// 		directionXY = directionXY.GetNormalized();
// 	}

	out_mtv_XY = directionXY * overlapDistance;

	return true;
}


Vec2 const GetNearestPointOnDisc2D( Vec2 const& referencePos, Vec2 const& discCenter, float discRadius )
{
	Vec2 centerToPointVector = referencePos - discCenter;
	Vec2 centerToPointNormal = centerToPointVector.GetNormalized();
	float centerToPointLength = centerToPointVector.GetLength();
	if (centerToPointLength <= discRadius)
		return referencePos;
	return discCenter + centerToPointNormal * Clamp( centerToPointLength, 0.f, discRadius );
}

Vec2 const GetNearestPointOnAABB2D( Vec2 const& referencePos, AABB2 const& box )
{
	return Vec2(
		Clamp( referencePos.x, box.m_mins.x, box.m_maxs.x ),
		Clamp( referencePos.y, box.m_mins.y, box.m_maxs.y )
	);
}

Vec2 const GetNearestPointOnInfiniteLine2D( Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine )
{
	Vec2 lineVector = anotherPointOnLine - pointOnLine;
	Vec2 startToPointVector = referencePos - pointOnLine;
	Vec2 lineVectorNormal = lineVector.GetNormalized();
	return pointOnLine + lineVectorNormal * DotProduct2D( startToPointVector, lineVectorNormal );
}

Vec2 const GetNearestPointOnLineSegment2D( Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd )
{
	Vec2 lineVector = lineSegEnd - lineSegStart;

	Vec2 startToPointVector = referencePos - lineSegStart;

	if (DotProduct2D( lineVector, startToPointVector ) <= 0)
		return lineSegStart;

	Vec2 endToPointVector = referencePos - lineSegEnd;
	if (DotProduct2D( lineVector, endToPointVector ) >= 0)
		return lineSegEnd;

	Vec2 lineVectorNormal = lineVector.GetNormalized();
	return lineSegStart + lineVectorNormal * DotProduct2D( startToPointVector, lineVectorNormal );
}

Vec2 const GetNearestPointOnCapsule2D( Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	Vec2 boneVector = boneEnd - boneStart;

	Vec2 startToPointVector = referencePos - boneStart;
	if (DotProduct2D( boneVector, startToPointVector ) <= 0)
		return boneStart + startToPointVector.GetClamped( radius );

	Vec2 endToPointVector = referencePos - boneEnd;
	if (DotProduct2D( boneVector, endToPointVector ) >= 0)
		return boneEnd + endToPointVector.GetClamped( radius );

	Vec2 boneVectorNormal = boneVector.GetNormalized();
	Vec2 nearestPointOnLine = boneStart + boneVectorNormal * DotProduct2D( startToPointVector, boneVectorNormal );
	float distToLine = DotProduct2D( startToPointVector, Vec2( -boneVectorNormal.y, boneVectorNormal.x ) );
	return nearestPointOnLine + Clamp( distToLine, -radius, radius ) * Vec2( -boneVectorNormal.y, boneVectorNormal.x );
}

Vec2 const GetNearestPointOnOBB2D( Vec2 const& referencePos, OBB2 const& box )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );

	Vec2 centerToPoint = referencePos - box.m_center;
	float xLocal = DotProduct2D( centerToPoint, iBasis );
	float yLocal = DotProduct2D( centerToPoint, jBasis );
	return box.m_center + Clamp( xLocal, -box.m_halfDimensions.x, box.m_halfDimensions.x ) * iBasis + Clamp( yLocal, -box.m_halfDimensions.y, box.m_halfDimensions.y ) * jBasis;
}

Vec3 const GetNearestPointOnSphere3D( Vec3 const& referencePos, Vec3 const& sphereCenter, float sphereRadius )
{
	Vec3 centerToPointVector = referencePos - sphereCenter;
	float centerToPointLength = centerToPointVector.GetLength();

	if (centerToPointLength <= sphereRadius)
	{
		return referencePos;
	}

	Vec3 centerToPointNormal = centerToPointVector / centerToPointLength;
	return sphereCenter + centerToPointNormal * sphereRadius;
}

Vec3 const GetNearestPointOnAABB3D( Vec3 const& referencePos, AABB3 const& box )
{
	return Vec3(
		Clamp( referencePos.x, box.m_mins.x, box.m_maxs.x ),
		Clamp( referencePos.y, box.m_mins.y, box.m_maxs.y ),
		Clamp( referencePos.z, box.m_mins.z, box.m_maxs.z )
	);
}

Vec3 const GetNearestPointOnLineSegment3D( Vec3 const& referencePos, Vec3 const& lineSegStart, Vec3 const& lineSegEnd )
{
	Vec3 lineVector = lineSegEnd - lineSegStart;

	Vec3 startToPointVector = referencePos - lineSegStart;

	if (DotProduct3D( lineVector, startToPointVector ) <= 0)
		return lineSegStart;

	Vec3 endToPointVector = referencePos - lineSegEnd;
	if (DotProduct3D( lineVector, endToPointVector ) >= 0)
		return lineSegEnd;

	Vec3 lineVectorNormal = lineVector.GetNormalized();
	return lineSegStart + lineVectorNormal * DotProduct3D( startToPointVector, lineVectorNormal );
}

Vec3 const GetNearestPointOnZCylinder3D( Vec3 const& referencePos, Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius )
{
	Vec2 nearestDisc2D = GetNearestPointOnDisc2D( referencePos.GetXY(), cylinderStart.GetXY(), cylinderRadius );
	float nearestZ = Clamp( referencePos.z, cylinderStart.z, cylinderStart.z + cylinderHeight );
	return Vec3( nearestDisc2D.x, nearestDisc2D.y, nearestZ );
}

Vec3 const GetNearestPointOnOBBToLineSegment3D( Vec3 const& lineSegStart, Vec3 const& lineSegEnd, OBB3 const& obb )
{
	Vec3 lineCenter = (lineSegStart + lineSegEnd) * 0.5f;
	Vec3 delta = lineCenter - obb.m_center;
	Vec3 closestPoint = obb.m_center;

	float distAlongI = DotProduct3D( delta, obb.m_iBasisNormal );
	distAlongI = MAX( -obb.m_halfDimensions.x, MIN( obb.m_halfDimensions.x, distAlongI ) );
	closestPoint += distAlongI * obb.m_iBasisNormal;

	float distAlongJ = DotProduct3D( delta, obb.m_jBasisNormal );
	distAlongJ = MAX( -obb.m_halfDimensions.y, MIN( obb.m_halfDimensions.y, distAlongJ ) );
	closestPoint += distAlongJ * obb.m_jBasisNormal;

	float distAlongK = DotProduct3D( delta, obb.m_kBasisNormal );
	distAlongK = MAX( -obb.m_halfDimensions.z, MIN( obb.m_halfDimensions.z, distAlongK ) );
	closestPoint += distAlongK * obb.m_kBasisNormal;

	return closestPoint;
}

Mat44 const GetLookAtMatrix( Vec3 forwardVec )
{
	Vec3 iBasis = forwardVec.GetNormalized();
	Vec3 jBasis = Vec3( 0.f, 1.f, 0.f );
	Vec3 kBasis = Vec3( 0.f, 0.f, 1.f );
	if (iBasis != Vec3( 0.f, 0.f, 1.f ))
	{
		jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), iBasis ).GetNormalized();
		kBasis = CrossProduct3D( iBasis, jBasis );
	}
	return Mat44( iBasis, jBasis, kBasis, Vec3::ZERO );
}

Mat44 const CreateLookRotation( Vec3 const& forward, Vec3 const& up ) 
{
	Vec3 x = forward.GetNormalized();               // X: Forward
	Vec3 z = up.GetNormalized();                    // Z: Up
	Vec3 y = CrossProduct3D( z, x ).GetNormalized();    // Y: Left

	Mat44 m;

	m.SetIJK3D( x, y, z );  // X-axis = Forward
							// Y-axis = Left
							// Z-axis = Up

	return m;
}

Mat44 const GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale )
{
	Mat44 transformMat;
	if (billboardType == BillboardType::FULL_FACING)
	{
		Vec3 cameraPos = cameraMatrix.GetTranslation3D();
		Vec3 textToCameraVec = cameraPos - billboardPosition;
		transformMat = GetLookAtMatrix( textToCameraVec );
	}
	else if (billboardType == BillboardType::FULL_OPPOSING)
	{
		transformMat.SetIJK3D( -cameraMatrix.GetIBasis3D(), -cameraMatrix.GetJBasis3D(), cameraMatrix.GetKBasis3D() );
	}
	else if (billboardType == BillboardType::WORLD_UP_FACING)
	{
		Vec3 cameraPos = cameraMatrix.GetTranslation3D();
		Vec3 textToCameraVec = cameraPos - billboardPosition;
		transformMat = GetLookAtMatrix( textToCameraVec );
		transformMat.SetIJK3D( transformMat.GetIBasis3D(), transformMat.GetJBasis3D(), Vec3( 0.f, 0.f, 1.f ) );
	}
	else if (billboardType == BillboardType::WORLD_UP_OPPOSING)
	{
		transformMat.SetIJK3D( -cameraMatrix.GetIBasis3D(), -cameraMatrix.GetJBasis3D(), Vec3( 0.f, 0.f, 1.f ) );
	}
	else if (billboardType == BillboardType::NONE)
	{

	}
	transformMat.SetTranslation3D( billboardPosition );
	transformMat.AppendScaleNonUniform2D( billboardScale );
	return transformMat;
}

bool PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedpoint )
{
	Vec2 pointToCenterVector = mobileDiscCenter - fixedpoint;
	float pointToCenterLength = pointToCenterVector.GetLength();
	if (pointToCenterLength <= discRadius)
	{
		Vec2 pointToCenterNormal = pointToCenterVector.GetNormalized();
		mobileDiscCenter += pointToCenterNormal * (discRadius - pointToCenterLength);
		return true;
	}
	return false;
}

bool PushDiscOutOfFixedDisc2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius )
{
	Vec2 fixedToMobileVector = mobileDiscCenter - fixedDiscCenter;
	float fixedToMobileLength = fixedToMobileVector.GetLength();
	if (fixedToMobileLength <= mobileDiscRadius + fixedDiscRadius)
	{
		Vec2 pointToCenterNormal = fixedToMobileVector.GetNormalized();
		mobileDiscCenter += pointToCenterNormal * (mobileDiscRadius + fixedDiscRadius - fixedToMobileLength);
		return true;
	}
	return false;
}

bool PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius )
{
	Vec2 fixedToMobileVector = aCenter - bCenter;
	float fixedToMobileLength = fixedToMobileVector.GetLength();
	if (fixedToMobileLength <= aRadius + bRadius)
	{
		Vec2 pointToCenterNormal = fixedToMobileVector.GetNormalized();
		float moveDistTotal = aRadius + bRadius - fixedToMobileLength;
		aCenter += pointToCenterNormal * (moveDistTotal * 0.5f);
		bCenter += -pointToCenterNormal * (moveDistTotal * 0.5f);
		return true;
	}
	return false;
}

bool PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox )
{
	Vec2 centerToNearestPointOnAABBVector = mobileDiscCenter - fixedBox.GetNearestPoint( mobileDiscCenter );
	float centerToNearestPointOnAABBLength = centerToNearestPointOnAABBVector.GetLength();
	if (centerToNearestPointOnAABBLength <= discRadius)
	{
		Vec2 centerToNearestPointOnAABBNormal = centerToNearestPointOnAABBVector.GetNormalized();
		mobileDiscCenter += centerToNearestPointOnAABBNormal * discRadius - centerToNearestPointOnAABBVector;
		return true;
	}
	return false;
}

bool PushOBBsOutOfEachOther3D( OBB3& obbA, OBB3& obbB )
{
	Vec3 mtv;
	if (DoOBBsOverlap3D( obbA, obbB, mtv ))
	{
		obbA.m_center -= mtv * 0.5f;
		obbB.m_center += mtv * 0.5f;
		return true;
	}
	return false;
}

bool PushOBBsOutOfFixedOBBs3D( OBB3& mobileOBB3, OBB3 const& FixedOBB3 )
{
	Vec3 mtv;
	if (DoOBBsOverlap3D( mobileOBB3, FixedOBB3, mtv ))
	{
		mobileOBB3.m_center -= mtv * 0.5f;
		return true;
	}
	return false;
}

bool BounceDiscsOffPoint( Vec2& center, float radius, Vec2& velocity, Vec2 const& point, float elasticity )
{
	if (!PushDiscOutOfFixedPoint2D( center, radius, point ))
	{
		return false;
	}
	Vec2 discToPointVec = (point - center).GetNormalized();
	Vec2 Vn = DotProduct2D( velocity, discToPointVec ) * discToPointVec;
	Vec2 Vt = velocity - Vn;
	velocity = Vt - Vn * elasticity;
	return true;
}

bool BounceDiscsOffEachOther( Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticity )
{
	if (!PushDiscsOutOfEachOther2D( centerA, radiusA, centerB, radiusB ))
	{
		return false;
	}
	
	Vec2 aToBVec = centerB - centerA;
	Vec2 aToBVecNormal = aToBVec.GetNormalized();
	float dotVnA = DotProduct2D( velocityA, aToBVecNormal );
	Vec2 VnA = dotVnA * aToBVecNormal;
	Vec2 VtA = velocityA - VnA;
	float dotVnB = DotProduct2D( velocityB, aToBVecNormal );
	Vec2 VnB = dotVnB * aToBVecNormal;
	Vec2 VtB = velocityB - VnB;

	if (dotVnB < dotVnA)
	{
		velocityA = VtA + VnB * elasticity;
		velocityB = VtB + VnA * elasticity;
		return true;
	}

	return false;
}

bool BounceDiscOffDisc( Vec2& centerA, float radiusA, Vec2& velocityA, Vec2 const& centerB, float radiusB, float elasticity )
{
	//Vec2 closestOnDisc = GetNearestPointOnDisc2D( centerA, centerB, radiusB );
	return BounceDiscsOffPoint( centerA, radiusA + radiusB, velocityA, centerB, elasticity );
}

bool BounceDiscOffCapsule( Vec2& center, float radius, Vec2& velocity, Vec2 const& boneStart, Vec2 const& boneEnd, float radiusCapsule, float elasticity )
{
	//Vec2 closestOnCapsule = GetNearestPointOnCapsule2D( center, boneStart, boneEnd, radiusCapsule );
	Vec2 closestOnCapsule = GetNearestPointOnLineSegment2D( center, boneStart, boneEnd );
	return BounceDiscsOffPoint( center, radius + radiusCapsule, velocity, closestOnCapsule, elasticity );
	//return BounceDiscsOffPoint( center, radius, velocity, closestOnCapsule, elasticity );
}

bool BounceDiscOffOBB( Vec2& center, float radius, Vec2& velocity, OBB2 const& box, float elasticity )
{
	Vec2 closestOnOBB = GetNearestPointOnOBB2D( center, box );
	return BounceDiscsOffPoint( center, radius, velocity, closestOnOBB, elasticity );
}

void TransformPosition2D( Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation )
{
	if (uniformScale != 1.f)
	{
		posToTransform *= uniformScale;
	}
	if (rotationDegrees != 0.f)
	{
		posToTransform.RotateDegrees( rotationDegrees );
;	}
	if (translation != Vec2( 0.f, 0.f ))
	{
		posToTransform += translation;
	}
}

void TransformPositionXY3D( Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY )
{
	if (scaleXY != 1.f)
	{
		posToTransform = Vec3( posToTransform.x * scaleXY, posToTransform.y * scaleXY, posToTransform.z );
	}
	if (zRotationDegrees != 0.f)
	{
		posToTransform = posToTransform.GetRotatedAboutZDegrees( zRotationDegrees );
	}
	if (translationXY != Vec2( 0.f, 0.f ))
	{
		posToTransform += Vec3( translationXY.x, translationXY.y, 0.f );
	}
}

void TransformPosition2D( Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation )
{
	posToTransform = Vec2(
		posToTransform.x * iBasis.x + posToTransform.y * jBasis.x + translation.x,
		posToTransform.x * iBasis.y + posToTransform.y * jBasis.y + translation.y
	);
}

void TransformPositionXY3D( Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{
	posToTransform = Vec3(
		posToTransform.x * iBasis.x + posToTransform.y * jBasis.x + translationXY.x,
		posToTransform.x * iBasis.y + posToTransform.y * jBasis.y + translationXY.y,
		posToTransform.z
	);
}

float DotProduct2D( Vec2 const& a, Vec2 const& b )
{
	return a.x * b.x + a.y * b.y;
}

float DotProduct3D( Vec3 const& a, Vec3 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotProduct4D( Vec4 const& a, Vec4 const& b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float CrossProduct2D( Vec2 const& a, Vec2 const& b )
{
	return a.x * b.y - a.y * b.x;
}

Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b)
{
	return Vec3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

float Clamp( float value, float minValue, float maxValue )
{
	if (value < minValue)
		return minValue;
	else if (value > maxValue)
		return maxValue;
	return value;
}

int ClampInt( int value, int minValue, int maxValue )
{
	if (value < minValue)
		return minValue;
	else if (value > maxValue)
		return maxValue;
	return value;
}

float ClampZeroToOne( float value )
{
	return Clamp( value, 0.f, 1.f );
}

float Interpolate( float start, float end, float fractionTowardEnd )
{
	return (end - start) * fractionTowardEnd + start;
}

Vec2 Interpolate( Vec2 start, Vec2 end, float fractionTowardEnd )
{
	return (end - start) * fractionTowardEnd + start;
}

Vec3 Interpolate( Vec3 start, Vec3 end, float fractionTowardEnd )
{
	return (end - start) * fractionTowardEnd + start;
}

float GetFractionWithinRange( float value, float rangeStart, float rangeEnd )
{
	return (value - rangeStart) / (rangeEnd - rangeStart);
}

float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	return Interpolate( outStart, outEnd, GetFractionWithinRange( inValue, inStart, inEnd ) );
}

float RangeMapZeroToOne( float inValue, float inStart, float inEnd )
{
	return Interpolate( 0.f, 1.0f, GetFractionWithinRange( inValue, inStart, inEnd ) );
}

float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	return Interpolate( outStart, outEnd, GetFractionWithinRange( Clamp( inValue, inStart, inEnd ), inStart, inEnd ) );
}

int RoundDownToInt( float value )
{
	return (int)floorf( value );
}

float NormalizeByte( unsigned char targetByte )
{
	float rawFloat = static_cast<float>(targetByte);
	return RangeMap( rawFloat, 0.f, 255.f, 0.f, 1.f );
}

unsigned char DenormalizeByte( float targetFloat )
{
	return static_cast<unsigned char>(Clamp( targetFloat * 256.f, 0.f, 255.f ));
}

float ComputeCubicBezier1D( float A, float B, float C, float D, float t )
{
	float AB = Interpolate( A, B, t );
	float BC = Interpolate( B, C, t );
	float CD = Interpolate( C, D, t );

	float ABC = Interpolate( AB, BC, t );
	float BCD = Interpolate( BC, CD, t );

	float ABCD = Interpolate( ABC, BCD, t );
	
	return ABCD;
}

float ComputeQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t )
{
	float AB = Interpolate( A, B, t );
	float BC = Interpolate( B, C, t );
	float CD = Interpolate( C, D, t );
	float DE = Interpolate( D, E, t );
	float EF = Interpolate( E, F, t );

	float ABC = Interpolate( AB, BC, t );
	float BCD = Interpolate( BC, CD, t );
	float CDE = Interpolate( CD, DE, t );
	float DEF = Interpolate( DE, EF, t );

	float ABCD = Interpolate( ABC, BCD, t );
	float BCDE = Interpolate( BCD, CDE, t );
	float CDEF = Interpolate( CDE, DEF, t );

	float ABCDE = Interpolate( ABCD, BCDE, t );
	float BCDEF = Interpolate( BCDE, CDEF, t );

	float ABCDEF = Interpolate( ABCDE, BCDEF, t );

	return ABCDEF;
}

float SmoothStart2( float t )
{
	return t * t;
}

float SmoothStart3( float t )
{
	return t * t * t;
}

float SmoothStart4( float t )
{
	return t * t * t * t;
}

float SmoothStart5( float t )
{
	return t * t * t * t * t;
}

float SmoothStart6( float t )
{
	return t * t * t * t * t * t;
}

float SmoothStop2( float t )
{
	return 1.f - (1.f - t) * (1.f - t);
}

float SmoothStop3( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStop4( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStop5( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStop6( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStep3( float t )
{
	return Interpolate( SmoothStart3( t ), SmoothStop3( t ), t );
}

float SmoothStep5( float t )
{
	return Interpolate( SmoothStart5( t ), SmoothStop5( t ), t );
}

float Hesitate3( float t )
{
	return ComputeCubicBezier1D( 0.f, 0.f, 1.f, 1.f, t );
}

float Hesitate5( float t )
{
	return (1.f - t) * SmoothStop4( t ) + SmoothStart4( t );
	//return ComputeQuinticBezier1D( 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t );
}

float CustomFunkyEasingFunction( float t )
{
	return Hesitate5( t ) * Hesitate5( t );
}

void xyzToZyx( float x, float y, float z, float& zyx_z, float& zyx_y, float& zyx_x ) 
{
	float sy = SinDegrees( y );
	float cy = CosDegrees( y );
	float sx = SinDegrees( x );
	float cx = CosDegrees( x );
	float sz = SinDegrees( z );
	float cz = CosDegrees( z );

	zyx_z = Atan2Degrees( cz * sy * cx + sz * sx, cz * cy );
	zyx_y = ASinDegrees( -cz * sy * sx + sz * cx );
	zyx_x = Atan2Degrees( sz * sy * cx - cz * sx, sz * sy * sx + cz * cx );
}