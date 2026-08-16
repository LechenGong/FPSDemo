#include "Engine/Animation/IKSolver.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quat.hpp"

namespace
{
constexpr float IK_EPSILON = 0.0001f;

Vec3 GetSafeNormalized( Vec3 const& vector, Vec3 const& fallback )
{
	if (vector.GetLengthSquared() > IK_EPSILON * IK_EPSILON)
	{
		return vector.GetNormalized();
	}

	if (fallback.GetLengthSquared() > IK_EPSILON * IK_EPSILON)
	{
		return fallback.GetNormalized();
	}

	return Vec3( 1.f, 0.f, 0.f );
}

Vec3 RejectFromAxis( Vec3 const& vector, Vec3 const& axis )
{
	return vector - GetProjectedOnto3D( vector, axis );
}

Quat GetRotationOnlyQuat( Mat44 const& transform )
{
	Mat44 rotationOnly;
	rotationOnly.SetIJKT3D(
		GetSafeNormalized( transform.GetIBasis3D(), Vec3( 1.f, 0.f, 0.f ) ),
		GetSafeNormalized( transform.GetJBasis3D(), Vec3( 0.f, 1.f, 0.f ) ),
		GetSafeNormalized( transform.GetKBasis3D(), Vec3( 0.f, 0.f, 1.f ) ),
		Vec3::ZERO );
	return Quat( rotationOnly ).GetNormalized();
}

Quat GetFromToRotation( Vec3 const& fromDir, Vec3 const& toDir, Vec3 const& fallbackAxis )
{
	Vec3 from = GetSafeNormalized( fromDir, Vec3( 1.f, 0.f, 0.f ) );
	Vec3 to = GetSafeNormalized( toDir, from );

	float dot = Clamp( DotProduct3D( from, to ), -1.f, 1.f );
	if (dot > 1.f - IK_EPSILON)
	{
		return Quat::IDENTITY;
	}

	Vec3 axis = CrossProduct3D( from, to );
	if (axis.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		axis = RejectFromAxis( fallbackAxis, from );
		if (axis.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
		{
			axis = RejectFromAxis( Vec3::UP, from );
		}
		if (axis.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
		{
			axis = RejectFromAxis( Vec3( 0.f, 1.f, 0.f ), from );
		}
	}
	axis = GetSafeNormalized( axis, Vec3::UP );

	float angleDegrees = ACosDegrees( dot );
	return Quat( axis, angleDegrees ).GetNormalized();
}

Mat44 BuildRotatedJointTransform( Mat44 const& currentTransform, Vec3 const& jointPos, Vec3 const& currentDir, Vec3 const& desiredDir, Vec3 const& fallbackAxis )
{
	Quat currentRotation = GetRotationOnlyQuat( currentTransform );
	Quat deltaRotation = GetFromToRotation( currentDir, desiredDir, fallbackAxis );
	Quat solvedRotation = (deltaRotation * currentRotation).GetNormalized();
	return Mat44( jointPos, solvedRotation, currentTransform.GetScale3D() );
}
}

bool IKSolver::SolveTwoBoneIK( TwoBoneIKInput const& input, TwoBoneIKResult& output )
{
	float upperLen = (input.midPos - input.rootPos).GetLength();
	float lowerLen = (input.endPos - input.midPos).GetLength();
	if (upperLen <= IK_EPSILON || lowerLen <= IK_EPSILON)
	{
		return false;
	}

	Vec3 currentChainDir = GetSafeNormalized( input.endPos - input.rootPos, Vec3( 1.f, 0.f, 0.f ) );
	Vec3 toTarget = input.targetPos - input.rootPos;
	Vec3 targetDir = GetSafeNormalized( toTarget, currentChainDir );

	float minDist = fabsf( upperLen - lowerLen ) + 0.001f;
	float maxDist = upperLen + lowerLen - 0.001f;
	if (maxDist < minDist)
	{
		maxDist = minDist;
	}
	float targetDist = Clamp( toTarget.GetLength(), minDist, maxDist );

	Vec3 poleVector = input.poleVector - input.rootPos;
	Vec3 bendHint = RejectFromAxis( poleVector, targetDir );
	if (bendHint.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		bendHint = RejectFromAxis( input.midPos - input.rootPos, targetDir );
	}
	if (bendHint.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		bendHint = RejectFromAxis( input.midPos - input.rootPos, currentChainDir );
	}
	if (bendHint.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		bendHint = RejectFromAxis( Vec3::UP, targetDir );
	}
	if (bendHint.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		bendHint = RejectFromAxis( Vec3( 0.f, 1.f, 0.f ), targetDir );
	}
	bendHint = GetSafeNormalized( bendHint, Vec3::UP );

	Vec3 planeNormal = CrossProduct3D( targetDir, bendHint );
	if (planeNormal.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		planeNormal = CrossProduct3D( targetDir, Vec3::UP );
	}
	if (planeNormal.GetLengthSquared() <= IK_EPSILON * IK_EPSILON)
	{
		planeNormal = CrossProduct3D( targetDir, Vec3( 0.f, 1.f, 0.f ) );
	}
	planeNormal = GetSafeNormalized( planeNormal, Vec3( 0.f, 1.f, 0.f ) );

	Vec3 bendDir = GetSafeNormalized( CrossProduct3D( planeNormal, targetDir ), bendHint );

	float cosRootAngle = Clamp( (upperLen * upperLen + targetDist * targetDist - lowerLen * lowerLen) / (2.f * upperLen * targetDist), -1.f, 1.f );
	float rootAngle = acosf( cosRootAngle );

	float rootAlongTarget = cosf( rootAngle ) * upperLen;
	float rootAlongBend = sinf( rootAngle ) * upperLen;

	output.solvedMidPos = input.rootPos + targetDir * rootAlongTarget + bendDir * rootAlongBend;
	output.solvedEndPos = input.rootPos + targetDir * targetDist;

	output.rootWorldTransform = BuildRotatedJointTransform(
		input.rootTransform,
		input.rootPos,
		input.midPos - input.rootPos,
		output.solvedMidPos - input.rootPos,
		planeNormal );

	output.midWorldTransform = BuildRotatedJointTransform(
		input.midTransform,
		output.solvedMidPos,
		input.endPos - input.midPos,
		output.solvedEndPos - output.solvedMidPos,
		planeNormal );

	output.endWorldTransform = input.endTransform;
	output.endWorldTransform.SetTranslation3D( output.solvedEndPos );
	return true;
}
