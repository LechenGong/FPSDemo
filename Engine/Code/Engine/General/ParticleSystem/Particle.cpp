#include "Engine/General/ParticleSystem/Particle.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/General/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

Particle::Particle( EmitDef* emitDef, Vec3 position )
{
	m_size = RollRandomFloatInRange( emitDef->minSize, emitDef->maxSize );
	m_rotation = RollRandomFloatInRange( emitDef->minRotation, emitDef->maxRotation );
	m_rotationSpeed = RollRandomFloatInRange( emitDef->minRotationSpeed, emitDef->maxRotationSpeed );
	m_maxLifetime = RollRandomFloatInRange( emitDef->minLifetime, emitDef->maxLifetime );
	m_lifetime = 0.f;
	m_direction = emitDef->direction;

	m_position = emitDef->position + position;
	m_speed = RollRandomFloatInRange( emitDef->minSpeed, emitDef->maxSpeed );
	m_velocity = m_speed * emitDef->direction;
	m_color.r = (unsigned char)RollRandomIntInRange( (int)emitDef->minColor.r, (int)emitDef->maxColor.r );
	m_color.g = (unsigned char)RollRandomIntInRange( (int)emitDef->minColor.g, (int)emitDef->maxColor.g );
	m_color.b = (unsigned char)RollRandomIntInRange( (int)emitDef->minColor.b, (int)emitDef->maxColor.b );
	m_color.a = (unsigned char)emitDef->startAlpha;

	m_emitDef = emitDef;
}

void Particle::Update( float deltaSeconds, Mat44 const& cameraMatrix )
{
	UNUSED( cameraMatrix );
	if (m_maxLifetime - m_lifetime < deltaSeconds)
		deltaSeconds = m_maxLifetime - m_lifetime;

	m_lifetime += deltaSeconds;
	
	float alphaT = RangeMapClamped( m_lifetime, m_emitDef->startAlphaTime, m_emitDef->endAlphaTime, 0.f, 1.f );
	float speedT = RangeMapClamped( m_lifetime, m_emitDef->startSpeedTime, m_emitDef->endSpeedTime, 0.f, 1.f );
	float scaleT = RangeMapClamped( m_lifetime, m_emitDef->startScaleTime, m_emitDef->endScaleTime, 0.f, 1.f );

	m_color.a = (unsigned char)(m_emitDef->startAlpha + alphaT * (m_emitDef->endAlpha - m_emitDef->startAlpha));
	m_speed = m_emitDef->startSpeed + speedT * (m_emitDef->endSpeed - m_emitDef->startSpeed);
	m_velocity = m_speed * m_direction;
	m_scale = m_emitDef->startScale + scaleT * (m_emitDef->endScale - m_emitDef->startScale);

	m_position += m_velocity * deltaSeconds;

	float scaledSize = m_size * m_scale;
// 	m_quad.clear();
// 	AddVertsForQuad3D( m_quad,
// 		Vec3( m_position.x, m_position.y - scaledSize, m_position.z - scaledSize ),
// 		Vec3( m_position.x, m_position.y + scaledSize, m_position.z - scaledSize ),
// 		Vec3( m_position.x, m_position.y + scaledSize, m_position.z + scaledSize ),
// 		Vec3( m_position.x, m_position.y - scaledSize, m_position.z + scaledSize ),
// 		Rgba8::WHITE, AABB2::ZERO_TO_ONE );
// 	
// 	Mat44 billboardTransform = GetBillboardMatrix( BillboardType::FULL_FACING, cameraMatrix, m_position, Vec2::ONE );
// 	
// 	TransformVertexArray3D( m_quad, billboardTransform );

	m_quad.clear();
	AddVertsForQuad3D( m_quad,
		Vec3( m_position.x - scaledSize, m_position.y - scaledSize, m_position.z ),
		Vec3( m_position.x + scaledSize, m_position.y - scaledSize, m_position.z ),
		Vec3( m_position.x + scaledSize, m_position.y + scaledSize, m_position.z ),
		Vec3( m_position.x - scaledSize, m_position.y + scaledSize, m_position.z ),
		Rgba8::WHITE, AABB2::ZERO_TO_ONE );
}

void Particle::Render() const
{
	g_theRenderer->BindShader( g_theRenderer->CreateShader( m_emitDef->shader.c_str(), VertexType::VERTEX_PCU ) );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( m_emitDef->texture.c_str() ) );
	g_theRenderer->DrawVertexArray( m_quad );
}
