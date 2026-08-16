#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
//#include "Engine/Core/EngineCommon.hpp"

Vec2 const Vec2::ZERO = Vec2( 0.f, 0.f );
Vec2 const Vec2::HALF = Vec2( 0.5f, 0.5f );
Vec2 const Vec2::ONE = Vec2( 1.f, 1.f );
Vec2 const Vec2::UP = Vec2( 1.f, 0.f );
Vec2 const Vec2::DOWN = Vec2( -1.f, 0.f );
Vec2 const Vec2::LEFT = Vec2( 0.f, -1.f );
Vec2 const Vec2::RIGHT = Vec2( 0.f, 1.f );

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}

Vec2::Vec2( std::string data )
{
	SetFromText( data );
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

Vec2 const Vec2::MakeFromPolarDegrees( float orientationDegrees, float length /*= 1.f */ )
{
	return Vec2( CosDegrees( orientationDegrees ) * length, SinDegrees( orientationDegrees ) * length );
}

Vec2 const Vec2::MakeFromPolarRadians( float orientationRadians, float length /*= 1.f */ )
{
	return Vec2( cosf( orientationRadians ) * length, sinf( orientationRadians ) * length );
}

float Vec2::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}

float Vec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float Vec2::GetOrientationDegrees() const
{
	return Atan2Degrees( y, x );
}

float Vec2::GetOrientationRadians() const
{
	return atan2f( y, x );
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2( -y, x );
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2( y, -x );
}

Vec2 const Vec2::GetRotatedRadians( float deltaRadians ) const
{
	float newRadians = GetOrientationRadians() + deltaRadians;
	float length = GetLength();
	return Vec2( cosf( newRadians ) * length, sinf( newRadians ) * length );
}

Vec2 const Vec2::GetRotatedDegrees( float deltaDegrees ) const
{
	float newDegrees = GetOrientationDegrees() + deltaDegrees;
	float length = GetLength();
	return Vec2( CosDegrees( newDegrees ) * length, SinDegrees( newDegrees ) * length );
}

Vec2 const Vec2::GetClamped( float maxLength ) const
{
	float length = this->GetLength();
	return (length <= maxLength) ? *this : (this->GetNormalized() * maxLength);
}

Vec2 const Vec2::GetNormalized() const
{
	if (x * x + y * y == 1.f)
		return Vec2( x, y );

	float length = this->GetLength();
	if (length > 0)
	{
		float scale = 1.f / length;
		return Vec2( x * scale, y * scale );
	}
	return Vec2( x, y );
}

Vec2 const Vec2::GetReflected( Vec2 const& reflectOffNormal ) const
{
	float dotP2D = DotProduct2D( reflectOffNormal, *this );
	Vec2 inVectorN = dotP2D * reflectOffNormal;
	Vec2 inVectorT = *this - inVectorN;
	Vec2 outVector = inVectorT + -inVectorN;
	return outVector;
}

void Vec2::SetOrientationRadians( float newOrientationRadians )
{
	float length = GetLength();
	x = cosf( newOrientationRadians ) * length;
	y = sinf( newOrientationRadians ) * length;
}

void Vec2::SetOrientationDegrees( float newOrientationDegrees )
{
	float length = GetLength();
	x = CosDegrees( newOrientationDegrees ) * length;
	y = SinDegrees( newOrientationDegrees ) * length;
}

void Vec2::SetPolarRadians( float newOrientationRadians, float newLength )
{
	x = cosf( newOrientationRadians ) * newLength;
	y = sinf( newOrientationRadians ) * newLength;
}

void Vec2::SetPolarDegrees( float newOrientationDegrees, float newLength )
{
	x = CosDegrees( newOrientationDegrees ) * newLength;
	y = SinDegrees( newOrientationDegrees ) * newLength;
}

void Vec2::Rotate90Degrees()
{
	*this = Vec2( -y, x );
}

void Vec2::RotateMinus90Degrees()
{
	*this = Vec2( y, -x );
}

void Vec2::RotateRadians( float deltaRadians )
{
	*this = GetRotatedRadians( deltaRadians );
}

void Vec2::RotateDegrees( float deltaDegrees )
{
	*this = GetRotatedDegrees( deltaDegrees );
}

void Vec2::SetLength( float newLength )
{
	*this = GetNormalized() * newLength;
}

void Vec2::ClampLength( float maxLength )
{
	*this = GetClamped( maxLength );
}

void Vec2::Normalize()
{
	*this = GetNormalized();
}

void Vec2::Reflect( Vec2 const& reflectOffNormal )
{
	*this = GetReflected( reflectOffNormal );
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float prevLength = GetLength();
	Normalize();
	return prevLength;
}

void Vec2::SetFromText( std::string text )
{
	Strings elems;
	if (text.find( ',' ) != std::string::npos)
		elems = Split( text, ',' );
	else if (text.find( ' ' ) != std::string::npos)
		elems = Split( text, ' ', true );
	else if (text.find( '~' ) != std::string::npos)
		elems = Split( text, '~' );
	else
		return;
	x = stof( elems[0] );
	y = stof( elems[1] );
}

std::string Vec2::ToString() const
{
	std::string str = "";
	str += std::to_string( x );
	str += ' ';
	str += std::to_string( y );
	return str;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( x / inverseScale, y / inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( uniformScale * vecToScale.x, uniformScale * vecToScale.y );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return !(x == compare.x && y == compare.y);
}
