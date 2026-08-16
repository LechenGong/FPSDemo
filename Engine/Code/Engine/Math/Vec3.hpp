#pragma once

#include <string>

#include "Engine/Math/Vec2.hpp"

//-----------------------------------------------------------------------------------------------
struct Vec3
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

public:
	Vec3() = default;
	Vec3( std::string data );
	explicit Vec3( float initialX, float initialY, float initialZ );
	explicit Vec3( Vec2 vec2 );

	static Vec3 const MakeFromPolarRadians( float latitudeRadians, float longtitudeRadians, float length = 1.0f );
	static Vec3 const MakeFromPolarDegrees( float latitudeDegrees, float longtitudeDegrees, float length = 1.0f );

	Vec2 GetXY() const;
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;
	Vec3 const GetRotatedAboutZRadians( float deltaRadians ) const;
	Vec3 const GetRotatedAboutZDegrees( float deltaDegrees ) const;
	Vec3 const GetClamped( float maxLength ) const;
	Vec3 const GetNormalized() const;

	void SetFromText( std::string text );
	std::string ToString() const;

	bool		operator==( const Vec3& compare ) const;		// Vec3 == Vec3
	bool		operator!=( const Vec3& compare ) const;		// Vec3 != Vec3
	const Vec3	operator+( const Vec3& vecToAdd ) const;		// Vec3 + Vec3
	const Vec3	operator-( const Vec3& vecToSubtract ) const;	// Vec3 - Vec3
	const Vec3	operator-() const;								// -Vec3, i.e. "unary negation"
	const Vec3	operator*( float uniformScale ) const;			// Vec3 * float
	const Vec3	operator*( const Vec3& vecToMultiply ) const;	// Vec3 * Vec3
	const Vec3	operator/( float inverseScale ) const;			// Vec3 / float

	void		operator+=( const Vec3& vecToAdd );				// Vec3 += Vec3
	void		operator-=( const Vec3& vecToSubtract );		// Vec3 -= Vec3
	void		operator*=( const float uniformScale );			// Vec3 *= float
	void		operator/=( const float uniformDivisor );		// Vec3 /= float
	void		operator=( const Vec3& copyFrom );				// Vec3 = Vec3

	friend Vec3 const operator*( float uniformScale, Vec3 const& vecToScale ); // float * vec3

	float& operator[] ( size_t index );
	const float& operator[] ( size_t index ) const;

public:
	static const Vec3 ZERO;
	static const Vec3 ONE;
	static const Vec3 FORWARD;
	static const Vec3 LEFT;
	static const Vec3 UP;
};


