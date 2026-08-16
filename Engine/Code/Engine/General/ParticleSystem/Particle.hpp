#pragma once

#include <vector>

#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"

struct EmitDef;

class Particle
{
public:
	Particle( EmitDef* emitDef, Vec3 position );

	Vec3 m_position; // position should be relative to actor
	Vec3 m_velocity;
	Rgba8 m_color;
	float m_size;
	float m_speed;
	float m_scale;
	Vec3 m_direction;
	float m_rotation;
	float m_rotationSpeed;
	float m_lifetime;
	float m_maxLifetime;

	void Update( float deltaSeconds, Mat44 const& cameraMatrix );
	void Render() const;

	EmitDef* m_emitDef;
	std::vector<Vertex_PCU> m_quad;
};