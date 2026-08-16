#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

EulerAngles const EulerAngles::ZERO = EulerAngles( 0.f, 0.f, 0.f );

EulerAngles::EulerAngles( std::string data )
{
	SetFromText( data );
}

EulerAngles::EulerAngles( float yawDegrees, float pitchDegrees, float rollDegrees )
{
	m_yawDegrees = yawDegrees;
	m_pitchDegrees = pitchDegrees;
	m_rollDegrees = rollDegrees;
}

EulerAngles::EulerAngles( Vec3 vec3 )
{
	m_yawDegrees = vec3.x;
	m_pitchDegrees = vec3.y;
	m_rollDegrees = vec3.z;
}

void EulerAngles::GetAsVectors_IFwd_JLeft_KUp( Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_UpKBasis )
{
	float CosYaw = CosDegrees( m_yawDegrees );
	float CosPitch = CosDegrees( m_pitchDegrees );
	float CosRoll = CosDegrees( m_rollDegrees );
	float SinYaw = SinDegrees( m_yawDegrees );
	float SinPitch = SinDegrees( m_pitchDegrees );
	float SinRoll = SinDegrees( m_rollDegrees );

	Mat44 mat = Mat44();
	mat.m_values[Mat44::Ix] = CosYaw * CosPitch;
	mat.m_values[Mat44::Iy] = SinYaw * CosPitch;
	mat.m_values[Mat44::Iz] = -SinPitch;

	mat.m_values[Mat44::Jx] = -SinYaw * CosRoll + CosYaw * SinPitch * SinRoll;
	mat.m_values[Mat44::Jy] = CosYaw * CosRoll + SinRoll * SinYaw * SinPitch;
	mat.m_values[Mat44::Jz] = CosPitch * SinRoll;

	mat.m_values[Mat44::Kx] = SinRoll * SinYaw + CosYaw * SinPitch * CosRoll;
	mat.m_values[Mat44::Ky] = -CosYaw * SinRoll + CosRoll * SinYaw * SinPitch;
	mat.m_values[Mat44::Kz] = CosPitch * CosRoll;

	out_forwardIBasis = mat.GetIBasis3D();
	out_leftJBasis = mat.GetJBasis3D();
	out_UpKBasis = mat.GetKBasis3D();
}

Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	float CosYaw = CosDegrees( m_yawDegrees );
	float CosPitch = CosDegrees( m_pitchDegrees );
	float CosRoll = CosDegrees( m_rollDegrees );
	float SinYaw = SinDegrees( m_yawDegrees );
	float SinPitch = SinDegrees( m_pitchDegrees );
	float SinRoll = SinDegrees( m_rollDegrees );

	Mat44 mat = Mat44();
	mat.m_values[Mat44::Ix] = CosYaw * CosPitch;
	mat.m_values[Mat44::Iy] = SinYaw * CosPitch;
	mat.m_values[Mat44::Iz] = -SinPitch;

	mat.m_values[Mat44::Jx] = -SinYaw * CosRoll + CosYaw * SinPitch * SinRoll;
	mat.m_values[Mat44::Jy] = CosYaw * CosRoll + SinRoll * SinYaw * SinPitch;
	mat.m_values[Mat44::Jz] = CosPitch * SinRoll;

	mat.m_values[Mat44::Kx] = SinRoll * SinYaw + CosYaw * SinPitch * CosRoll;
	mat.m_values[Mat44::Ky] = -CosYaw * SinRoll + CosRoll * SinYaw * SinPitch;
	mat.m_values[Mat44::Kz] = CosPitch * CosRoll;

	return mat;
}

Vec3 EulerAngles::GetForwardDir_XFwd_YLeft_Zup()
{
	float cy = CosDegrees( m_yawDegrees );
	float sy = SinDegrees( m_yawDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sp = SinDegrees( m_pitchDegrees );
	return Vec3( cy * cp, sy * cp, -sp );
}

void EulerAngles::SetFromText( std::string text )
{
	Strings elems = Split( text, ',' );
	m_yawDegrees = stof( elems[0] );
	m_pitchDegrees = stof( elems[1] );
	m_rollDegrees = stof( elems[2] );
}

EulerAngles const EulerAngles::operator+( EulerAngles const& other ) const
{
	EulerAngles result;
	result.m_yawDegrees = m_yawDegrees + other.m_yawDegrees;
	result.m_rollDegrees = m_rollDegrees + other.m_rollDegrees;
	result.m_pitchDegrees = m_pitchDegrees + other.m_pitchDegrees;
	return result;
}