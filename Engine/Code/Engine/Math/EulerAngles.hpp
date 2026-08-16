#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"

class EulerAngles
{
public:
	EulerAngles() = default;
	EulerAngles( std::string data );
	EulerAngles( Vec3 vec3 );
	EulerAngles( float yawDegrees, float pitchDegrees, float rollDegrees );
	void GetAsVectors_IFwd_JLeft_KUp( Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_UpKBasis );
	Mat44 GetAsMatrix_IFwd_JLeft_KUp() const;
	Vec3 GetForwardDir_XFwd_YLeft_Zup();

	void SetFromText( std::string data );

	EulerAngles const operator+( EulerAngles const& other ) const;

public:
	float m_yawDegrees = 0.0f;
	float m_pitchDegrees = 0.0f;
	float m_rollDegrees = 0.0f;

public:
	static const EulerAngles ZERO;
};
