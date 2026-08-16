#pragma once

//#define _USE_MATH_DEFINES
#include <math.h>

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Plane.hpp"

#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

enum class BillboardType
{
	NONE = -1,
	WORLD_UP_FACING,
	WORLD_UP_OPPOSING,
	FULL_FACING,
	FULL_OPPOSING,
	COUNT
};

constexpr float PI = 3.14159265358979323846f;

unsigned int RoundToPrevPower2( unsigned int base );
unsigned int RoundToNextPower2( unsigned int base );
float Power( float base, unsigned int pow );

float GetUnitDegree( float degrees );
float Clamp( float value, float minValue, float maxValue );
int ClampInt( int value, int minValue, int maxValue );
float ClampZeroToOne( float value );
float Interpolate( float start, float end, float fractionTowardEnd );
Vec2 Interpolate( Vec2 start, Vec2 end, float fractionTowardEnd );
Vec3 Interpolate( Vec3 start, Vec3 end, float fractionTowardEnd );
float GetFractionWithinRange( float value, float rangeStart, float rangeEnd );
float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd );
float RangeMapZeroToOne( float inValue, float inStart, float inEnd );
float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd );
int RoundDownToInt( float value );

float ConvertDegreesToRadians( float degrees );
float ConvertRadiansToDegrees( float radians );
float CosDegrees( float degrees );
float ACosDegrees( float value );
float SinDegrees( float degrees );
float ASinDegrees( float value );
float Atan2Degrees( float y, float x );
float GetShortestAngularDispDegrees( float startDegrees, float endDegrees );
float GetTurnedTowardDegrees( float currentDegrees, float goalDegrees, float maxDeltaDegrees );
float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b );

float GetDistance2D( Vec2 const& positionA, Vec2 const& positionB );
float GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB );
float GetDistance3D( Vec3 const& positionA, Vec3 const& positionB );
float GetDistanceSquared3D( Vec3 const& positionA, Vec3 const& positionB );
float GetDistanceXY3D( Vec3 const& positionA, Vec3 const& positionB );
float GetDistanceXYSquared3D( Vec3 const& positionA, Vec3 const& positionB );  
int GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB );
float GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& vecToProjectOnto );
Vec2 const GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vecToProjectOnto );
float GetProjectedLength3D( Vec3 const& vectorToProject, Vec3 const& vecToProjectOnto );
Vec3 const GetProjectedOnto3D( Vec3 const& vectorToProject, Vec3 const& vecToProjectOnto );

float GetLineSegmentsDistanceSquared3D( Vec3 const& startA, Vec3 const& endA, Vec3 const& startB, Vec3 const& endB );
float GetLineSegmentsDistanceSquared3D( Vec3 const& startA, Vec3 const& endA, Vec3 const& startB, Vec3 const& endB, Vec3& closestPointOnA, Vec3& closestPointOnB );

bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius );
bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& box );
bool IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius );
bool IsPointInsideOBB2D( Vec2 const& point, OBB2 const& box );
bool IsPointInsideOBB3D( Vec3 const& point, OBB3 const& box );
bool IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
bool IsPointInsideDirectedSector2D( Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius );
bool IsPointInsideSphere3D( Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius );
bool IsPointInsideAABB3D( Vec3 const& point, AABB3 const& box );
bool IsPointInsideCapsule3D( Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius );
bool IsPointInsideHexigon3D( Vec3 const& point, Vec3 const& center, float radius );
bool DoDiscsOverlap2D( Vec2 const& centerA, float const& radiusA, Vec2 const& centerB, float const& radiusB );
bool DoAABBsOverlap2D( AABB2 const& aabbA, AABB2 const& aabbB );
bool DoDiscOverlapAABB2D( AABB2 const& aabb, Vec2 const& center, float const& radius );
bool DoDiscOverlapOBB2D( Vec2 const& discCenter, float discRadius, OBB2 const& box );
bool DoDiscOverlapCapsule2D( Vec2 const& discCenter, float discRadius, Vec2 const& boneStart, Vec2 const& boneEnd, float radius );
bool DoDiscOverlapDirectedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
bool DoDiscOverlapOrientedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius );
bool DoSpheresOverlap3D( Vec3 const& centerA, float const& radiusA, Vec3 const& centerB, float const& radiusB );
bool DoSpheresOverlap3D( Vec3 const& centerA, float const& radiusA, Vec3 const& centerB, float const& radiusB, Vec3& out_mtv_XY );
bool DoAABBsOverlap3D( AABB3 const& aabbA, AABB3 const& aabbB );
bool DoZCylindersOverlap3D( Vec3 const& cylinderAStart, float cylinderAHeight, float cylinderARadius, Vec3 const& cylinderBStart, float cylinderBHeight, float cylinderBRadius );
bool DoSphereAndAABBOverlap3D( Vec3 const& center, float const& radius, AABB3 const& aabb );
bool DoZCylinderAndAABBOverlap3D( Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius, AABB3 const& aabb );
bool DoZCylinderAndSphereOverlap3D( Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius, Vec3 const& center, float const& radius );
bool DoPlaneOverlapZCylinder3D( Plane const& plane, Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius);
bool DoPlaneOverlapAABB3D( Plane const& plane, AABB3 const& aabb );
bool DoPlaneOverlapSphere3D( Plane const& plane, Vec3 const& center, float radius );
bool DoPlaneOverlapOBB3D( Plane const& plane, OBB3 const& obb );
bool DoOBBsOverlap3D( OBB3 const& obbA, OBB3 const& obbB );
bool DoOBBsOverlap3D( OBB3 const& obbA, OBB3 const& obbB, Vec3& out_mtv );
bool DoCapsuleOverlapOBB3D( Vec3 const& boneStart, Vec3 const& boneEnd, float radius, OBB3 const& obb );
bool DoCapsuleOverlapOBB3D( Vec3 const& boneStart, Vec3 const& boneEnd, float radius, OBB3 const& obb, Vec3& out_mtv );
bool DoCapsulesOverlap3D( Vec3 const& boneStartA, Vec3 const& boneEndA, float radiusA, Vec3 const& boneStartB, Vec3 const& boneEndB, float radiusB );
bool DoCapsulesOverlap3D( Vec3 const& boneStartA, Vec3 const& boneEndA, float radiusA, Vec3 const& boneStartB, Vec3 const& boneEndB, float radiusB, Vec3& out_mtv_XY );
bool DoCapsuleOverlapSphere3D( Vec3 const& boneStart, Vec3 const& boneEnd, float capsuleRadius, Vec3 const& center, float sphereRadius );
bool DoCapsuleOverlapSphere3D( Vec3 const& boneStart, Vec3 const& boneEnd, float capsuleRadius, Vec3 const& center, float sphereRadius, Vec3& out_mtv_XY );
Vec2 const GetNearestPointOnDisc2D( Vec2 const& referencePos, Vec2 const& discCenter, float discRadius );
Vec2 const GetNearestPointOnAABB2D( Vec2 const& referencePos, AABB2 const& box );
Vec2 const GetNearestPointOnInfiniteLine2D( Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine );
Vec2 const GetNearestPointOnLineSegment2D( Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd );
Vec2 const GetNearestPointOnCapsule2D( Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius );
Vec2 const GetNearestPointOnOBB2D( Vec2 const& referencePos, OBB2 const& box );
Vec3 const GetNearestPointOnSphere3D( Vec3 const& referencePos, Vec3 const& sphereCenter, float sphereRadius );
Vec3 const GetNearestPointOnAABB3D( Vec3 const& referencePos, AABB3 const& box );
Vec3 const GetNearestPointOnLineSegment3D( Vec3 const& referencePos, Vec3 const& lineSegStart, Vec3 const& lineSegEnd );
Vec3 const GetNearestPointOnZCylinder3D( Vec3 const& referencePos, Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius );

Vec3 const GetNearestPointOnOBBToLineSegment3D( Vec3 const& lineSegStart, Vec3 const& lineSegEnd, OBB3 const& obb );

Mat44 const GetLookAtMatrix( Vec3 forwardVec );
Mat44 const CreateLookRotation( Vec3 const& forward, Vec3 const& up );
Mat44 const GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale );

bool PushDiscOutOfFixedPoint2D( Vec2 & mobileDiscCenter, float discRadius , Vec2 const& fixedpoint );
bool PushDiscOutOfFixedDisc2D( Vec2 & mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius );
bool PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius );
bool PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox );

bool PushOBBsOutOfEachOther3D( OBB3& obbA, OBB3& obbB );
bool PushOBBsOutOfFixedOBBs3D( OBB3& mobileOBB3, OBB3 const& FixedOBB3 );

bool BounceDiscsOffPoint( Vec2& center, float radius, Vec2& velocity, Vec2 const& point, float elasticity );
bool BounceDiscsOffEachOther( Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticity );
bool BounceDiscOffDisc( Vec2& centerA, float radiusA, Vec2& velocityA, Vec2 const& centerB, float radiusB, float elasticity );
bool BounceDiscOffCapsule( Vec2& center, float radius, Vec2& velocity, Vec2 const& boneStart, Vec2 const& boneEnd, float radiusCapsule, float elasticity );
bool BounceDiscOffOBB( Vec2& center, float radius, Vec2& velocity, OBB2 const& box, float elasticity );

void TransformPosition2D( Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation );
void TransformPositionXY3D( Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY );
void TransformPosition2D( Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation );
void TransformPositionXY3D( Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY );

float DotProduct2D( Vec2 const& a, Vec2 const& b );
float DotProduct3D( Vec3 const& a, Vec3 const& b );
float DotProduct4D( Vec4 const& a, Vec4 const& b );

float CrossProduct2D( Vec2 const& a, Vec2 const& b );
Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b);

float NormalizeByte( unsigned char targetByte );
unsigned char DenormalizeByte( float targetFloat );

float ComputeCubicBezier1D( float A, float B, float C, float D, float t );
float ComputeQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t );

float SmoothStart2( float t );
float SmoothStart3( float t );
float SmoothStart4( float t );
float SmoothStart5( float t );
float SmoothStart6( float t );

float SmoothStop2( float t );
float SmoothStop3( float t );
float SmoothStop4( float t );
float SmoothStop5( float t );
float SmoothStop6( float t );

float SmoothStep3( float t );
float SmoothStep5( float t );

float Hesitate3( float t );
float Hesitate5( float t );

float CustomFunkyEasingFunction( float t );

void xyzToZyx( float x, float y, float z, float& zyx_z, float& zyx_y, float& zyx_x );