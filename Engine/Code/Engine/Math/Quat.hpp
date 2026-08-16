#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Quat
{
public:
	Quat();
	Quat( Quat const& quat );
	Quat( float pX, float pY, float pZ, float pW );
	Quat( Vec3 const& pAxis, float pDegree );
	Quat( Vec4 const& pAxis, float pDegree );
	Quat( Mat44 const& rotationMatrix );
	Quat( EulerAngles const& pEulerAngles );
	//Quat( std::string data );
	~Quat();

	Quat& operator=( Quat const& other );
	bool const operator==( Quat const& other ) const;

	Quat operator+( Quat const& other ) const;
	Quat operator-( Quat const& other ) const;
	Quat operator*( Quat const& other ) const;
	Quat operator/( Quat const& other ) const;
	Quat& operator+=( Quat const& other );
	Quat& operator-=( Quat const& other );
	Quat& operator*=( Quat const& other );
	Quat& operator/=( Quat const& other );

	Quat operator*( float scalar ) const;
	Quat operator/( float scalar ) const;
	Quat& operator*=( float scalar );
	Quat& operator/=( float scalar );

	void Set( float pX, float pY, float pZ, float pW = 1.f );

	float DotProduct( Quat const& other ) const;
	Quat GetConjugated() const;
	Quat GetInversed() const;
	Quat GetNormalized() const;
	float GetLength() const;

	void Conjugate();
	void Inverse();
	void Normalize();

	Vec3 Rotate( Vec3 const& vec3 ) const;
	Mat44 ToRotationMatrix() const;

	EulerAngles ToEulerAngles() const;
	static Quat FromEulerAngles( EulerAngles const& pEulerAngles );

	static Quat SLERP( Quat const& start, Quat const& end, float t );
	static Quat NLERP( Quat const& start, Quat const& end, float t );

	bool IsNormaliazed() const;
	float GetAngle() const;
	Vec3 GetAxis() const;

	Vec3 GetForwardVector() const;
	Vec3 GetUpVector() const;
	Vec3 GetLeftVector() const;

	static const Quat IDENTITY;

public:
	float w = 1.f;
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};
