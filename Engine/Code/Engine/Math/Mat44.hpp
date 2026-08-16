#pragma once

#include <string>

struct Vec2;
struct Vec3;
struct Vec4;
class Quat;
enum class BillboardType;

#include "Engine/Math/Vec4.hpp"

//   Column-Major
// | Ix  Jx  Kx  Tx	|
// | Iy  Jy  Ky  Ty	|
// | Iz  Jz  Kz  Tz	|
// | Iw  Jw  Kw  Tw	|
struct Mat44
{
	enum { Ix, Iy, Iz, Iw, Jx, Jy, Jz, Jw, Kx, Ky, Kz, Kw, Tx, Ty, Tz, Tw };
	float m_values[16];

	Mat44();
	explicit Mat44( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D );
	explicit Mat44( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D );
	explicit Mat44( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D = Vec4( 0.f, 0.f, 0.f, 1.f ) );
	explicit Mat44( float const* sixteenValueBasisMajor );
	explicit Mat44( Vec3 const& translation3D, Quat const& rotation3D, Vec3 const& scale3D );

	std::string ToString() const;
	void FromString( std::string const& str );

	static Mat44 const CreateTranslation2D( Vec2 const& translationXY );
	static Mat44 const CreateTranslation3D( Vec3 const& translationXYZ );
	static Mat44 const CreateUniformScale2D( float uniformScaleXY );
	static Mat44 const CreateUniformScale3D( float uniformScaleXYZ );
	static Mat44 const CreateNonUniformScale2D( Vec2 const& nonUniformScaleXY );
	static Mat44 const CreateNonUniformScale3D( Vec3 const& nonUniformScaleXYZ );
	static Mat44 const CreateZRotationDegrees( float rotationDegreesAboutZ );
	static Mat44 const CreateYRotationDegrees( float rotationDegreesAboutY );
	static Mat44 const CreateXRotationDegrees( float rotationDegreesAboutX );
	static Mat44 const CreateOrthoProjection( float left, float right, float bottom, float top, float zNear, float zFar );
	static Mat44 const CreatePerspectiveProjection( float fovDegrees, float aspect, float zNear, float zFar );
	static Mat44 const Interpolate( Mat44 const& matrixA, Mat44 const& matrixB, float factor );

	Vec2 const TransformVectorQuantity2D( Vec2 const& vectorQuantityXY ) const;
	Vec3 const TransformVectorQuantity3D( Vec3 const& vectorQuantityXY ) const;
	Vec2 const TransformPosition2D( Vec2 const& positionXY ) const;
	Vec3 const TransformPosition3D( Vec3 const& position3D ) const;
	Vec4 const TransformPosition4D( Vec4 const& position4D ) const;
	Vec4 const TransformHomogeneous3D( Vec4 const& homogeneousPoint3D ) const;

	float const GetElement( int i, int j ) const;
	float* GetAsFloatArray();
	float const* GetAsFloatArray() const;
	Vec2 const GetIBasis2D() const;
	Vec2 const GetJBasis2D() const;
	Vec2 const GetTranslation2D() const;
	Vec3 const GetIBasis3D() const;
	Vec3 const GetJBasis3D() const;
	Vec3 const GetKBasis3D() const;
	Vec3 const GetTranslation3D() const;
	Vec4 const GetIBasis4D() const;
	Vec4 const GetJBasis4D() const;
	Vec4 const GetKBasis4D() const;
	Vec4 const GetTranslation4D() const;
	Vec3 const GetScale3D() const;
	Mat44 const GetOrthonormalInverse() const;
	Mat44 const GetInverse() const;

	void SetIdentity();
	void SetElement( int i, int j, float value );
	void SetTranslation2D( Vec2 const& translationXY );
	void SetTranslation3D( Vec3 const& translationXYZ );
	void SetIJ2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D );
	void SetIJT2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY );
	void SetIJK3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D );
	void SetIJKT3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ );
	void SetIJKT4D( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D );
	void SetScale3D( Vec3 const& scale3D );
	void Scale3D( Vec3 const& scale3D );
	void Transpose();
	void Orthonormalize_IFwd_JLeft_KUp();

	void Append( Mat44 const& appendThis ); // A.Append(B) -> A = A x B
	void AppendZRotation( float degreesRotationAboutZ );
	void AppendYRotation( float degreesRotationAboutY );
	void AppendXRotation( float degreesRotationAboutX );
	void AppendTranslation2D( Vec2 const& translationXY );
	void AppendTranslation3D( Vec3 const& translationXYZ );
	void AppendScaleUniform2D( float uniformScaleXY );
	void AppendScaleUniform3D( float uniformScaleXYZ );
	void AppendScaleNonUniform2D( Vec2 const& nonUniformScaleXY );
	void AppendScaleNonUniform3D( Vec3 const& nonUniformScaleXYZ );

	Mat44 const operator*( Mat44 const& appendThis ) const;
};
