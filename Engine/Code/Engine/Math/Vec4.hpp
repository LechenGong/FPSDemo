#pragma once


//-----------------------------------------------------------------------------------------------
struct Vec4
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;
public:
	// Construction/Destruction
	~Vec4() {}												// destructor (do nothing)
	Vec4() {}												// default constructor (do nothing)
	Vec4( const Vec4& copyFrom );							// copy constructor (from another vec2)
	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );		// explicit constructor (from x, y)

	bool		operator==( const Vec4& compare ) const;		// Vec4 == Vec4
	bool		operator!=( const Vec4& compare ) const;		// Vec4 != Vec4
	const Vec4	operator+( const Vec4& vecToAdd ) const;		// Vec4 + Vec4
	const Vec4	operator-( const Vec4& vecToSubtract ) const;	// Vec4 - Vec4
	const Vec4	operator-() const;								// -Vec4, i.e. "unary negation"
	const Vec4	operator*( float uniformScale ) const;			// Vec4 * float
	const Vec4	operator*( const Vec4& vecToMultiply ) const;	// Vec4 * Vec4
	const Vec4	operator/( float inverseScale ) const;			// Vec4 / float

	void		operator+=( const Vec4& vecToAdd );				// Vec4 += Vec4
	void		operator-=( const Vec4& vecToSubtract );		// Vec4 -= Vec4
	void		operator*=( const float uniformScale );			// Vec4 *= float
	void		operator/=( const float uniformDivisor );		// Vec4 /= float
	void		operator=( const Vec4& copyFrom );				// Vec4 = Vec4

	friend Vec4 const operator*( float uniformScale, Vec4 const& vecToScale ); // float * Vec4
};
