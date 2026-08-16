#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quat.hpp"

#include "Engine/Core/StringUtils.hpp"

Mat44::Mat44()
{
	m_values[Ix] = 1; m_values[Jx] = 0; m_values[Kx] = 0; m_values[Tx] = 0;
	m_values[Iy] = 0; m_values[Jy] = 1; m_values[Ky] = 0; m_values[Ty] = 0;
	m_values[Iz] = 0; m_values[Jz] = 0; m_values[Kz] = 1; m_values[Tz] = 0;
	m_values[Iw] = 0; m_values[Jw] = 0; m_values[Kw] = 0; m_values[Tw] = 1;
}

Mat44::Mat44( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D )
{
	m_values[Ix] = iBasis2D.x;	m_values[Jx] = jBasis2D.x;	m_values[Kx] = 0;	m_values[Tx] = translation2D.x;
	m_values[Iy] = iBasis2D.y;	m_values[Jy] = jBasis2D.y;	m_values[Ky] = 0;	m_values[Ty] = translation2D.y;
	m_values[Iz] = 0;			m_values[Jz] = 0;			m_values[Kz] = 1;	m_values[Tz] = 0;
	m_values[Iw] = 0;			m_values[Jw] = 0;			m_values[Kw] = 0;	m_values[Tw] = 1;
}

Mat44::Mat44( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D )
{
	m_values[Ix] = iBasis3D.x;	m_values[Jx] = jBasis3D.x;	m_values[Kx] = kBasis3D.x;	m_values[Tx] = translation3D.x;
	m_values[Iy] = iBasis3D.y;	m_values[Jy] = jBasis3D.y;	m_values[Ky] = kBasis3D.y;	m_values[Ty] = translation3D.y;
	m_values[Iz] = iBasis3D.z;	m_values[Jz] = jBasis3D.z;	m_values[Kz] = kBasis3D.z;	m_values[Tz] = translation3D.z;
	m_values[Iw] = 0;			m_values[Jw] = 0;			m_values[Kw] = 0;			m_values[Tw] = 1;
}

Mat44::Mat44( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D )
{
	m_values[Ix] = iBasis4D.x;	m_values[Jx] = jBasis4D.x;	m_values[Kx] = kBasis4D.x;	m_values[Tx] = translation4D.x;
	m_values[Iy] = iBasis4D.y;	m_values[Jy] = jBasis4D.y;	m_values[Ky] = kBasis4D.y;	m_values[Ty] = translation4D.y;
	m_values[Iz] = iBasis4D.z;	m_values[Jz] = jBasis4D.z;	m_values[Kz] = kBasis4D.z;	m_values[Tz] = translation4D.z;
	m_values[Iw] = iBasis4D.w;	m_values[Jw] = jBasis4D.w;	m_values[Kw] = kBasis4D.w;	m_values[Tw] = translation4D.w;
}

Mat44::Mat44( float const* sixteenValueBasisMajor )
{
	m_values[Ix] = sixteenValueBasisMajor[0];	m_values[Jx] = sixteenValueBasisMajor[4];	m_values[Kx] = sixteenValueBasisMajor[8];	m_values[Tx] = sixteenValueBasisMajor[12];
	m_values[Iy] = sixteenValueBasisMajor[1];	m_values[Jy] = sixteenValueBasisMajor[5];	m_values[Ky] = sixteenValueBasisMajor[9];	m_values[Ty] = sixteenValueBasisMajor[13];
	m_values[Iz] = sixteenValueBasisMajor[2];	m_values[Jz] = sixteenValueBasisMajor[6];	m_values[Kz] = sixteenValueBasisMajor[10];	m_values[Tz] = sixteenValueBasisMajor[14];
	m_values[Iw] = sixteenValueBasisMajor[3];	m_values[Jw] = sixteenValueBasisMajor[7];	m_values[Kw] = sixteenValueBasisMajor[11];	m_values[Tw] = sixteenValueBasisMajor[15];
}

Mat44::Mat44( Vec3 const& translation3D, Quat const& rotation3D, Vec3 const& scale3D )
{
	Mat44 transformMatrix = CreateTranslation3D( translation3D );
	transformMatrix.Append( rotation3D.ToRotationMatrix() );
	transformMatrix.Scale3D( scale3D );
	*this = transformMatrix;
}

std::string Mat44::ToString() const
{
	std::string result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result += std::to_string( GetElement( i, j ) );
			result += ' ';
		}
	}
	return result;
}

void Mat44::FromString( std::string const& str )
{
	Strings strs = Split( str, ' ', true );
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			SetElement( i, j, std::stof( strs[i * 4 + j] ) );
		}
	}
}

Mat44 const Mat44::CreateTranslation2D( Vec2 const& translationXY )
{
	return Mat44( 
		Vec2( 1.f, 0.f ), 
		Vec2( 0.f, 1.f ), 
		translationXY 
	);
}

Mat44 const Mat44::CreateTranslation3D( Vec3 const& translationXYZ )
{
	return Mat44( 
		Vec3( 1.f, 0.f, 0.f ), 
		Vec3( 0.f, 1.f, 0.f ), 
		Vec3( 0.f, 0.f, 1.f ), 
		translationXYZ 
	);
}

Mat44 const Mat44::CreateUniformScale2D( float uniformScaleXY )
{
	return Mat44(
		Vec2( uniformScaleXY, 0.f ),
		Vec2( 0.f, uniformScaleXY ),
		Vec2( 0.f, 0.f )
	);
}

Mat44 const Mat44::CreateUniformScale3D( float uniformScaleXYZ )
{
	return Mat44( 
		Vec3( uniformScaleXYZ, 0.f, 0.f ),
		Vec3( 0.f, uniformScaleXYZ, 0.f ),
		Vec3( 0.f, 0.f, uniformScaleXYZ ),
		Vec3( 0.f, 0.f, 0.f )
	);
}

Mat44 const Mat44::CreateNonUniformScale2D( Vec2 const& nonUniformScaleXY )
{
	return Mat44( 
		Vec2( nonUniformScaleXY.x, 0.f ), 
		Vec2( 0.f, nonUniformScaleXY.y ), 
		Vec2( 0.f, 0.f ) 
	);
}

Mat44 const Mat44::CreateNonUniformScale3D( Vec3 const& nonUniformScaleXYZ )
{
	return Mat44( 
		Vec3( nonUniformScaleXYZ.x, 0.f, 0.f ), 
		Vec3( 0.f, nonUniformScaleXYZ.y, 0.f ), 
		Vec3( 0.f, 0.f, nonUniformScaleXYZ.z ), 
		Vec3( 0.f, 0.f, 0.f ) 
	);
}

Mat44 const Mat44::CreateZRotationDegrees( float rotationDegreesAboutZ )
{
	float sinTheta = SinDegrees( rotationDegreesAboutZ );
	float cosTheta = CosDegrees( rotationDegreesAboutZ );
	return Mat44( 
		Vec2( cosTheta, sinTheta ), 
		Vec2( -sinTheta, cosTheta ), 
		Vec2( 0.f, 0.f ) 
	);
}

Mat44 const Mat44::CreateYRotationDegrees( float rotationDegreesAboutY )
{
	float sinTheta = SinDegrees( rotationDegreesAboutY );
	float cosTheta = CosDegrees( rotationDegreesAboutY );
	return Mat44( 
		Vec3( cosTheta, 0.f, -sinTheta ), 
		Vec3( 0.f, 1.f, 0.f ), 
		Vec3( sinTheta, 0.f, cosTheta ), 
		Vec3( 0.f, 0.f, 0.f ) 
	);
}

Mat44 const Mat44::CreateXRotationDegrees( float rotationDegreesAboutX )
{
	float sinTheta = SinDegrees( rotationDegreesAboutX );
	float cosTheta = CosDegrees( rotationDegreesAboutX );
	return Mat44( 
		Vec3( 1.f, 0.f, 0.f ), 
		Vec3( 0.f, cosTheta, sinTheta ), 
		Vec3( 0.f, -sinTheta, cosTheta ), 
		Vec3( 0.f, 0.f, 0.f ) 
	);
}

Mat44 const Mat44::CreateOrthoProjection( float left, float right, float bottom, float top, float zNear, float zFar )
{
	Mat44 ortho;
	ortho.m_values[Ix] = 2.f / (right - left);
	ortho.m_values[Jy] = 2.f / (top - bottom);
	ortho.m_values[Kz] = 1.f / (zFar - zNear);
	ortho.m_values[Tx] = (left + right) / (left - right);
	ortho.m_values[Ty] = (bottom + top) / (bottom - top);
	ortho.m_values[Tz] = zNear / (zNear - zFar);
	return ortho;
}

Mat44 const Mat44::CreatePerspectiveProjection( float fovDegrees, float aspect, float zNear, float zFar )
{
	Mat44 perspective;
	//this is scale Y
	perspective.m_values[Jy] = CosDegrees( fovDegrees * 0.5f ) / SinDegrees( fovDegrees * 0.5f );  // equals 1 if vertical Field of View is 90
	//this is scale X
	perspective.m_values[Ix] = perspective.m_values[Jy] / aspect; // equals scaleY if screen is square (aspect=1.0); equals 1 if square screen and FOV 90
	perspective.m_values[Kz] = zFar / (zFar - zNear);
	perspective.m_values[Kw] = 1.0f; // this puts Z into the W component (in preparation for the hardware w-divide)
	// this is translate Z
	perspective.m_values[Tz] = (zNear * zFar) / (zNear - zFar);
	perspective.m_values[Tw] = 0.0f;  // Otherwise we would be putting Z+1 (instead of Z) into the W component
	return perspective;
}

Mat44 const Mat44::Interpolate( Mat44 const& matrixA, Mat44 const& matrixB, float factor )
{
	Vec3 interpolatedTranslation = matrixA.GetTranslation3D() * (1.0f - factor) + matrixB.GetTranslation3D() * factor;
	Quat interpolatedRotation = Quat::SLERP( Quat( matrixA ), Quat( matrixB ), factor );
	Vec3 interpolatedScale = matrixA.GetScale3D() * (1.0f - factor) + matrixB.GetScale3D() * factor;

	return Mat44( interpolatedTranslation, interpolatedRotation, interpolatedScale );
}

Vec2 const Mat44::TransformVectorQuantity2D( Vec2 const& vectorQuantityXY ) const
{
	return Vec2(
		m_values[Ix] * vectorQuantityXY.x + m_values[Jx] * vectorQuantityXY.y,
		m_values[Iy] * vectorQuantityXY.x + m_values[Jy] * vectorQuantityXY.y
	);
}

Vec3 const Mat44::TransformVectorQuantity3D( Vec3 const& vectorQuantityXY ) const
{
	return Vec3(
		m_values[Ix] * vectorQuantityXY.x + m_values[Jx] * vectorQuantityXY.y + m_values[Kx] * vectorQuantityXY.z,
		m_values[Iy] * vectorQuantityXY.x + m_values[Jy] * vectorQuantityXY.y + m_values[Ky] * vectorQuantityXY.z,
		m_values[Iz] * vectorQuantityXY.x + m_values[Jz] * vectorQuantityXY.y + m_values[Kz] * vectorQuantityXY.z
	);
}

Vec2 const Mat44::TransformPosition2D( Vec2 const& positionXY ) const
{
	return Vec2(
		m_values[Ix] * positionXY.x + m_values[Jx] * positionXY.y + m_values[Tx],
		m_values[Iy] * positionXY.x + m_values[Jy] * positionXY.y + m_values[Ty]
	);
}

Vec3 const Mat44::TransformPosition3D( Vec3 const& position3D ) const
{
	return Vec3(
		m_values[Ix] * position3D.x + m_values[Jx] * position3D.y + m_values[Kx] * position3D.z + m_values[Tx],
		m_values[Iy] * position3D.x + m_values[Jy] * position3D.y + m_values[Ky] * position3D.z + m_values[Ty],
		m_values[Iz] * position3D.x + m_values[Jz] * position3D.y + m_values[Kz] * position3D.z + m_values[Tz]
	);
}

Vec4 const Mat44::TransformPosition4D( Vec4 const& position4D ) const
{
	return Vec4(
		m_values[Ix] * position4D.x + m_values[Jx] * position4D.y + m_values[Kx] * position4D.z + m_values[Tx] * position4D.w,
		m_values[Iy] * position4D.x + m_values[Jy] * position4D.y + m_values[Ky] * position4D.z + m_values[Ty] * position4D.w,
		m_values[Iz] * position4D.x + m_values[Jz] * position4D.y + m_values[Kz] * position4D.z + m_values[Tz] * position4D.w,
		m_values[Ix] * position4D.x + m_values[Jy] * position4D.y + m_values[Kz] * position4D.z + m_values[Tz] * position4D.w
	);
}

Vec4 const Mat44::TransformHomogeneous3D( Vec4 const& homogeneousPoint3D ) const
{
	return Vec4(
		m_values[Ix] * homogeneousPoint3D.x + m_values[Jx] * homogeneousPoint3D.y + m_values[Kx] * homogeneousPoint3D.z + m_values[Tx] * homogeneousPoint3D.w,
		m_values[Iy] * homogeneousPoint3D.x + m_values[Jy] * homogeneousPoint3D.y + m_values[Ky] * homogeneousPoint3D.z + m_values[Ty] * homogeneousPoint3D.w,
		m_values[Iz] * homogeneousPoint3D.x + m_values[Jz] * homogeneousPoint3D.y + m_values[Kz] * homogeneousPoint3D.z + m_values[Tz] * homogeneousPoint3D.w,
		m_values[Iw] * homogeneousPoint3D.x + m_values[Jw] * homogeneousPoint3D.y + m_values[Kw] * homogeneousPoint3D.z + m_values[Tw] * homogeneousPoint3D.w
	);
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

float const Mat44::GetElement( int i, int j ) const
{
	return m_values[j * 4 + i];
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2( m_values[Ix], m_values[Iy] );
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2( m_values[Jx], m_values[Jy] );
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2( m_values[Tx], m_values[Ty] );
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3( m_values[Ix], m_values[Iy], m_values[Iz] );
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3( m_values[Jx], m_values[Jy], m_values[Jz] );
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3( m_values[Kx], m_values[Ky], m_values[Kz] );
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3( m_values[Tx], m_values[Ty], m_values[Tz] );
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4( m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw] );
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4( m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw] );
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4( m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw] );
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4( m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw] );
}

Vec3 const Mat44::GetScale3D() const
{
	return Vec3(
		GetIBasis3D().GetLength(),
		GetJBasis3D().GetLength(),
		GetKBasis3D().GetLength()
	);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Vec3 transform = this->GetTranslation3D();
	Mat44 rotation = Mat44( this->GetIBasis3D(), this->GetJBasis3D(), this->GetKBasis3D(), Vec3::ZERO );
	rotation.Transpose();
	rotation.AppendTranslation3D( transform * -1 );
	return rotation;
}

Mat44 const Mat44::GetInverse() const
{
	Mat44 inv;
	float det;

	inv.m_values[Ix] = m_values[Jy] * m_values[Kz] * m_values[Tw] -
		m_values[Jy] * m_values[Kw] * m_values[Tz] -
		m_values[Ky] * m_values[Jz] * m_values[Tw] +
		m_values[Ky] * m_values[Jw] * m_values[Tz] +
		m_values[Ty] * m_values[Jz] * m_values[Kw] -
		m_values[Ty] * m_values[Jw] * m_values[Kz];

	inv.m_values[Jx] = -m_values[Jx] * m_values[Kz] * m_values[Tw] +
		m_values[Jx] * m_values[Kw] * m_values[Tz] +
		m_values[Kx] * m_values[Jz] * m_values[Tw] -
		m_values[Kx] * m_values[Jw] * m_values[Tz] -
		m_values[Tx] * m_values[Jz] * m_values[Kw] +
		m_values[Tx] * m_values[Jw] * m_values[Kz];

	inv.m_values[Kx] = m_values[Jx] * m_values[Ky] * m_values[Tw] -
		m_values[Jx] * m_values[Kw] * m_values[Ty] -
		m_values[Kx] * m_values[Jy] * m_values[Tw] +
		m_values[Kx] * m_values[Jw] * m_values[Ty] +
		m_values[Tx] * m_values[Jy] * m_values[Kw] -
		m_values[Tx] * m_values[Jw] * m_values[Ky];

	inv.m_values[Tx] = -m_values[Jx] * m_values[Ky] * m_values[Tz] +
		m_values[Jx] * m_values[Kz] * m_values[Ty] +
		m_values[Kx] * m_values[Jy] * m_values[Tz] -
		m_values[Kx] * m_values[Jz] * m_values[Ty] -
		m_values[Tx] * m_values[Jy] * m_values[Kz] +
		m_values[Tx] * m_values[Jz] * m_values[Ky];

	inv.m_values[Iy] = -m_values[Iy] * m_values[Kz] * m_values[Tw] +
		m_values[Iy] * m_values[Kw] * m_values[Tz] +
		m_values[Ky] * m_values[Iz] * m_values[Tw] -
		m_values[Ky] * m_values[Iw] * m_values[Tz] -
		m_values[Ty] * m_values[Iz] * m_values[Kw] +
		m_values[Ty] * m_values[Iw] * m_values[Kz];

	inv.m_values[Jy] = m_values[Ix] * m_values[Kz] * m_values[Tw] -
		m_values[Ix] * m_values[Kw] * m_values[Tz] -
		m_values[Kx] * m_values[Iz] * m_values[Tw] +
		m_values[Kx] * m_values[Iw] * m_values[Tz] +
		m_values[Tx] * m_values[Iz] * m_values[Kw] -
		m_values[Tx] * m_values[Iw] * m_values[Kz];

	inv.m_values[Ky] = -m_values[Ix] * m_values[Ky] * m_values[Tw] +
		m_values[Ix] * m_values[Kw] * m_values[Ty] +
		m_values[Kx] * m_values[Iy] * m_values[Tw] -
		m_values[Kx] * m_values[Iw] * m_values[Ty] -
		m_values[Tx] * m_values[Iy] * m_values[Kw] +
		m_values[Tx] * m_values[Iw] * m_values[Ky];

	inv.m_values[Ty] = m_values[Ix] * m_values[Ky] * m_values[Tz] -
		m_values[Ix] * m_values[Kz] * m_values[Ty] -
		m_values[Kx] * m_values[Iy] * m_values[Tz] +
		m_values[Kx] * m_values[Iz] * m_values[Ty] +
		m_values[Tx] * m_values[Iy] * m_values[Kz] -
		m_values[Tx] * m_values[Iz] * m_values[Ky];

	inv.m_values[Iz] = m_values[Iy] * m_values[Jz] * m_values[Tw] -
		m_values[Iy] * m_values[Jw] * m_values[Tz] -
		m_values[Jy] * m_values[Iz] * m_values[Tw] +
		m_values[Jy] * m_values[Iw] * m_values[Tz] +
		m_values[Ty] * m_values[Iz] * m_values[Jw] -
		m_values[Ty] * m_values[Iw] * m_values[Jz];

	inv.m_values[Jz] = -m_values[Ix] * m_values[Jz] * m_values[Tw] +
		m_values[Ix] * m_values[Jw] * m_values[Tz] +
		m_values[Jx] * m_values[Iz] * m_values[Tw] -
		m_values[Jx] * m_values[Iw] * m_values[Tz] -
		m_values[Tx] * m_values[Iz] * m_values[Jw] +
		m_values[Tx] * m_values[Iw] * m_values[Jz];

	inv.m_values[Kz] = m_values[Ix] * m_values[Jy] * m_values[Tw] -
		m_values[Ix] * m_values[Jw] * m_values[Ty] -
		m_values[Jx] * m_values[Iy] * m_values[Tw] +
		m_values[Jx] * m_values[Iw] * m_values[Ty] +
		m_values[Tx] * m_values[Iy] * m_values[Jw] -
		m_values[Tx] * m_values[Iw] * m_values[Jy];

	inv.m_values[Tz] = -m_values[Ix] * m_values[Jy] * m_values[Tz] +
		m_values[Ix] * m_values[Jz] * m_values[Ty] +
		m_values[Jx] * m_values[Iy] * m_values[Tz] -
		m_values[Jx] * m_values[Iz] * m_values[Ty] -
		m_values[Tx] * m_values[Iy] * m_values[Jz] +
		m_values[Tx] * m_values[Iz] * m_values[Jy];

	inv.m_values[Iw] = -m_values[Iy] * m_values[Jz] * m_values[Kw] +
		m_values[Iy] * m_values[Jw] * m_values[Kz] +
		m_values[Jy] * m_values[Iz] * m_values[Kw] -
		m_values[Jy] * m_values[Iw] * m_values[Kz] -
		m_values[Ky] * m_values[Iz] * m_values[Jw] +
		m_values[Ky] * m_values[Iw] * m_values[Jz];

	inv.m_values[Jw] = m_values[Ix] * m_values[Jz] * m_values[Kw] -
		m_values[Ix] * m_values[Jw] * m_values[Kz] -
		m_values[Jx] * m_values[Iz] * m_values[Kw] +
		m_values[Jx] * m_values[Iw] * m_values[Kz] +
		m_values[Kx] * m_values[Iz] * m_values[Jw] -
		m_values[Kx] * m_values[Iw] * m_values[Jz];

	inv.m_values[Kw] = -m_values[Ix] * m_values[Jy] * m_values[Kw] +
		m_values[Ix] * m_values[Jw] * m_values[Ky] +
		m_values[Jx] * m_values[Iy] * m_values[Kw] -
		m_values[Jx] * m_values[Iw] * m_values[Ky] -
		m_values[Kx] * m_values[Iy] * m_values[Jw] +
		m_values[Kx] * m_values[Iw] * m_values[Jy];

	inv.m_values[Tw] = m_values[Ix] * m_values[Jy] * m_values[Kz] -
		m_values[Ix] * m_values[Jz] * m_values[Ky] -
		m_values[Jx] * m_values[Iy] * m_values[Kz] +
		m_values[Jx] * m_values[Iz] * m_values[Ky] +
		m_values[Kx] * m_values[Iy] * m_values[Jz] -
		m_values[Kx] * m_values[Iz] * m_values[Jy];

	det = m_values[Ix] * inv.m_values[Ix] + m_values[Iy] * inv.m_values[Jx] + m_values[Iz] * inv.m_values[Kx] + m_values[Iw] * inv.m_values[Tx];

	if (det == 0)
		return Mat44(); // Return an identity matrix or handle the error appropriately

	det = 1.0f / det;

	for (int i = 0; i < 16; i++)
		inv.m_values[i] = inv.m_values[i] * det;

	return inv;
}

void Mat44::SetIdentity()
{
	m_values[Ix] = 1; m_values[Jx] = 0; m_values[Kx] = 0; m_values[Tx] = 0;
	m_values[Iy] = 0; m_values[Jy] = 1; m_values[Ky] = 0; m_values[Ty] = 0;
	m_values[Iz] = 0; m_values[Jz] = 0; m_values[Kz] = 1; m_values[Tz] = 0;
	m_values[Iw] = 0; m_values[Jw] = 0; m_values[Kw] = 0; m_values[Tw] = 1;
}

void Mat44::SetElement( int i, int j, float value )
{
	m_values[j * 4 + i] = value;
}

void Mat44::SetTranslation2D( Vec2 const& translationXY )
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetTranslation3D( Vec3 const& translationXYZ )
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJ2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D )
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
}

void Mat44::SetIJT2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY )
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJK3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D )
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
}

void Mat44::SetIJKT3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ )
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJKT4D( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D )
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}


void Mat44::SetScale3D( Vec3 const& scale3D )
{
	SetIJK3D(
		GetIBasis3D().GetNormalized() * scale3D.x,
		GetJBasis3D().GetNormalized() * scale3D.y,
		GetKBasis3D().GetNormalized() * scale3D.z
	);
}

void Mat44::Scale3D( Vec3 const& scale3D )
{
	SetIJK3D(
		GetIBasis3D() * scale3D.x,
		GetJBasis3D() * scale3D.y,
		GetKBasis3D() * scale3D.z
	);
}

void Mat44::Transpose()
{
	Mat44 old = *this;
	m_values[Ix] = old.m_values[Ix];
	m_values[Iy] = old.m_values[Jx];
	m_values[Iz] = old.m_values[Kx];
	m_values[Iw] = old.m_values[Tx];

	m_values[Jx] = old.m_values[Iy];
	m_values[Jy] = old.m_values[Jy];
	m_values[Jz] = old.m_values[Ky];
	m_values[Jw] = old.m_values[Ty];

	m_values[Kx] = old.m_values[Iz];
	m_values[Ky] = old.m_values[Jz];
	m_values[Kz] = old.m_values[Kz];
	m_values[Kw] = old.m_values[Tz];

	m_values[Tx] = old.m_values[Iw];
	m_values[Ty] = old.m_values[Jw];
	m_values[Tz] = old.m_values[Kw];
	m_values[Tw] = old.m_values[Tw];
}

void Mat44::Orthonormalize_IFwd_JLeft_KUp()
{
	Vec3 iBasis = GetIBasis3D();
	Vec3 jBasis = GetJBasis3D();
	Vec3 kBasis = GetKBasis3D();

	iBasis = iBasis.GetNormalized();
	
	jBasis -= GetProjectedOnto3D( jBasis, iBasis );
	jBasis = jBasis.GetNormalized();

	kBasis -= GetProjectedOnto3D( kBasis, iBasis );
	kBasis -= GetProjectedOnto3D( kBasis, jBasis );
	kBasis = kBasis.GetNormalized();

	SetIJK3D( iBasis, jBasis, kBasis );
}

void Mat44::Append( Mat44 const& appendThis )
{
	Mat44 raw = *this;
	m_values[Ix] = raw.m_values[Ix] * appendThis.m_values[Ix] + raw.m_values[Jx] * appendThis.m_values[Iy] + raw.m_values[Kx] * appendThis.m_values[Iz] + raw.m_values[Tx] * appendThis.m_values[Iw];
	m_values[Iy] = raw.m_values[Iy] * appendThis.m_values[Ix] + raw.m_values[Jy] * appendThis.m_values[Iy] + raw.m_values[Ky] * appendThis.m_values[Iz] + raw.m_values[Ty] * appendThis.m_values[Iw];
	m_values[Iz] = raw.m_values[Iz] * appendThis.m_values[Ix] + raw.m_values[Jz] * appendThis.m_values[Iy] + raw.m_values[Kz] * appendThis.m_values[Iz] + raw.m_values[Tz] * appendThis.m_values[Iw];
	m_values[Iw] = raw.m_values[Iw] * appendThis.m_values[Ix] + raw.m_values[Jw] * appendThis.m_values[Iy] + raw.m_values[Kw] * appendThis.m_values[Iz] + raw.m_values[Tw] * appendThis.m_values[Iw];

	m_values[Jx] = raw.m_values[Ix] * appendThis.m_values[Jx] + raw.m_values[Jx] * appendThis.m_values[Jy] + raw.m_values[Kx] * appendThis.m_values[Jz] + raw.m_values[Tx] * appendThis.m_values[Jw];
	m_values[Jy] = raw.m_values[Iy] * appendThis.m_values[Jx] + raw.m_values[Jy] * appendThis.m_values[Jy] + raw.m_values[Ky] * appendThis.m_values[Jz] + raw.m_values[Ty] * appendThis.m_values[Jw];
	m_values[Jz] = raw.m_values[Iz] * appendThis.m_values[Jx] + raw.m_values[Jz] * appendThis.m_values[Jy] + raw.m_values[Kz] * appendThis.m_values[Jz] + raw.m_values[Tz] * appendThis.m_values[Jw];
	m_values[Jw] = raw.m_values[Iw] * appendThis.m_values[Jx] + raw.m_values[Jw] * appendThis.m_values[Jy] + raw.m_values[Kw] * appendThis.m_values[Jz] + raw.m_values[Tw] * appendThis.m_values[Jw];

	m_values[Kx] = raw.m_values[Ix] * appendThis.m_values[Kx] + raw.m_values[Jx] * appendThis.m_values[Ky] + raw.m_values[Kx] * appendThis.m_values[Kz] + raw.m_values[Tx] * appendThis.m_values[Kw];
	m_values[Ky] = raw.m_values[Iy] * appendThis.m_values[Kx] + raw.m_values[Jy] * appendThis.m_values[Ky] + raw.m_values[Ky] * appendThis.m_values[Kz] + raw.m_values[Ty] * appendThis.m_values[Kw];
	m_values[Kz] = raw.m_values[Iz] * appendThis.m_values[Kx] + raw.m_values[Jz] * appendThis.m_values[Ky] + raw.m_values[Kz] * appendThis.m_values[Kz] + raw.m_values[Tz] * appendThis.m_values[Kw];
	m_values[Kw] = raw.m_values[Iw] * appendThis.m_values[Kx] + raw.m_values[Jw] * appendThis.m_values[Ky] + raw.m_values[Kw] * appendThis.m_values[Kz] + raw.m_values[Tw] * appendThis.m_values[Kw];

	m_values[Tx] = raw.m_values[Ix] * appendThis.m_values[Tx] + raw.m_values[Jx] * appendThis.m_values[Ty] + raw.m_values[Kx] * appendThis.m_values[Tz] + raw.m_values[Tx] * appendThis.m_values[Tw];
	m_values[Ty] = raw.m_values[Iy] * appendThis.m_values[Tx] + raw.m_values[Jy] * appendThis.m_values[Ty] + raw.m_values[Ky] * appendThis.m_values[Tz] + raw.m_values[Ty] * appendThis.m_values[Tw];
	m_values[Tz] = raw.m_values[Iz] * appendThis.m_values[Tx] + raw.m_values[Jz] * appendThis.m_values[Ty] + raw.m_values[Kz] * appendThis.m_values[Tz] + raw.m_values[Tz] * appendThis.m_values[Tw];
	m_values[Tw] = raw.m_values[Iw] * appendThis.m_values[Tx] + raw.m_values[Jw] * appendThis.m_values[Ty] + raw.m_values[Kw] * appendThis.m_values[Tz] + raw.m_values[Tw] * appendThis.m_values[Tw];
}

void Mat44::AppendZRotation( float degreesRotationAboutZ )
{
	Append( CreateZRotationDegrees( degreesRotationAboutZ ) );
}

void Mat44::AppendYRotation( float degreesRotationAboutY )
{
	Append( CreateYRotationDegrees( degreesRotationAboutY ) );
}

void Mat44::AppendXRotation( float degreesRotationAboutX )
{
	Append( CreateXRotationDegrees( degreesRotationAboutX ) );
}

void Mat44::AppendTranslation2D( Vec2 const& translationXY )
{
	Append( CreateTranslation2D( translationXY ) );
}

void Mat44::AppendTranslation3D( Vec3 const& translationXYZ )
{
	Append( CreateTranslation3D( translationXYZ ) );
}

void Mat44::AppendScaleUniform2D( float uniformScaleXY )
{
	Append( CreateUniformScale2D( uniformScaleXY ) );
}

void Mat44::AppendScaleUniform3D( float uniformScaleXYZ )
{
	Append( CreateUniformScale3D( uniformScaleXYZ ) );
}

void Mat44::AppendScaleNonUniform2D( Vec2 const& nonUniformScaleXY )
{
	Append( CreateNonUniformScale2D( nonUniformScaleXY ) );
}

void Mat44::AppendScaleNonUniform3D( Vec3 const& nonUniformScaleXYZ )
{
	Append( CreateNonUniformScale3D( nonUniformScaleXYZ ) );
}

Mat44 const Mat44::operator*( Mat44 const& appendThis ) const
{
	Mat44 result;
	result.m_values[Ix] = m_values[Ix] * appendThis.m_values[Ix] + m_values[Jx] * appendThis.m_values[Iy] + m_values[Kx] * appendThis.m_values[Iz] + m_values[Tx] * appendThis.m_values[Iw];
	result.m_values[Iy] = m_values[Iy] * appendThis.m_values[Ix] + m_values[Jy] * appendThis.m_values[Iy] + m_values[Ky] * appendThis.m_values[Iz] + m_values[Ty] * appendThis.m_values[Iw];
	result.m_values[Iz] = m_values[Iz] * appendThis.m_values[Ix] + m_values[Jz] * appendThis.m_values[Iy] + m_values[Kz] * appendThis.m_values[Iz] + m_values[Tz] * appendThis.m_values[Iw];
	result.m_values[Iw] = m_values[Iw] * appendThis.m_values[Ix] + m_values[Jw] * appendThis.m_values[Iy] + m_values[Kw] * appendThis.m_values[Iz] + m_values[Tw] * appendThis.m_values[Iw];

	result.m_values[Jx] = m_values[Ix] * appendThis.m_values[Jx] + m_values[Jx] * appendThis.m_values[Jy] + m_values[Kx] * appendThis.m_values[Jz] + m_values[Tx] * appendThis.m_values[Jw];
	result.m_values[Jy] = m_values[Iy] * appendThis.m_values[Jx] + m_values[Jy] * appendThis.m_values[Jy] + m_values[Ky] * appendThis.m_values[Jz] + m_values[Ty] * appendThis.m_values[Jw];
	result.m_values[Jz] = m_values[Iz] * appendThis.m_values[Jx] + m_values[Jz] * appendThis.m_values[Jy] + m_values[Kz] * appendThis.m_values[Jz] + m_values[Tz] * appendThis.m_values[Jw];
	result.m_values[Jw] = m_values[Iw] * appendThis.m_values[Jx] + m_values[Jw] * appendThis.m_values[Jy] + m_values[Kw] * appendThis.m_values[Jz] + m_values[Tw] * appendThis.m_values[Jw];

	result.m_values[Kx] = m_values[Ix] * appendThis.m_values[Kx] + m_values[Jx] * appendThis.m_values[Ky] + m_values[Kx] * appendThis.m_values[Kz] + m_values[Tx] * appendThis.m_values[Kw];
	result.m_values[Ky] = m_values[Iy] * appendThis.m_values[Kx] + m_values[Jy] * appendThis.m_values[Ky] + m_values[Ky] * appendThis.m_values[Kz] + m_values[Ty] * appendThis.m_values[Kw];
	result.m_values[Kz] = m_values[Iz] * appendThis.m_values[Kx] + m_values[Jz] * appendThis.m_values[Ky] + m_values[Kz] * appendThis.m_values[Kz] + m_values[Tz] * appendThis.m_values[Kw];
	result.m_values[Kw] = m_values[Iw] * appendThis.m_values[Kx] + m_values[Jw] * appendThis.m_values[Ky] + m_values[Kw] * appendThis.m_values[Kz] + m_values[Tw] * appendThis.m_values[Kw];

	result.m_values[Tx] = m_values[Ix] * appendThis.m_values[Tx] + m_values[Jx] * appendThis.m_values[Ty] + m_values[Kx] * appendThis.m_values[Tz] + m_values[Tx] * appendThis.m_values[Tw];
	result.m_values[Ty] = m_values[Iy] * appendThis.m_values[Tx] + m_values[Jy] * appendThis.m_values[Ty] + m_values[Ky] * appendThis.m_values[Tz] + m_values[Ty] * appendThis.m_values[Tw];
	result.m_values[Tz] = m_values[Iz] * appendThis.m_values[Tx] + m_values[Jz] * appendThis.m_values[Ty] + m_values[Kz] * appendThis.m_values[Tz] + m_values[Tz] * appendThis.m_values[Tw];
	result.m_values[Tw] = m_values[Iw] * appendThis.m_values[Tx] + m_values[Jw] * appendThis.m_values[Ty] + m_values[Kw] * appendThis.m_values[Tz] + m_values[Tw] * appendThis.m_values[Tw];
	return result;
}
