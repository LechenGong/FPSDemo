#include "Engine/Math/Quat.hpp"
#include "Engine/Math/MathUtils.hpp"

Quat const Quat::IDENTITY = Quat( 0.f, 0.f, 0.f, 1.f );

Quat::Quat()
	: x( 0 ), y( 0 ), z( 0 ), w( 1 )
{
}

Quat::Quat( Quat const& quat )
	: x( quat.x ), y( quat.y ), z( quat.z ), w( quat.w )
{
}

Quat::Quat( float pX, float pY, float pZ, float pW )
	: x( pX ), y( pY ), z( pZ ), w( pW )
{
}

Quat::Quat( Vec3 const& pAxis, float pDegree )
{
	Vec3 normalAxis = pAxis.GetNormalized();
	float halfDegree = pDegree * 0.5f;
	float s = SinDegrees( halfDegree );
	x = normalAxis.x * s;
	y = normalAxis.y * s;
	z = normalAxis.z * s;
	w = CosDegrees( halfDegree );
}

Quat::Quat( Vec4 const& pAxis, float pDegree )
{
	float halfDegree = pDegree * 0.5f;
	float s = SinDegrees( halfDegree );
	x = pAxis.x * s;
	y = pAxis.y * s;
	z = pAxis.z * s;
	w = CosDegrees( halfDegree );
}

Quat::Quat( Mat44 const& rotationMatrix )
{
	float trace = rotationMatrix.GetElement( 0, 0 ) + rotationMatrix.GetElement( 1, 1 ) + rotationMatrix.GetElement( 2, 2 );
	if (trace > 0) {
		float s = 0.5f / sqrt( trace + 1.0f );
		w = 0.25f / s;
		x = (rotationMatrix.GetElement( 2, 1 ) - rotationMatrix.GetElement( 1, 2 )) * s;
		y = (rotationMatrix.GetElement( 0, 2 ) - rotationMatrix.GetElement( 2, 0 )) * s;
		z = (rotationMatrix.GetElement( 1, 0 ) - rotationMatrix.GetElement( 0, 1 )) * s;
	}
	else {
		if (rotationMatrix.GetElement( 0, 0 ) > rotationMatrix.GetElement( 1, 1 ) && rotationMatrix.GetElement( 0, 0 ) > rotationMatrix.GetElement( 2, 2 )) 
		{
			float s = 2.0f * sqrt( 1.0f + rotationMatrix.GetElement( 0, 0 ) - rotationMatrix.GetElement( 1, 1 ) - rotationMatrix.GetElement( 2, 2 ) );
			w = (rotationMatrix.GetElement( 2, 1 ) - rotationMatrix.GetElement( 1, 2 )) / s;
			x = 0.25f * s;
			y = (rotationMatrix.GetElement( 0, 1 ) + rotationMatrix.GetElement( 1, 0 )) / s;
			z = (rotationMatrix.GetElement( 0, 2 ) + rotationMatrix.GetElement( 2, 0 )) / s;
		}
		else if (rotationMatrix.GetElement( 1, 1 ) > rotationMatrix.GetElement( 2, 2 )) 
		{
			float s = 2.0f * sqrt( 1.0f + rotationMatrix.GetElement( 1, 1 ) - rotationMatrix.GetElement( 0, 0 ) - rotationMatrix.GetElement( 2, 2 ) );
			w = (rotationMatrix.GetElement( 0, 2 ) - rotationMatrix.GetElement( 2, 0 )) / s;
			x = (rotationMatrix.GetElement( 0, 1 ) + rotationMatrix.GetElement( 1, 0 )) / s;
			y = 0.25f * s;
			z = (rotationMatrix.GetElement( 1, 2 ) + rotationMatrix.GetElement( 2, 1 )) / s;
		}
		else 
		{
			float s = 2.0f * sqrt( 1.0f + rotationMatrix.GetElement( 2, 2 ) - rotationMatrix.GetElement( 0, 0 ) - rotationMatrix.GetElement( 1, 1 ) );
			w = (rotationMatrix.GetElement( 1, 0 ) - rotationMatrix.GetElement( 0, 1 )) / s;
			x = (rotationMatrix.GetElement( 0, 2 ) + rotationMatrix.GetElement( 2, 0 )) / s;
			y = (rotationMatrix.GetElement( 1, 2 ) + rotationMatrix.GetElement( 2, 1 )) / s;
			z = 0.25f * s;
		}
	}
}

Quat::Quat( EulerAngles const& pEulerAngles )
{
	float cy = CosDegrees( pEulerAngles.m_yawDegrees * 0.5f );
	float sy = SinDegrees( pEulerAngles.m_yawDegrees * 0.5f );
	float cp = CosDegrees( pEulerAngles.m_pitchDegrees * 0.5f );
	float sp = SinDegrees( pEulerAngles.m_pitchDegrees * 0.5f );
	float cr = CosDegrees( pEulerAngles.m_rollDegrees * 0.5f );
	float sr = SinDegrees( pEulerAngles.m_rollDegrees * 0.5f );

	w = cr * cp * cy + sr * sp * sy;
	x = sr * cp * cy - cr * sp * sy;
	y = cr * sp * cy + sr * cp * sy;
	z = cr * cp * sy - sr * sp * cy;
}

Quat::~Quat()
{
}

Quat& Quat::operator=( Quat const& other )
{
	w = other.w;
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

bool const Quat::operator==( Quat const& other ) const
{
	return fabs( x - other.x ) < 0.0001f && fabs( y - other.y ) < 0.0001f &&
		fabs( z - other.z ) < 0.0001f && fabs( w - other.w ) < 0.0001f;
}

Quat Quat::operator+( Quat const& other ) const
{
	return Quat( x + other.x, y + other.y, z + other.z, w + other.w );
}

Quat Quat::operator-( Quat const& other ) const
{
	return Quat(x - other.x, y - other.y, z - other.z, w - other.w);
}

Quat Quat::operator*( Quat const& other ) const
{
	return Quat(
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w,
		w * other.w - x * other.x - y * other.y - z * other.z
	);
}

Quat Quat::operator/( Quat const& other ) const
{
	return *this * other.GetInversed();
}

Quat& Quat::operator+=( Quat const& other )
{
	x = x + other.x;
	y = y + other.y;
	z = z + other.z;
	w = w + other.w;
	return *this;
}

Quat& Quat::operator-=( Quat const& other )
{
	x = x - other.x;
	y = y - other.y;
	z = z - other.z;
	w = w - other.w;
	return *this;
}

Quat& Quat::operator*=( Quat const& other )
{
	*this = *this * other;
	return *this;
}

Quat& Quat::operator/=( Quat const& other )
{
	*this = *this * other.GetInversed();
	return *this;
}

Quat Quat::operator*( float scalar ) const
{
	return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
}

Quat Quat::operator/( float scalar ) const
{
	return Quat(x / scalar, y / scalar, z / scalar, w / scalar);
}

Quat& Quat::operator*=( float scalar )
{
	x = x * scalar;
	y = y * scalar;
	z = z * scalar;
	w = w * scalar;
	return *this;
}

Quat& Quat::operator/=( float scalar )
{
	x = x / scalar;
	y = y / scalar;
	z = z / scalar;
	w = w / scalar;
	return *this;
}

void Quat::Set( float pX, float pY, float pZ, float pW )
{
	x = pX;
	y = pY;
	z = pZ;
	w = pW;
}

float Quat::DotProduct( Quat const& other ) const
{
	return x * other.x + y * other.y + z * other.z + w * other.w;
}

Quat Quat::GetConjugated() const
{
	return Quat( -x, -y, -z, w );
}

Quat Quat::GetInversed() const
{
	return GetConjugated() * (1.f / GetLength());
}

Quat Quat::GetNormalized() const
{
	float length = GetLength();
	return Quat( x / length, y / length, z / length, w / length );
}

float Quat::GetLength() const
{
	return sqrt( x * x + y * y + z * z + w * w );
}

void Quat::Conjugate()
{
	x = -x;
	y = -y;
	z = -z;
}

void Quat::Inverse()
{
	float lengthInv = 1.f / GetLength();
	x = -x * lengthInv;
	y = -y * lengthInv;
	z = -z * lengthInv;
	w = w * lengthInv;
}

void Quat::Normalize()
{
	float length = GetLength();
	x = x / length;
	y = y / length;
	z = z / length;
	w = w / length;
}

Vec3 Quat::Rotate( Vec3 const& vec3 ) const
{
	Quat result = *this * Quat( vec3.x, vec3.y, vec3.z, 0.f ) * GetInversed();
	return Vec3( result.x, result.y, result.z );
}

Mat44 Quat::ToRotationMatrix() const
{
	Mat44 result;
	result.SetElement( 0, 0, 1.0f - 2.0f * (y * y + z * z) );
	result.SetElement( 0, 1, 2.0f * (x * y - w * z) );
	result.SetElement( 0, 2, 2.0f * (x * z + w * y) );
	result.SetElement( 1, 0, 2.0f * (x * y + w * z) );
	result.SetElement( 1, 1, 1.0f - 2.0f * (x * x + z * z) );
	result.SetElement( 1, 2, 2.0f * (y * z - w * x) );
	result.SetElement( 2, 0, 2.0f * (x * z - w * y) );
	result.SetElement( 2, 1, 2.0f * (y * z + w * x) );
	result.SetElement( 2, 2, 1.0f - 2.0f * (x * x + y * y) );
	return result;
}

EulerAngles Quat::ToEulerAngles() const
{
	EulerAngles result;
	float sinR_cosP = 2.f * (w * z + x * y);
	float cosR_cosP = 1.f - 2.f * (y * y + z * z);
	result.m_yawDegrees = Atan2Degrees( sinR_cosP, cosR_cosP );

	float sinP = 2.f * (w * y - z * x);
	if (fabs( sinP ) >= 1.f)
	{
		result.m_pitchDegrees = copysign( 90.f, sinP );
	}
	else
	{
		result.m_pitchDegrees = ASinDegrees( sinP );
	}

	float sinY_cosP = 2.f * (w * x + y * z);
	float cosY_cosP = 1.f - 2.f * (x * x + y * y);
	result.m_rollDegrees = Atan2Degrees( sinY_cosP, cosY_cosP );

	return result;
}

Quat Quat::FromEulerAngles( EulerAngles const& pEulerAngles )
{
	return Quat( pEulerAngles );
}

Quat Quat::SLERP( Quat const& start, Quat const& end, float t )
{
	float dotP = start.DotProduct( end );
	Quat correctedEnd = dotP < 0.f ? end * -1.f : end; // if in the opposite direction
	dotP = fabs( dotP );

	if (dotP > 0.9995f) // degenerate to nlerp if too close
	{
		return NLERP( start, end, t );
	}

	float theta = acos( dotP );
	float sinTheta = sin( theta );
	float weightedStart = sin( (1.f - t) * theta ) / sinTheta;
	float weightedEnd = sin( t * theta ) / sinTheta;

	return start * weightedStart + correctedEnd * weightedEnd;
}

Quat Quat::NLERP( Quat const& start, Quat const& end, float t )
{
	Quat result = start * (1.f - t) + end * t;
	return result.GetNormalized();
}

bool Quat::IsNormaliazed() const
{
	return false;
}

float Quat::GetAngle() const
{
	return 0.0f;
}

Vec3 Quat::GetAxis() const
{
	return Vec3();
}

Vec3 Quat::GetForwardVector() const
{
	return Vec3(
		1.0f - 2.0f * (y * y + z * z),
		2.0f * (x * y + w * z),
		2.0f * (x * z - w * y)
	);
}

Vec3 Quat::GetUpVector() const
{
	return Vec3(
		2.0f * (x * z + w * y),
		2.0f * (y * z - w * x),
		1.0f - 2.0f * (x * x + y * y)
	);
}

Vec3 Quat::GetLeftVector() const
{
	return Vec3(
		2.0f * (x * y - w * z),
		1.0f - 2.0f * (x * x + z * z),
		2.0f * (x * z + w * y)
	);
}