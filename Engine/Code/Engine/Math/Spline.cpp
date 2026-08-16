#include "Engine/Math/Spline.hpp"
#include "Engine/Math/MathUtils.hpp"

CubicBezierCurve2D::CubicBezierCurve2D( Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos )
	: m_startPos( startPos )
	, m_guidePos1( guidePos1 )
	, m_guidePos2( guidePos2 )
	, m_endPos( endPos )
{
}

CubicBezierCurve2D::CubicBezierCurve2D( CubicHermiteCurve2D const& fromHermite )
{
	m_startPos = fromHermite.m_startPos;
	m_guidePos1 = fromHermite.m_startPos + fromHermite.m_startVelocity / 3.f;
	m_guidePos2 = fromHermite.m_endPos - fromHermite.m_endVelocity / 3.f;
	m_endPos = fromHermite.m_endPos;
}

Vec2 CubicBezierCurve2D::EvaluateAtParametric( float parametricZeroToOne ) const
{
	Vec2 AB = Interpolate( m_startPos, m_guidePos1, parametricZeroToOne );
	Vec2 BC = Interpolate( m_guidePos1, m_guidePos2, parametricZeroToOne );
	Vec2 CD = Interpolate( m_guidePos2, m_endPos, parametricZeroToOne );

	Vec2 ABC = Interpolate( AB, BC, parametricZeroToOne );
	Vec2 BCD = Interpolate( BC, CD, parametricZeroToOne );

	Vec2 ABCD = Interpolate( ABC, BCD, parametricZeroToOne );

	return ABCD;
}

float CubicBezierCurve2D::GetApproximateLength( int numSubdivisions ) const
{
	float length = 0.f;
	float sub = 1.f / (float)numSubdivisions;
	for (int i = 0; i < numSubdivisions; i++)
	{
		Vec2 currentPoint = EvaluateAtParametric( sub * i );
		Vec2 nextPoint = EvaluateAtParametric( sub * (i + 1) );
		length += GetDistance2D( currentPoint, nextPoint );
	}
	return length;
}

Vec2 CubicBezierCurve2D::EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivision ) const
{
	float length = 0;
	float sub = 1.f / (float)numSubdivision;
	float distRemain = distanceAlongCurve;
	Vec2 resultPos;
	for (int i = 0; i < numSubdivision; i++)
	{
		Vec2 currentPos = EvaluateAtParametric( sub * i );
		Vec2 nextPos = EvaluateAtParametric( sub * (i + 1) );
		float dist = GetDistance2D( currentPos, nextPos );
		length += dist;
		if (length > distanceAlongCurve)
		{
			return Interpolate( currentPos, nextPos, distRemain / dist );
		}
		else
		{
			distRemain -= dist;
		}
	}
	return Vec2::ZERO;
}

CubicHermiteCurve2D::CubicHermiteCurve2D( Vec2 startPos, Vec2 startVelocity, Vec2 endPos, Vec2 endVelocity )
	:m_startPos( startPos )
	, m_startVelocity( startVelocity )
	, m_endPos( endPos )
	, m_endVelocity( endVelocity )
{
}

CubicHermiteCurve2D::CubicHermiteCurve2D( CubicBezierCurve2D const& fromHermite )
{
	m_startPos = fromHermite.m_startPos;
	m_startVelocity = (fromHermite.m_guidePos1 - fromHermite.m_startPos) * 3.f;
	m_endPos = fromHermite.m_endPos;
	m_endVelocity = (fromHermite.m_endPos - fromHermite.m_guidePos2) * 3.f;
}

Vec2 CubicHermiteCurve2D::EvaluateAtParametric( float parametricZeroToOne ) const
{
	CubicBezierCurve2D bezierCurve = CubicBezierCurve2D( *this );
	return bezierCurve.EvaluateAtParametric( parametricZeroToOne );
}

float CubicHermiteCurve2D::GetApproximateLength( int numSubdivisions ) const
{
	CubicBezierCurve2D bezierCurve = CubicBezierCurve2D( *this );
	return bezierCurve.GetApproximateLength( numSubdivisions );
}

Vec2 CubicHermiteCurve2D::EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivision ) const
{
	CubicBezierCurve2D bezierCurve = CubicBezierCurve2D( *this );
	return bezierCurve.EvaluateAtApproximateDistance( distanceAlongCurve, numSubdivision );
}

CatmullRomSpline2D::CatmullRomSpline2D( std::vector<Vec2> const& positions )
{
	if (positions.empty())
		return;

	m_positions = positions;
	m_velocities.resize( m_positions.size() );
	m_velocities[0] = Vec2( 0.f, 0.f );
	m_velocities.back() = Vec2( 0.f, 0.f );
	for (size_t i = 1; i < m_positions.size() - 1; i++)
	{
		Vec2 prevPos = m_positions[i - 1];
		Vec2 nextPos = m_positions[i + 1];
		m_velocities[i] = (nextPos - prevPos) * 0.5f;
	}
}

void CatmullRomSpline2D::SetPositions( std::vector<Vec2> const& positions )
{
	if (positions.empty())
		return;

	m_positions = positions;
	m_velocities.resize( m_positions.size() );
	m_velocities[0] = Vec2( 0.f, 0.f );
	m_velocities.back() = Vec2( 0.f, 0.f );
	for (size_t i = 1; i < m_positions.size() - 1; i++)
	{
		Vec2 prevPos = m_positions[i - 1];
		Vec2 nextPos = m_positions[i + 1];
		m_velocities[i] = (nextPos - prevPos) * 0.5f;
	}
}

Vec2 CatmullRomSpline2D::EvaluateAtParametric( float parametricZeroToCurveNum ) const
{
	for (size_t i = 0; i < m_positions.size() - 1; i++)
	{
		if (parametricZeroToCurveNum <= 1.f)
		{
			CubicHermiteCurve2D hertmiteCurve( m_positions[i], m_velocities[i], m_positions[i + 1], m_velocities[i + 1] );
			return hertmiteCurve.EvaluateAtParametric( parametricZeroToCurveNum );
		}
		parametricZeroToCurveNum -= 1.f;
	}
	return Vec2::ZERO;
}

float CatmullRomSpline2D::GetApproximateLength( int numSubdivisions ) const
{
	float length = 0.f;
	for (size_t i = 0; i < m_positions.size() - 1; i++)
	{
		CubicHermiteCurve2D hertmiteCurve( m_positions[i], m_velocities[i], m_positions[i + 1], m_velocities[i + 1] );
		length += hertmiteCurve.GetApproximateLength( numSubdivisions );
	}
	return length;
}

Vec2 CatmullRomSpline2D::EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivision ) const
{
	float distRemain = distanceAlongCurve;
	for (size_t i = 0; i < m_positions.size() - 1; i++)
	{
		CubicHermiteCurve2D hertmiteCurve( m_positions[i], m_velocities[i], m_positions[i + 1], m_velocities[i + 1] );
		float currentCurveDistance = hertmiteCurve.GetApproximateLength( numSubdivision );
		if (distRemain < currentCurveDistance)
		{
			return hertmiteCurve.EvaluateAtApproximateDistance( distRemain, numSubdivision );
		}
		else
		{
			distRemain -= currentCurveDistance;
		}

	}
	return Vec2::ZERO;
}
