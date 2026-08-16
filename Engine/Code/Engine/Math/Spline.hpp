#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <vector>
class CubicHermiteCurve2D;

class CubicBezierCurve2D
{
public:
	CubicBezierCurve2D() {};
	CubicBezierCurve2D( Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos );
	explicit CubicBezierCurve2D( CubicHermiteCurve2D const& fromHermite );
	Vec2 EvaluateAtParametric( float parametricZeroToOne ) const;
	float GetApproximateLength( int numSubdivisions = 64 ) const;
	Vec2 EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivision = 64 ) const;

public:
	Vec2 m_startPos;
	Vec2 m_guidePos1;
	Vec2 m_guidePos2;
	Vec2 m_endPos;
};

class CubicHermiteCurve2D
{
public:
	CubicHermiteCurve2D() {};
	CubicHermiteCurve2D( Vec2 startPos, Vec2 startVelocity, Vec2 endPos, Vec2 endVelocity );
	explicit CubicHermiteCurve2D( CubicBezierCurve2D const& fromHermite );
	Vec2 EvaluateAtParametric( float parametricZeroToOne ) const;
	float GetApproximateLength( int numSubdivisions = 64 ) const;
	Vec2 EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivision = 64 ) const;

public:
	Vec2 m_startPos;
	Vec2 m_startVelocity;
	Vec2 m_endPos;
	Vec2 m_endVelocity;
};

class CatmullRomSpline2D
{
public:
	CatmullRomSpline2D() {};
	CatmullRomSpline2D( std::vector<Vec2> const& positions );
	void SetPositions( std::vector<Vec2> const& positions );
	Vec2 EvaluateAtParametric( float parametricZeroToCurveNum ) const;
	float GetApproximateLength( int numSubdivisions = 64 ) const;
	Vec2 EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivision = 64 ) const;

public:
	std::vector<Vec2> m_positions;
	std::vector<Vec2> m_velocities;
};
