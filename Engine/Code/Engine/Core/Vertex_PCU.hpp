#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"

struct Vertex_PCU
{
public:
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

	Vertex_PCU();
	Vertex_PCU( float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	Vertex_PCU( Vec2 pos, Rgba8 color );
	Vertex_PCU( Vec3 pos, Rgba8 color, Vec2 uv );
	Vertex_PCU( Vec3 pos, Rgba8 color );
	~Vertex_PCU();
};
