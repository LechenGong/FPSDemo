#include "Engine/Core/Vertex_PCU.hpp"

Vertex_PCU::Vertex_PCU( float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
	: m_position( x, y, 0.f )
	, m_color( r, g, b, a )
	, m_uvTexCoords( 0.f, 0.f )
{
}

Vertex_PCU::Vertex_PCU( Vec2 pos, Rgba8 color )
	: m_position( pos )
	, m_color( color )
	, m_uvTexCoords( 0.f, 0.f )
{
}

Vertex_PCU::Vertex_PCU( Vec3 pos, Rgba8 color, Vec2 uv )
	: m_position( pos )
	, m_color( color )
	, m_uvTexCoords( uv )
{
}

Vertex_PCU::Vertex_PCU( Vec3 pos, Rgba8 color )
	: m_position( pos )
	, m_color( color )
	, m_uvTexCoords( 0.f, 0.f )
{
}

Vertex_PCU::Vertex_PCU()
	: m_position( Vec3( 0.f, 0.f, 0.f) )
	, m_color( Rgba8(0, 0, 0, 0) )
	, m_uvTexCoords( Vec2( 0.f, 0.f ) )
{
}

Vertex_PCU::~Vertex_PCU()
{

}