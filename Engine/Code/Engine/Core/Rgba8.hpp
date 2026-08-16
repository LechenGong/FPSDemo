#pragma once

#include <string>

struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

	Rgba8();
	Rgba8( std::string text );
	Rgba8( unsigned char r, unsigned char g, unsigned char b );
	Rgba8( unsigned char r, unsigned char g, unsigned char b, unsigned char a );

	void SetFromText( std::string text );
	std::string ToString() const;
	void GetAsFloats( float* colorAsFloats ) const;

	void operator=( const Rgba8& copyFrom );
	bool const operator==( const Rgba8& other ) const;
	bool const operator!=( const Rgba8& other ) const;

	static const Rgba8 WHITE;
	static const Rgba8 BLACK;
	static const Rgba8 RED;
	static const Rgba8 CYAN;
	static const Rgba8 GREEN;
	static const Rgba8 MAGENTA;
	static const Rgba8 BLUE;
	static const Rgba8 YELLOW;
	static const Rgba8 GRAY;
	static const Rgba8 ORANGE;
};

Rgba8 Interpolate( Rgba8 start, Rgba8 end, float fraction );