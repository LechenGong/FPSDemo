#pragma once

#include <cmath>
#include <string>

#include "Engine/Math/IntVec2.hpp"

struct IntVec3
{
public: 
	int x = 0;
	int y = 0;
	int z = 0;
public:
	// Construction/Destruction
	~IntVec3() {}												// destructor (do nothing)
	IntVec3() {}												// default constructor (do nothing)
	IntVec3( const IntVec3& copyFrom );							// copy constructor (from another IntVec3)
	IntVec3( std::string data );
	explicit IntVec3( int initialX, int initialY, int initialZ );		// explicit constructor (from x, y)
														//Accessors (const methods)
	IntVec2 GetXY() const;
	float GetLength() const;
	int	 GetTaxicabLength() const;
	int	 GetLengthSquared() const;

	void SetFromText( std::string text );

	// Operators (const)
	bool const operator==( const IntVec3& compare ) const;
	bool const operator!=( const IntVec3& compare ) const;
	IntVec3 const operator+( const IntVec3& ) const;
	IntVec3 const operator-( const IntVec3& ) const;
	void operator+=( const IntVec3& );
	void operator-=( const IntVec3& );
	void operator=( const IntVec3& );

	static const IntVec3 ZERO;
	static const IntVec3 ONE;
	static const IntVec3 FRONT;
	static const IntVec3 BACK;
	static const IntVec3 UP;
	static const IntVec3 DOWN;
	static const IntVec3 LEFT;
	static const IntVec3 RIGHT;
};