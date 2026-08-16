#include "Engine/General/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/General/ParticleSystem/Particle.hpp"
#include "Engine/Core/XmlUtils.hpp"

std::unordered_map<std::string, std::vector<EmitDef>> ParticleEmitter::s_emitDefinitions;

ParticleEmitter::ParticleEmitter( std::string name, Vec3 position )
{
	m_name = name;
	m_position = position;
}

void ParticleEmitter::LoadEmitXml( const char* filePath )
{
	using namespace tinyxml2;

	XmlDocument doc;
	if (doc.LoadFile( filePath ) != XML_SUCCESS) 
	{
		return;
	}

	XmlElement* root = doc.FirstChildElement( "EffectDefinitions" );
	if (!root) return;

	for (XmlElement* effect = root->FirstChildElement( "EffectDefinition" ); effect; effect = effect->NextSiblingElement( "EffectDefinition" ))
	{
		std::string name = effect->Attribute( "name" );

		for (XmlElement* emitter = effect->FirstChildElement( "Emitter" ); emitter; emitter = emitter->NextSiblingElement( "Emitter" ))
		{
			EmitDef emitDef;
			emitDef.name = emitter->Attribute( "name" );
			emitDef.count = emitter->IntAttribute( "count" );
			emitDef.texture = emitter->Attribute( "textureName" );
			emitDef.shader = emitter->Attribute( "shaderName" );

			emitDef.position = ParseXmlAttribute( *emitter, "position", Vec3::ZERO );
			emitDef.direction = ParseXmlAttribute( *emitter, "directon", Vec3::ZERO );

			emitDef.spread = emitter->FloatAttribute( "spread" );
			emitDef.minLifetime = emitter->FloatAttribute( "minLifetime" );
			emitDef.maxLifetime = emitter->FloatAttribute( "maxLifetime" );
			emitDef.minOffset = emitter->FloatAttribute( "minOffset" );
			emitDef.maxOffset = emitter->FloatAttribute( "maxOffset" );
			emitDef.minRotation = emitter->FloatAttribute( "minRotation" );
			emitDef.maxRotation = emitter->FloatAttribute( "maxRotation" );
			emitDef.minColor = ParseXmlAttribute( *emitter, "minColor", Rgba8::WHITE );
			emitDef.maxColor = ParseXmlAttribute( *emitter, "maxColor", Rgba8::WHITE );
			emitDef.minSize = emitter->FloatAttribute( "minSize" );
			emitDef.maxSize = emitter->FloatAttribute( "maxSize" );
			emitDef.minSpeed = emitter->FloatAttribute( "minSpeed" );
			emitDef.maxSpeed = emitter->FloatAttribute( "maxSpeed" );
			emitDef.minRotationSpeed = emitter->FloatAttribute( "minRotationSpeed" );
			emitDef.maxRotationSpeed = emitter->FloatAttribute( "maxRotationSpeed" );
			emitDef.startAlpha = emitter->FloatAttribute( "startAlpha" );
			emitDef.endAlpha = emitter->FloatAttribute( "endAlpha" );
			emitDef.startAlphaTime = emitter->FloatAttribute( "startAlphaTime" );
			emitDef.endAlphaTime = emitter->FloatAttribute( "endAlphaTime" );
			emitDef.startSpeed = emitter->FloatAttribute( "startSpeed" );
			emitDef.endSpeed = emitter->FloatAttribute( "endSpeed" );
			emitDef.startSpeedTime = emitter->FloatAttribute( "startSpeedTime" );
			emitDef.endSpeedTime = emitter->FloatAttribute( "endSpeedTime" );
			emitDef.startScale = emitter->FloatAttribute( "startScale" );
			emitDef.endScale = emitter->FloatAttribute( "endSclae" );
			emitDef.startScaleTime = emitter->FloatAttribute( "startScaleTime" );
			emitDef.endScaleTime = emitter->FloatAttribute( "endScaleTime" );

			s_emitDefinitions[name].push_back( emitDef );
		}
	}
}

void ParticleEmitter::Emit()
{
	if (s_emitDefinitions.find( m_name ) == s_emitDefinitions.end())
		return;

	for (EmitDef& emitDef : s_emitDefinitions[m_name])
	{
		Particle* newParticle = new Particle( &emitDef, m_position );
		bool flag = false;
		for (int i = 0; i < m_particles.size(); i++)
		{
			if (m_particles[i] == nullptr)
			{
				m_particles[i] = newParticle;
				flag = true;
				break;
			}
		}
		if (!flag)
			m_particles.push_back( newParticle );
	}
}

void ParticleEmitter::Emit( Vec3 direction )
{
	if (s_emitDefinitions.find( m_name ) == s_emitDefinitions.end())
		return;

	for (EmitDef& emitDef : s_emitDefinitions[m_name])
	{
		Particle* newParticle = new Particle( &emitDef, m_position );
		newParticle->m_direction = direction;
		bool flag = false;
		for (int i = 0; i < m_particles.size(); i++)
		{
			if (m_particles[i] == nullptr)
			{
				m_particles[i] = newParticle;
				flag = true;
				break;
			}
		}
		if (!flag)
			m_particles.push_back( newParticle );
	}
}

void ParticleEmitter::Update( float deltaSeconds, Mat44 const& cameraMatrix )
{
	for (auto& particle : m_particles)
	{
		if (!particle) continue;

		particle->Update( deltaSeconds, cameraMatrix );
		if (particle->m_lifetime >= particle->m_maxLifetime)
		{
			delete particle;
			particle = nullptr;
		}
	}
}

void ParticleEmitter::Render() const
{
	for (auto& particle : m_particles)
	{
		if (!particle) continue;

		particle->Render();
	}
}
