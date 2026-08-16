#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

IntVec2 const IntVec2::ZERO = IntVec2( 0, 0 );
IntVec2 const IntVec2::ONE = IntVec2( 1, 1 );
IntVec2 const IntVec2::UP = IntVec2( 0, 1 );
IntVec2 const IntVec2::DOWN = IntVec2( 0, -1 );
IntVec2 const IntVec2::LEFT = IntVec2( -1, 0 );
IntVec2 const IntVec2::RIGHT = IntVec2( 1, 0 );

IntVec2::IntVec2( const IntVec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

IntVec2::IntVec2( std::string data )
{
	SetFromText( data );
}

IntVec2::IntVec2( int initialX, int initialY )
{
	x = initialX;
	y = initialY;
}

float IntVec2::GetLength() const
{
	return sqrtf( static_cast<float>(GetLengthSquared()) );
}

int IntVec2::GetTaxicabLength() const
{
	return abs( x ) + abs( y );
}

int IntVec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float IntVec2::GetOrientationRadians() const
{
	return atan2f( static_cast<float>(y), static_cast<float>(x) );
}

float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees( static_cast<float>(y), static_cast<float>(x) );
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2( -y, x );
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2( y, -x );
}

void IntVec2::Rotate90Degrees()
{
	int temp = x;
	x = -y;
	y = temp;
}

void IntVec2::RotateMinus90Degrees()
{
	int temp = x;
	x = y;
	y = -temp;
}

void IntVec2::SetFromText( std::string text )
{
	Strings elems = Split( text, ',' );
	x = stoi( elems[0] );
	y = stoi( elems[1] );
}

bool const IntVec2::operator==( const IntVec2& compare ) const
{
	return x == compare.x && y == compare.y;
}

bool const IntVec2::operator!=( const IntVec2& compare ) const
{
	return !(x == compare.x && y == compare.y);
}

IntVec2 const IntVec2::operator+( const IntVec2& vecToAdd ) const
{
	return IntVec2( x + vecToAdd.x, y + vecToAdd.y );
}

IntVec2 const IntVec2::operator-( const IntVec2& vecToSubtract ) const
{
	return IntVec2( x - vecToSubtract.x, y - vecToSubtract.y );
}

//-----------------------------------------------------------------------------------------------
void IntVec2::operator+=( const IntVec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator-=( const IntVec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


void IntVec2::operator=( const IntVec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}
