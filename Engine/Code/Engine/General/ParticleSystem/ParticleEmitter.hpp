#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"

struct EmitDef 
{
	std::string name;
	int count;
	std::string texture;
	std::string shader;
	Vec3 position;
	Vec3 direction;
	float spread;
	float minLifetime, maxLifetime;
	float minOffset, maxOffset;
	float minRotation, maxRotation;
	Rgba8 minColor, maxColor;
	float minSize, maxSize;
	float minSpeed, maxSpeed;
	float minRotationSpeed, maxRotationSpeed;
	float startAlpha, endAlpha;
	float startAlphaTime, endAlphaTime;
	float startSpeed, endSpeed;
	float startSpeedTime, endSpeedTime;
	float startScale, endScale;
	float startScaleTime, endScaleTime;
};

class Particle;

class ParticleEmitter
{
public:
	ParticleEmitter() {};
	ParticleEmitter( std::string name, Vec3 position );

	std::string m_name;

	Vec3 m_position;

	std::vector<Particle*> m_particles;

public:
	static void LoadEmitXml( const char* filePath );
	static std::unordered_map<std::string, std::vector<EmitDef>> s_emitDefinitions;

	void Emit();
	void Emit( Vec3 direction );
	void Update( float deltaSeconds, Mat44 const& cameraMatrix );
	void Render() const;
};