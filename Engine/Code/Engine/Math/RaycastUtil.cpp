#include "Engine/Math/RaycastUtil.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

bool RaycastDisc2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius )
{
	Vec2 iBasis = fwdNormal;
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	Vec2 rayVector = discCenter - startPos;
	float rayVectorInJ = DotProduct2D( rayVector, jBasis );

	if (rayVectorInJ >= discRadius)
		return false;
	if (rayVectorInJ <= -discRadius)
		return false;

	if (IsPointInsideDisc2D( startPos, discCenter, discRadius ))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		result.m_impactDist = 0.f;
		return true;
	}
	float rayVectorInI = DotProduct2D( rayVector, iBasis );

	float a = sqrtf( discRadius * discRadius - rayVectorInJ * rayVectorInJ );
	float impactDist = rayVectorInI - a;
	
	if (impactDist >= maxDist)
		return false;
	if (impactDist <= -2.f * a)
		return false;

	result.m_impactDist = impactDist;
	result.m_impactPos = startPos + impactDist * iBasis;
	result.m_impactNormal = (result.m_impactPos - discCenter).GetNormalized();
	result.m_didImpact = true;
	return true;
}

bool RaycastSegment2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 segmentStartPos, Vec2 segmentEndPos )
{
	Vec2 iBasis = fwdNormal;
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	Vec2 S = startPos;
	float M = maxDist;
	Vec2 P = segmentStartPos;
	Vec2 Q = segmentEndPos;
	Vec2 SP = P - S;
	Vec2 SQ = Q - S;
	float SPJ = DotProduct2D( SP, jBasis );
	float SQJ = DotProduct2D( SQ, jBasis );

	if (SPJ * SQJ >= 0)
		return false;

	float t = SPJ / (SPJ - SQJ);
	Vec2 I = P + t * (Q - P);
	Vec2 SI = I - S;
	float impactDist = DotProduct2D( SI, iBasis );

	if (impactDist >= M)
		return false;
	if (impactDist <= 0)
		return false;

	Vec2 PQ = Q - P;
	Vec2 N = PQ.GetRotated90Degrees();
	if (SQJ < 0)
	{
		N *= -1.f;
	}
	N.Normalize();

	result.m_impactDist = impactDist;
	result.m_impactPos = startPos + impactDist * iBasis;
	result.m_impactNormal = N;
	result.m_didImpact = true;
	return true;
}

bool RaycastAABB2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 aabb )
{
	Vec2 endPos = startPos + fwdNormal * maxDist;
	Vec2 rayAABBMin;
	Vec2 rayAABBMax;
	rayAABBMin.x = MIN( startPos.x, endPos.x );
	rayAABBMin.y = MIN( startPos.y, endPos.y );
	rayAABBMax.x = MAX( startPos.x, endPos.x );
	rayAABBMax.y = MAX( startPos.y, endPos.y );
	AABB2 rayAABB( rayAABBMin, rayAABBMax );
	
	if (!DoAABBsOverlap2D( aabb, rayAABB ))
		return false;

	if (IsPointInsideAABB2D( startPos, aabb ))
	{
		result.m_impactDist = maxDist;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		result.m_didImpact = true;
		return true;
	}

	bool isXInverse = false;
	bool isYInverse = false;
	bool isIntersectX = false;
	bool isIntersectY = false;

	float tMinX = (aabb.m_mins.x - startPos.x) / (endPos.x - startPos.x);
	float tMaxX = (aabb.m_maxs.x - startPos.x) / (endPos.x - startPos.x);
	if (tMinX > tMaxX)
	{
		std::swap( tMinX, tMaxX );
		isXInverse = true;
	}
	float tMinY = (aabb.m_mins.y - startPos.y) / (endPos.y - startPos.y);
	float tMaxY = (aabb.m_maxs.y - startPos.y) / (endPos.y - startPos.y);
	if (tMinY > tMaxY)
	{
		std::swap( tMinY, tMaxY );
		isYInverse = true;
	}

	FloatRange tOverlapX = FloatRange( tMinX, tMaxX );
	FloatRange tOverlapY = FloatRange( tMinY, tMaxY );
	FloatRange tOverlapResult;
	if (DoFloatRangeOverlap( tOverlapX, tOverlapY ))
	{
		if (tOverlapX.x < tOverlapY.x)
		{
			tOverlapResult.x = tOverlapY.x;
			isIntersectY = true;
		}
		else
		{
			tOverlapResult.x = tOverlapX.x;
			isIntersectX = true;
		}
		if (tOverlapX.y > tOverlapY.y)
		{
			tOverlapResult.y = tOverlapY.y;
		}
		else
		{
			tOverlapResult.y = tOverlapX.y;
		}
		result.m_impactDist = tOverlapResult.x * maxDist;
		result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
		if (isIntersectY)
		{
			if (isYInverse)
			{
				result.m_impactNormal = Vec2( aabb.m_maxs.x - aabb.m_mins.x, 0.f ).GetNormalized().GetRotated90Degrees();
			}
			else
			{
				result.m_impactNormal = Vec2( aabb.m_maxs.x - aabb.m_mins.x, 0.f ).GetNormalized().GetRotatedMinus90Degrees();
			}
		}
		if (isIntersectX)
		{
			if (isXInverse)
			{
				result.m_impactNormal = Vec2( aabb.m_maxs.x - aabb.m_mins.x, 0.f ).GetNormalized();
			}
			else
			{
				result.m_impactNormal = Vec2( aabb.m_mins.x - aabb.m_maxs.x, 0.f ).GetNormalized();
			}
		}
		result.m_didImpact = true;
		return true;
	}
	else
	{
		return false;
	}
}

bool RaycastConvex2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2 convexHull2 )
{
	Vec2 lastInwardIntersection;
	Vec2 firstOutwardIntersection;
	float maxInwardIntersectDist = 0.f;
	float minOutwardIntersectDist = 99999.f;
	int maxInwardIndex = 0;
	bool hasAnyIntersection = false;
	bool hasAnyInwardIntersection = false;
	bool hasAnyOutwardIntersection = false;

	for (int i = 0; i < convexHull2.m_boundingPlanes.size(); i++)
	{
		Plane2 const& plane = convexHull2.m_boundingPlanes[i];
		Vec2 intersection;
		float intersectDist = 0.f;
		if (plane.GetIntersection( intersection, startPos, fwdNormal, maxDist, intersectDist ))
		{
			hasAnyIntersection = true;
			if (plane.IsRayInward( fwdNormal ))
			{
				hasAnyInwardIntersection = true;
				if (intersectDist > maxInwardIntersectDist)
				{
					maxInwardIntersectDist = intersectDist;
					lastInwardIntersection = intersection;
					maxInwardIndex = i;
				}
			}
			else
			{
				hasAnyOutwardIntersection = true;
				if (intersectDist < minOutwardIntersectDist)
				{
					minOutwardIntersectDist = intersectDist;
					firstOutwardIntersection = intersection;
				}
			}
		}
	}

	if (!hasAnyIntersection)
	{
		result.m_didImpact = false;
		return false;
	}

	if (convexHull2.IsPointInside( startPos ))
	{
		result.m_impactPos = startPos;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		result.m_didImpact = true;
		return true;
	}

	if (!hasAnyOutwardIntersection)
	{
		firstOutwardIntersection = startPos + fwdNormal * maxDist;
	}
	if (!hasAnyInwardIntersection)
	{
		result.m_didImpact = false;
		return false;
	}
	Vec2 midPoint = (lastInwardIntersection + firstOutwardIntersection) / 2.f;
	if (!convexHull2.IsPointInside( midPoint ))
	{
		result.m_didImpact = false;
		return false;
	}

	result.m_impactPos = lastInwardIntersection;
	result.m_impactDist = maxInwardIntersectDist;
	result.m_impactNormal = convexHull2.m_boundingPlanes[maxInwardIndex].m_normal;
	result.m_didImpact = true;
	return true;
}

bool RaycastGridFirst2D( RaycastResult2D& result, Vec2 startPos, Vec2 fwdNormal, float maxDist, void const* gridSolidity, int gridLength, int gridWidth )
{
	std::vector<bool> solidity = *reinterpret_cast<std::vector<bool> const*>(gridSolidity);
	IntVec2 startTileCoord( (int)floorf( startPos.x ), (int)floorf( startPos.y ) );
	int startTileIndex = startTileCoord.y * gridLength + startTileCoord.x;

	if (solidity[startTileIndex] == true)
	{
		if (startTileCoord.x > 0 && startTileCoord.x < gridLength &&
			startTileCoord.y > 0 && startTileCoord.y < gridWidth)
		{
			result.m_didImpact = true;
			result.m_impactDist = 0.f;
			result.m_impactNormal = -fwdNormal;
			result.m_impactPos = startPos;
			return true;
		}
	}
			
	//how far forward between each X crossing
	float fwdDistPerXCrossing = 1.f / abs( fwdNormal.x );

	//one tile step forward horizontally (on x)
	int tileStepDirectionX = (fwdNormal.x < 0) ? -1 : 1;

	//x coordinate of the first x-crossing. Essentially rounding up (east) or down (west).
	float xAtFirstXCrossing = (float)(startTileCoord.x + (tileStepDirectionX + 1) / 2);

	//How far on x we must go until the first x-crossing
	float xDistToFirstCrossing = xAtFirstXCrossing - startPos.x;

	//How far forward we must go until the first x-crossing
	float fwdDistAtNextXCrossing = fabsf( xDistToFirstCrossing ) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = 1.f / abs( fwdNormal.y );
	int tileStepDirectionY = (fwdNormal.y < 0) ? -1 : 1;
	float yAtFirstYCrossing = (float)(startTileCoord.y + (tileStepDirectionY + 1) / 2);
	float yDistToFirstCrossing = yAtFirstYCrossing - startPos.y;
	float fwdDistAtNextYCrossing = fabsf( yDistToFirstCrossing ) * fwdDistPerYCrossing;

	IntVec2 currentTileCoord = startTileCoord;
	int currentTileIndex = startTileIndex;
	for (;;)
	{
		if (fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextXCrossing > maxDist)
			{
				result.m_didImpact = false;
				return false;
			}

			currentTileCoord.x += tileStepDirectionX;

			if ((float)currentTileCoord.x > gridLength ||
				(float)currentTileCoord.x < 0)
			{
				result.m_didImpact = false;
				return false;
			}

			currentTileIndex = currentTileCoord.y * gridLength + currentTileCoord.x;

			if (solidity[currentTileIndex] == true)
			{
				result.m_didImpact = true;
				result.m_impactDist = fwdDistAtNextXCrossing;
				result.m_impactNormal = Vec2( (fwdNormal.x > 0) ? -1.f : 1.f, 0.f );
				result.m_impactPos = startPos + fwdNormal * fwdDistAtNextXCrossing;
				return true;
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else if (fwdDistAtNextYCrossing < fwdDistAtNextXCrossing)
		{
			if (fwdDistAtNextYCrossing > maxDist)
			{
				result.m_didImpact = false;
				return false;
			}

			currentTileCoord.y += tileStepDirectionY;

			if ((float)currentTileCoord.y > gridWidth ||
				(float)currentTileCoord.y < 0)
			{
				result.m_didImpact = false;
				return false;
			}

			currentTileIndex += tileStepDirectionY * gridWidth;

			if (solidity[currentTileIndex] == true)
			{
				result.m_didImpact = true;
				result.m_impactDist = fwdDistAtNextYCrossing;
				result.m_impactNormal = Vec2( 0.f, (fwdNormal.y > 0) ? -1.f : 1.f );
				result.m_impactPos = startPos + fwdNormal * fwdDistAtNextYCrossing;
				return true;
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
}

bool RaycastGridAll2D( std::vector<RaycastResult2D>& results, Vec2 startPos, Vec2 fwdNormal, float maxDist, void const* gridSolidity, int gridLength, int gridWidth )
{
	std::vector<bool> solidity = *reinterpret_cast<std::vector<bool> const*>(gridSolidity);
	IntVec2 startTileCoord( (int)floorf( startPos.x ), (int)floorf( startPos.y ) );
	int startTileIndex = startTileCoord.y * gridLength + startTileCoord.x;

	if (startTileCoord.x > 0 && startTileCoord.x < gridLength &&
		startTileCoord.y > 0 && startTileCoord.y < gridWidth)
	{
		if (solidity[startTileIndex] == true)
		{
			RaycastResult2D result;
			result.m_didImpact = true;
			result.m_impactDist = 0.f;
			result.m_impactNormal = -fwdNormal;
			result.m_impactPos = startPos;
			results.push_back( result );
		}
	}

	//how far forward between each X crossing
	float fwdDistPerXCrossing = 1.f / abs( fwdNormal.x );

	//one tile step forward horizontally (on x)
	int tileStepDirectionX = (fwdNormal.x < 0) ? -1 : 1;

	//x coordinate of the first x-crossing. Essentially rounding up (east) or down (west).
	float xAtFirstXCrossing = (float)(startTileCoord.x + (tileStepDirectionX + 1) / 2);

	//How far on x we must go until the first x-crossing
	float xDistToFirstCrossing = xAtFirstXCrossing - startPos.x;

	//How far forward we must go until the first x-crossing
	float fwdDistAtNextXCrossing = fabsf( xDistToFirstCrossing ) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = 1.f / abs( fwdNormal.y );
	int tileStepDirectionY = (fwdNormal.y < 0) ? -1 : 1;
	float yAtFirstYCrossing = (float)(startTileCoord.y + (tileStepDirectionY + 1) / 2);
	float yDistToFirstCrossing = yAtFirstYCrossing - startPos.y;
	float fwdDistAtNextYCrossing = fabsf( yDistToFirstCrossing ) * fwdDistPerYCrossing;

	IntVec2 currentTileCoord = startTileCoord;
	int currentTileIndex = startTileIndex;
	for (;;)
	{
		if (fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextXCrossing > maxDist)
			{
				return false;
			}

			currentTileCoord.x += tileStepDirectionX;

			if ((float)currentTileCoord.x >= gridLength ||
				(float)currentTileCoord.x < 0 ||
				(float)currentTileCoord.y >= gridWidth ||
				(float)currentTileCoord.y < 0)
			{
				return false;
			}

			currentTileIndex = currentTileCoord.y * gridLength + currentTileCoord.x;

			if (solidity[currentTileIndex] == true)
			{
				RaycastResult2D result;
				result.m_didImpact = true;
				result.m_impactDist = fwdDistAtNextXCrossing;
				result.m_impactNormal = Vec2( (fwdNormal.x > 0) ? -1.f : 1.f, 0.f );
				result.m_impactPos = startPos + fwdNormal * fwdDistAtNextXCrossing;
				results.push_back( result );
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else if (fwdDistAtNextYCrossing < fwdDistAtNextXCrossing)
		{
			if (fwdDistAtNextYCrossing > maxDist)
			{
				return false;
			}

			currentTileCoord.y += tileStepDirectionY;

			if ((float)currentTileCoord.x >= gridLength ||
				(float)currentTileCoord.x < 0 ||
				(float)currentTileCoord.y >= gridWidth ||
				(float)currentTileCoord.y < 0)
			{
				return false;
			}

			currentTileIndex += tileStepDirectionY * gridWidth;

			if (solidity[currentTileIndex] == true)
			{
				RaycastResult2D result;
				result.m_didImpact = true;
				result.m_impactDist = fwdDistAtNextYCrossing;
				result.m_impactNormal = Vec2( 0.f, (fwdNormal.y > 0) ? -1.f : 1.f );
				result.m_impactPos = startPos + fwdNormal * fwdDistAtNextYCrossing;
				results.push_back( result );
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
}

bool RaycastZCylinder3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 cylinderStart, float cylinderHeight, float cylinderRadius )
{
	Vec3 endPos = startPos + fwdNormal * maxDist;
	AABB3 rayBound = AABB3( Vec3( MIN( startPos.x, endPos.x ), MIN( startPos.y, endPos.y ), MIN( startPos.z, endPos.z ) ), Vec3( MAX( startPos.x, endPos.x ), MAX( startPos.y, endPos.y ), MAX( startPos.z, endPos.z ) ) );
	AABB3 cylinderBound( Vec3( cylinderStart.x - cylinderRadius, cylinderStart.y - cylinderRadius, cylinderStart.z ),
		Vec3( cylinderStart.x + cylinderRadius, cylinderStart.y + cylinderRadius, cylinderStart.z + cylinderHeight ) );
	if (!DoAABBsOverlap3D( rayBound, cylinderBound ))
	{
		result.m_didImpact = false;
		return false;
	}

	Vec2 startPos2D = startPos.GetXY();
	Vec2 fwdNormal2D = fwdNormal.GetXY().GetNormalized();
	float maxDist2D = (fwdNormal * maxDist).GetXY().GetLength();

	Vec2 discPos = cylinderStart.GetXY();

	RaycastResult2D discResult;
	if (!RaycastDisc2D( discResult, startPos2D, fwdNormal2D, maxDist2D, discPos, cylinderRadius ))
	{
		result.m_didImpact = false;
		return false;
	}
	float fric = discResult.m_impactDist / maxDist2D;
	float impactZ3D = startPos.z + fric * fwdNormal.z * maxDist;

	RaycastResult3D cylinderResult;
	if (impactZ3D > cylinderStart.z && impactZ3D < cylinderStart.z + cylinderHeight)
	{
		result.m_didImpact = true;

		if (discResult.m_impactPos == startPos2D)
		{
			result.m_impactDist = 0.f;
			result.m_impactNormal = -fwdNormal;
			result.m_impactPos = startPos;
			return true;
		}
		else
		{
			result.m_impactDist = fric * maxDist;
			result.m_impactNormal = Vec3( discResult.m_impactNormal.x, discResult.m_impactNormal.y, 0.f );
			result.m_impactPos = startPos + result.m_impactDist * fwdNormal;
			return true;
		}
	}
	else
	{
		RaycastResult3D topResult, bottomResult;
		RaycastZPlane3D( topResult, startPos, fwdNormal, maxDist, cylinderStart.z + cylinderHeight );
		RaycastZPlane3D( bottomResult, startPos, fwdNormal, maxDist, cylinderStart.z );
		result.m_impactDist = 9999.f;
		if (IsPointInsideDisc2D( topResult.m_impactPos.GetXY(), discPos, cylinderRadius ))
		{
			if (topResult.m_impactDist < result.m_impactDist)
			{
				result = topResult;
			}
		}
		if (IsPointInsideDisc2D( bottomResult.m_impactPos.GetXY(), discPos, cylinderRadius ))
		{
			if (bottomResult.m_impactDist < result.m_impactDist)
			{
				result = bottomResult;
			}
		}
		if (result.m_impactDist != 9999.f)
		{
			return true;
		}
	}
	return false;
}

bool RaycastZPlane3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, float ZValue )
{
	Vec3 endPos = startPos + fwdNormal * maxDist;
	FloatRange ZRange( MIN( startPos.z, endPos.z ), MAX( startPos.z, endPos.z ) );
	if (!(ZValue > ZRange.x && ZValue < ZRange.y))
	{
		result.m_didImpact = false;
		return false;
	}

	float zDist = ZValue - startPos.z;
	float zPerUnit = DotProduct3D( fwdNormal, Vec3( 0.f, 0.f, 1.f ) );
	float impactDist = zDist / zPerUnit;

	result.m_didImpact = true;
	result.m_impactDist = impactDist;
	result.m_impactNormal = Vec3( 0.f, 0.f, (fwdNormal.z > 0) ? -1.f : 1.f );
	result.m_impactPos = startPos + fwdNormal * impactDist;
	return true;
}

bool RaycastAABB3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, AABB3 aabb )
{
	Vec3 endPos = startPos + fwdNormal * maxDist;
	AABB3 rayBound = AABB3( Vec3( MIN( startPos.x, endPos.x ), MIN( startPos.y, endPos.y ), MIN( startPos.z, endPos.z ) ), Vec3( MAX( startPos.x, endPos.x ), MAX( startPos.y, endPos.y ), MAX( startPos.z, endPos.z ) ) );
	if (!DoAABBsOverlap3D( rayBound, aabb ))
	{
		result.m_didImpact = false;
		return false;
	}

	if (IsPointInsideAABB3D( startPos, aabb ))
	{
		result.m_impactDist = 0.f;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		result.m_didImpact = true;
		return true;
	}

	bool isXInverse = false;
	bool isYInverse = false;
	bool isZInverse = false;
	bool isIntersectX = false;
	bool isIntersectY = false;
	bool isIntersectZ = false;

	float tMinX = (aabb.m_mins.x - startPos.x) / (endPos.x - startPos.x);
	float tMaxX = (aabb.m_maxs.x - startPos.x) / (endPos.x - startPos.x);
	if (tMinX > tMaxX)
	{
		std::swap( tMinX, tMaxX );
		isXInverse = true;
	}
	float tMinY = (aabb.m_mins.y - startPos.y) / (endPos.y - startPos.y);
	float tMaxY = (aabb.m_maxs.y - startPos.y) / (endPos.y - startPos.y);
	if (tMinY > tMaxY)
	{
		std::swap( tMinY, tMaxY );
		isYInverse = true;
	}
	FloatRange tOverlapX = FloatRange( tMinX, tMaxX );
	FloatRange tOverlapY = FloatRange( tMinY, tMaxY );
	FloatRange tOverlapResultXY;
	if (!DoFloatRangeOverlap( tOverlapX, tOverlapY ))
	{
		result.m_didImpact = false;
		return false;
	}
	if (tOverlapX.x < tOverlapY.x)
	{
		tOverlapResultXY.x = tOverlapY.x;
		isIntersectX = false;
		isIntersectY = true;
		isIntersectZ = false;
	}
	else
	{
		tOverlapResultXY.x = tOverlapX.x;
		isIntersectX = true;
		isIntersectY = false;
		isIntersectZ = false;
	}
	if (tOverlapX.y > tOverlapY.y)
	{
		tOverlapResultXY.y = tOverlapY.y;
	}
	else
	{
		tOverlapResultXY.y = tOverlapX.y;
	}
	float tMinZ = (aabb.m_mins.z - startPos.z) / (endPos.z - startPos.z);
	float tMaxZ = (aabb.m_maxs.z - startPos.z) / (endPos.z - startPos.z);
	if (tMinZ > tMaxZ)
	{
		std::swap( tMinZ, tMaxZ );
		isZInverse = true;
	}
	FloatRange tOverlapZ = FloatRange( tMinZ, tMaxZ );
	FloatRange tOverlapXYZ;
	if (!DoFloatRangeOverlap(tOverlapResultXY, tOverlapZ))
	{
		result.m_didImpact = false;
		return false;
	}
	if (tOverlapZ.x < tOverlapResultXY.x)
	{
		tOverlapXYZ.x = tOverlapResultXY.x;
		isIntersectZ = false;
	}
	else
	{
		tOverlapXYZ.x = tOverlapZ.x;
		isIntersectX = false;
		isIntersectY = false;
		isIntersectZ = true;
	}
	if (tOverlapResultXY.y > tOverlapZ.y)
	{
		tOverlapXYZ.y = tOverlapZ.y;
	}
	else
	{
		tOverlapXYZ.y = tOverlapResultXY.y;
	}
	result.m_impactDist = tOverlapXYZ.x * maxDist;
	result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
	if (isIntersectX)
	{
		if (isXInverse)
		{
			result.m_impactNormal = Vec3( 1.f, 0.f, 0.f );
		}
		else
		{
			result.m_impactNormal = Vec3( -1.f, 0.f, 0.f );
		}
	}
	else if (isIntersectY)
	{
		if (isYInverse)
		{
			result.m_impactNormal = Vec3( 0.f, 1.f, 0.f );
		}
		else
		{
			result.m_impactNormal = Vec3( 0.f, -1.f, 0.f );
		}
	}
	else if (isIntersectZ)
	{
		if (isZInverse)
		{
			result.m_impactNormal = Vec3( 0.f, 0.f, 1.f );
		}
		else
		{
			result.m_impactNormal = Vec3( 0.f, 0.f, -1.f );
		}
	}
	result.m_didImpact = true;
	return true;
}
bool RaycastSphere3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 const& sphereCenter, float sphereRadius )
{
	if (IsPointInsideSphere3D( startPos, sphereCenter, sphereRadius ))
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		result.m_impactPos = startPos;
		return true;
	}

	Vec3 startToCenter = sphereCenter - startPos;
	float startToCenterI = DotProduct3D( startToCenter, fwdNormal );

	if (startToCenterI >= maxDist || startToCenterI <= 0)
	{
		result.m_didImpact = false;
		return false;
	}

	Vec3 startToCenterIVec = startToCenterI * fwdNormal;
	Vec3 startToCenterJK = startToCenter - startToCenterIVec;

	float startToCenterJKLengthSquared = startToCenterJK.GetLengthSquared();

	if (startToCenterJKLengthSquared >= sphereRadius * sphereRadius)
	{
		result.m_didImpact = false;
		return false;
	}

	float a = sqrtf( sphereRadius * sphereRadius - startToCenterJKLengthSquared );
	float impactDist = startToCenterI - a;

	if (impactDist >= maxDist)
	{
		result.m_didImpact = false;
		return false;
	}

	result.m_didImpact = true;
	result.m_impactDist = impactDist;
	result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
	result.m_impactNormal = (result.m_impactPos - sphereCenter).GetNormalized();
	return true;
}

bool RaycastOBB3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, OBB3 obb )
{
	Vec3 localStartPos = obb.WorldPosToLocalPos( startPos );
	Vec3 localFwdNormal = obb.WorldVecToLocalVec( fwdNormal );
	AABB3 localAABB = obb.GetLocalAABB3();
	RaycastAABB3D( result, localStartPos, localFwdNormal, maxDist, localAABB );
	result.m_impactPos = obb.LocalPosToWorldPos( result.m_impactPos );
	result.m_impactNormal = obb.LocalVecToWorldVec( result.m_impactNormal );
	return result.m_didImpact;
}

bool RaycastCapsule3D( RaycastResult3D& result, Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 const& boneStart, Vec3 const& boneEnd, float capsuleRadius )
{
	result.m_didImpact = false;
	if (capsuleRadius <= 0.f || maxDist < 0.f)
	{
		return false;
	}

	Vec3 closestOnBone = GetNearestPointOnLineSegment3D( startPos, boneStart, boneEnd );
	Vec3 toClosest = startPos - closestOnBone;
	if (toClosest.GetLengthSquared() <= capsuleRadius * capsuleRadius)
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = startPos;
		if (toClosest.GetLengthSquared() > 0.000001f)
		{
			result.m_impactNormal = toClosest.GetNormalized();
		}
		else
		{
			result.m_impactNormal = -fwdNormal;
		}
		return true;
	}

	float bestDist = maxDist;
	bool anyHit = false;
	RaycastResult3D temp;

	auto considerHit = [&]( RaycastResult3D const& hit )
	{
		if (!hit.m_didImpact)
		{
			return;
		}
		if (hit.m_impactDist < 0.f || hit.m_impactDist > bestDist)
		{
			return;
		}
		bestDist = hit.m_impactDist;
		result = hit;
		anyHit = true;
	};

	// Caps (hemispheres approximated as full spheres — finite cylinder rejects side overlaps past ends).
	if (RaycastSphere3D( temp, startPos, fwdNormal, maxDist, boneStart, capsuleRadius ))
	{
		considerHit( temp );
	}
	if (RaycastSphere3D( temp, startPos, fwdNormal, maxDist, boneEnd, capsuleRadius ))
	{
		considerHit( temp );
	}

	// Finite cylinder along the bone (previously missing — rays through the torso always missed).
	Vec3 axis = boneEnd - boneStart;
	float const axisLenSq = axis.GetLengthSquared();
	if (axisLenSq > 0.000001f)
	{
		float const axisLen = sqrtf( axisLenSq );
		Vec3 const ba = axis * (1.f / axisLen);
		Vec3 const d = fwdNormal;
		Vec3 const delta = startPos - boneStart;

		float const md = DotProduct3D( delta, ba );
		float const nd = DotProduct3D( d, ba );
		Vec3 const o = delta - ba * md;
		Vec3 const dPerp = d - ba * nd;

		float const a = DotProduct3D( dPerp, dPerp );
		float const b = 2.f * DotProduct3D( o, dPerp );
		float const c = DotProduct3D( o, o ) - capsuleRadius * capsuleRadius;

		if (a > 0.000001f)
		{
			float const discr = b * b - 4.f * a * c;
			if (discr >= 0.f)
			{
				float const sqrtDiscr = sqrtf( discr );
				float const invDenom = 0.5f / a;
				float const candidates[2] = { ( -b - sqrtDiscr ) * invDenom, ( -b + sqrtDiscr ) * invDenom };
				for (float t : candidates)
				{
					if (t < 0.f || t > maxDist)
					{
						continue;
					}
					Vec3 const hitPos = startPos + d * t;
					float const axisT = DotProduct3D( hitPos - boneStart, ba );
					if (axisT < 0.f || axisT > axisLen)
					{
						continue;
					}
					Vec3 const onAxis = boneStart + ba * axisT;
					Vec3 normal = hitPos - onAxis;
					if (normal.GetLengthSquared() <= 0.000001f)
					{
						continue;
					}
					temp.m_didImpact = true;
					temp.m_impactDist = t;
					temp.m_impactPos = hitPos;
					temp.m_impactNormal = normal.GetNormalized();
					considerHit( temp );
				}
			}
		}
	}

	return anyHit;
}
