#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

IntVec3 const IntVec3::ZERO = IntVec3( 0, 0, 0 );
IntVec3 const IntVec3::ONE = IntVec3( 1, 1, 1 );
IntVec3 const IntVec3::FRONT = IntVec3( 1, 0, 0 );
IntVec3 const IntVec3::BACK = IntVec3( -1, 0, 0 );
IntVec3 const IntVec3::UP = IntVec3( 0, 0, 1 );
IntVec3 const IntVec3::DOWN = IntVec3( 0, 0, -1 );
IntVec3 const IntVec3::LEFT = IntVec3( 0, 1, 0 );
IntVec3 const IntVec3::RIGHT = IntVec3( 0, -1, 0 );

IntVec3::IntVec3( const IntVec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

IntVec3::IntVec3( std::string data )
{
	SetFromText( data );
}

IntVec3::IntVec3( int initialX, int initialY, int initialZ )
{
	x = initialX;
	y = initialY;
	z = initialZ;
}

IntVec2 IntVec3::GetXY() const
{
	return IntVec2( x, y );
}

float IntVec3::GetLength() const
{
	return sqrtf( static_cast<float>(GetLengthSquared()) );
}

int IntVec3::GetTaxicabLength() const
{
	return abs( x ) + abs( y ) + abs( z );
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

void IntVec3::SetFromText( std::string text )
{
	Strings elems = Split( text, ',' );
	x = stoi( elems[0] );
	y = stoi( elems[1] );
	z = stoi( elems[2] );
}

bool const IntVec3::operator==( const IntVec3& compare ) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}

bool const IntVec3::operator!=( const IntVec3& compare ) const
{
	return !(x == compare.x && y == compare.y && z == compare.z);
}

IntVec3 const IntVec3::operator+( const IntVec3& vecToAdd ) const
{
	return IntVec3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z );
}

IntVec3 const IntVec3::operator-( const IntVec3& vecToSubtract ) const
{
	return IntVec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
}

//-----------------------------------------------------------------------------------------------
void IntVec3::operator+=( const IntVec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void IntVec3::operator-=( const IntVec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


void IntVec3::operator=( const IntVec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}
