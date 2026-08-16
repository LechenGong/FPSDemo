#pragma once

#include <cmath>
#include <string>

struct IntVec2
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	int x = 0;
	int y = 0;
public:
	// Construction/Destruction
	~IntVec2() {}												// destructor (do nothing)
	IntVec2() {}												// default constructor (do nothing)
	IntVec2( const IntVec2& copyFrom );							// copy constructor (from another IntVec2)
	IntVec2( std::string data );
	explicit IntVec2( int initialX, int initialY );		// explicit constructor (from x, y)
														//Accessors (const methods)
	float GetLength() const;
	int	 GetTaxicabLength() const;
	int	 GetLengthSquared() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;

	//Mutators(non-const methods)
	void Rotate90Degrees();
	void RotateMinus90Degrees();

	void SetFromText( std::string text );

	// Operators (const)
	bool const operator==( const IntVec2& compare ) const;
	bool const operator!=( const IntVec2& compare ) const;
	IntVec2 const operator+( const IntVec2& ) const;
	IntVec2 const operator-( const IntVec2& ) const;
	void operator+=( const IntVec2& );
	void operator-=( const IntVec2& );
	void operator=( const IntVec2& );

	static const IntVec2 ZERO;
	static const IntVec2 ONE;
	static const IntVec2 UP;
	static const IntVec2 DOWN;
	static const IntVec2 LEFT;
	static const IntVec2 RIGHT;
};