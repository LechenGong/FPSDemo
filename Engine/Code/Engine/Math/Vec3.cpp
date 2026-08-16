#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

Vec3 const Vec3::ZERO = Vec3( 0.f, 0.f, 0.f );
Vec3 const Vec3::ONE = Vec3( 1.f, 1.f, 1.f );
Vec3 const Vec3::FORWARD = Vec3( 1.f, 0.f, 0.f );
Vec3 const Vec3::LEFT = Vec3( 0.f, 1.f, 0.f );
Vec3 const Vec3::UP = Vec3( 0.f, 0.f, 1.f );

Vec3::Vec3( std::string data )
{
	SetFromText( data );
}

Vec3::Vec3( float initialX, float initialY, float initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}

Vec3::Vec3( Vec2 vec2 )
	: x( vec2.x )
	, y( vec2.y )
	, z( 0.f )
{
}

Vec3 const Vec3::MakeFromPolarRadians( float latitudeRadians, float longtitudeRadians, float length/* = 1.0f*/ )
{
	return Vec3( length * cosf( longtitudeRadians ) * cosf( latitudeRadians ), length * sinf( longtitudeRadians ) * cosf( latitudeRadians ), length * -sinf( latitudeRadians ) );
}

Vec3 const Vec3::MakeFromPolarDegrees( float latitudeDegrees, float longtitudeDegrees, float length/* = 1.0f*/ )
{
	return Vec3( length * CosDegrees( longtitudeDegrees ) * CosDegrees( latitudeDegrees ), length * SinDegrees( longtitudeDegrees ) * CosDegrees( latitudeDegrees ), length * -SinDegrees( latitudeDegrees ) );
}

Vec2 Vec3::GetXY() const
{
	return Vec2( x, y );
}

float Vec3::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}

float Vec3::GetLengthXY() const
{
	return sqrtf( GetLengthXYSquared() );
}

float Vec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

float Vec3::GetLengthXYSquared() const
{
	return x * x + y * y;
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}

float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees( y, x );
}

Vec3 const Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	float newRadians = GetAngleAboutZRadians() + deltaRadians;
	float length = sqrtf( x * x + y * y );
	return Vec3( cosf( newRadians ) * length, sinf( newRadians ) * length, z );
}

Vec3 const Vec3::GetRotatedAboutZDegrees( float deltaDegrees ) const
{
	float newDegrees = GetAngleAboutZDegrees() + deltaDegrees;
	float length = sqrtf( x * x + y * y );
	return Vec3( CosDegrees( newDegrees ) * length, SinDegrees( newDegrees ) * length, z );
}

Vec3 const Vec3::GetClamped( float maxLength ) const
{
	float length = this->GetLength();
	return (length <= maxLength) ? *this : (this->GetNormalized() * maxLength);
}

Vec3 const Vec3::GetNormalized() const
{
	if (x * x + y * y + z * z == 1.f)
		return Vec3( x, y, z );

	float length = this->GetLength();
	if (length > 0)
	{
		float scale = 1.f / length;
		return Vec3( x * scale, y * scale, z * scale );
	}
	return Vec3( x, y, z );
}

void Vec3::SetFromText( std::string text )
{
	Strings elems;
	if (text.find( ',' ) != std::string::npos)
		elems = Split( text, ',' );
	else if (text.find( ' ' ) != std::string::npos)
		elems = Split( text, ' ', true );
	else
		return;
	x = stof( elems[0] );
	y = stof( elems[1] );
	z = stof( elems[2] );
}

std::string Vec3::ToString() const
{
	std::string str = "";
	str += std::to_string( x );
	str += ' ';
	str += std::to_string( y );
	str += ' ';
	str += std::to_string( z );
	return str;
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + ( const Vec3& vecToAdd ) const
{
	return Vec3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-( const Vec3& vecToSubtract ) const
{
	return Vec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3( -x, -y, -z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( float uniformScale ) const
{
	return Vec3( x * uniformScale, y * uniformScale, z * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( const Vec3& vecToMultiply ) const
{
	return Vec3( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/( float inverseScale ) const
{
	return Vec3( x / inverseScale, y / inverseScale, z / inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=( const Vec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=( const Vec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

//-----------------------------------------------------------------------------------------------
const Vec3 operator*( float uniformScale, const Vec3& vecToScale )
{
	return Vec3( uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z );
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==( const Vec3& compare ) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=( const Vec3& compare ) const
{
	return !(x == compare.x && y == compare.y && z == compare.z);
}

float& Vec3::operator[] ( size_t index )
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	default:
		return z;
	}
}

const float& Vec3::operator[] ( size_t index ) const
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	default:
		return z;
	}
}
