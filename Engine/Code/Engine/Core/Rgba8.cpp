#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

Rgba8 const Rgba8::WHITE = Rgba8( 255, 255, 255, 255 );
Rgba8 const Rgba8::BLACK = Rgba8( 0, 0, 0, 255 );
Rgba8 const Rgba8::RED = Rgba8( 255, 0, 0, 255 );
Rgba8 const Rgba8::CYAN = Rgba8( 0, 255, 255, 255 );
Rgba8 const Rgba8::GREEN = Rgba8( 0, 255, 0, 255 );
Rgba8 const Rgba8::MAGENTA = Rgba8( 255, 0, 255, 255 );
Rgba8 const Rgba8::BLUE = Rgba8( 0, 0, 255, 255 );
Rgba8 const Rgba8::YELLOW = Rgba8( 255, 255, 0, 255 );
Rgba8 const Rgba8::GRAY = Rgba8( 127, 127, 127, 255 );
Rgba8 const Rgba8::ORANGE = Rgba8( 250, 156, 28, 255 );

Rgba8::Rgba8()
	: r( 0 )
	, g( 0 )
	, b( 0 )
	, a( 0 )
{
}

Rgba8::Rgba8( std::string text )
{
	SetFromText( text );
}

Rgba8::Rgba8( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
	: r (r)
	, g (g)
	, b (b)
	, a (a)
{
}

Rgba8::Rgba8( unsigned char r, unsigned char g, unsigned char b )
	: r (r)
	, g (g)
	, b (b)
	, a (255)
{
}

void Rgba8::SetFromText( std::string text )
{
	if (text.find( ',' ) != std::string::npos)
	{
		Strings elems = Split( text, ',' );
		r = static_cast<unsigned char>(stoi( elems[0] ));
		g = static_cast<unsigned char>(stoi( elems[1] ));
		b = static_cast<unsigned char>(stoi( elems[2] ));
		if (elems.size() == 4)
			a = static_cast<unsigned char>(stoi( elems[3] ));
		else
			a = 255;
	}
	else if (text.find( ' ' ) != std::string::npos)
	{
		Strings elems = Split( text, ' ' );
		r = static_cast<unsigned char>(stoi( elems[0] ));
		g = static_cast<unsigned char>(stoi( elems[1] ));
		b = static_cast<unsigned char>(stoi( elems[2] ));
		if (elems.size() == 4)
			a = static_cast<unsigned char>(stoi( elems[3] ));
		else
			a = 255;
	}
}

std::string Rgba8::ToString() const
{
	std::string str = "";
	str += std::to_string( r );
	str += ' ';
	str += std::to_string( g );
	str += ' ';
	str += std::to_string( b );
	str += ' ';
	str += std::to_string( a );
	return str;
}

void Rgba8::GetAsFloats( float* colorAsFloats ) const
{
	colorAsFloats[0] = static_cast<float>(r) / 255.0f;
	colorAsFloats[1] = static_cast<float>(g) / 255.0f;
	colorAsFloats[2] = static_cast<float>(b) / 255.0f;
	colorAsFloats[3] = static_cast<float>(a) / 255.0f;
}

void Rgba8::operator=( const Rgba8& copyFrom )
{
	r = copyFrom.r;
	g = copyFrom.g;
	b = copyFrom.b;
	a = copyFrom.a;
}

bool const Rgba8::operator==( const Rgba8& other ) const
{
	return r == other.r &&
		g == other.g &&
		b == other.b &&
		a == other.a;
}

bool const Rgba8::operator!=( const Rgba8& other ) const
{
	return !(*this == other);
}

Rgba8 Interpolate( Rgba8 start, Rgba8 end, float fraction )
{
	unsigned char r = static_cast<unsigned char>(Interpolate( start.r, end.r, fraction ));
	unsigned char g = static_cast<unsigned char>(Interpolate( start.g, end.g, fraction ));
	unsigned char b = static_cast<unsigned char>(Interpolate( start.b, end.b, fraction ));
	unsigned char a = static_cast<unsigned char>(Interpolate( start.a, end.a, fraction ));
	return Rgba8( r, g, b, a );
}