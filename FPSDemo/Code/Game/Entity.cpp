#include "Game/Entity.hpp"
#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerController.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Weapon.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quat.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"

#include <cmath>

std::vector<EntityDefinition*> EntityDefinition::s_EntityDefs;

ActorAnimation* EntityDefinition::GetAnimation( AnimationGroup* animationGroup, Vec3 direction ) const
{
	if (animationGroup == nullptr || animationGroup->directions.empty())
	{
		return nullptr;
	}

	ActorAnimation* winner = animationGroup->directions[0];
	float maximum = -9999.f;
	for (ActorAnimation* actorAnimation : animationGroup->directions)
	{
		float dotProduct = DotProduct3D( actorAnimation->direction.GetNormalized(), direction.GetNormalized() );
		if (dotProduct > maximum)
		{
			maximum = dotProduct;
			winner = actorAnimation;
		}
	}
	return winner;
}

AnimationGroup* EntityDefinition::GetAnimationGroupByName( std::string searchingName ) const
{
	for (AnimationGroup* animationGroup : animations)
	{
		if (animationGroup->name == searchingName)
		{
			return animationGroup;
		}
	}
	if (animations.empty())
	{
		return nullptr;
	}
	return animations[0];
}

ActorSound* EntityDefinition::GetSoundByType( std::string type ) const
{
	for (ActorSound* actorSound : sounds)
	{
		if (actorSound->soundType == type)
		{
			return actorSound;
		}
	}
	return nullptr;
}

static Faction ParseFaction( std::string const& factionText )
{
	std::string lower = factionText;
	for (char& c : lower)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = (char)(c - 'A' + 'a');
		}
	}

	if (lower == "marine")
	{
		return Faction::MARINE;
	}
	if (lower == "demon")
	{
		return Faction::DEMON;
	}
	if (lower == "neutral")
	{
		return Faction::NEUTRAL;
	}

	return Faction::INVALID;
}

static void ParseDefaultGroup( XmlElement const& element, EntityDefinition& definition )
{
	definition.name = ParseXmlAttribute( element, "name", "" );
	definition.controllerName = ParseXmlAttribute( element, "controller", "" );
	definition.faction = ParseFaction( ParseXmlAttribute( element, "faction", "Neutral" ) );
	definition.health = ParseXmlAttribute( element, "health", 0.f );
	definition.canBePossessed = ParseXmlAttribute( element, "canBePossessed", false );
	definition.corpseLifetime = ParseXmlAttribute( element, "corpseLifetime", 0.f );
	definition.visible = ParseXmlAttribute( element, "visible", false );
	definition.dieOnSpawn = ParseXmlAttribute( element, "dieOnSpawn", false );
}

static CollisionShape ParseCollisionShape( std::string const& shapeText )
{
	std::string lower = shapeText;
	for (char& c : lower)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = (char)(c - 'A' + 'a');
		}
	}

	if (lower == "sphere")
	{
		return CollisionShape::SPHERE;
	}
	if (lower == "cube" || lower == "box" || lower == "aabb")
	{
		return CollisionShape::CUBE;
	}
	if (lower == "capsule" || lower.empty())
	{
		return CollisionShape::CAPSULE;
	}

	ERROR_AND_DIE( Stringf( "Unknown collision shape: %s", shapeText.c_str() ).c_str() );
}

static void ParseCollisionGroup( XmlElement const& element, EntityDefinition& definition )
{
	definition.collisionShape = ParseCollisionShape( ParseXmlAttribute( element, "shape", "Capsule" ) );
	definition.radius = ParseXmlAttribute( element, "radius", definition.radius );
	definition.height = ParseXmlAttribute( element, "height", definition.height );
	definition.collidesWithWorld = ParseXmlAttribute( element, "collidesWithWorld", definition.collidesWithWorld );

	if (element.Attribute( "collidesWithEntitys" ) != nullptr)
	{
		definition.collidesWithEntitys = ParseXmlAttribute( element, "collidesWithEntitys", definition.collidesWithEntitys );
	}
	else if (element.Attribute( "collidesWithEntities" ) != nullptr)
	{
		definition.collidesWithEntitys = ParseXmlAttribute( element, "collidesWithEntities", definition.collidesWithEntitys );
	}
	else if (element.Attribute( "collidesWithActors" ) != nullptr)
	{
		definition.collidesWithEntitys = ParseXmlAttribute( element, "collidesWithActors", definition.collidesWithEntitys );
	}

	definition.damageOnCollide = ParseXmlAttribute( element, "damageOnCollide", definition.damageOnCollide );
	definition.impulseOnCollide = ParseXmlAttribute( element, "impulseOnCollide", definition.impulseOnCollide );

	if (element.Attribute( "dieOnCollide" ) != nullptr)
	{
		definition.dieOnCollide = ParseXmlAttribute( element, "dieOnCollide", definition.dieOnCollide );
	}
	else
	{
		definition.dieOnCollide = ParseXmlAttribute( element, "diesOnCollide", definition.dieOnCollide );
	}
}

static void ParseExplosionGroup( XmlElement const& element, EntityDefinition& definition )
{
	definition.doesExplode = ParseXmlAttribute( element, "doesExplode", true );
	definition.range = ParseXmlAttribute( element, "range", definition.range );
	definition.damageOnExplosion = ParseXmlAttribute( element, "damageOnExplosion", definition.damageOnExplosion );
	definition.impulseOnExplosion = ParseXmlAttribute( element, "impulseOnExplosion", definition.impulseOnExplosion );
	definition.explosionAnimationActor = ParseXmlAttribute( element, "explosionAnimationActor", definition.explosionAnimationActor );
}

static void ParsePhysicsGroup( XmlElement const& element, EntityDefinition& definition )
{
	if (element.Attribute( "simulatePhysics" ) != nullptr)
	{
		definition.simulatePhysics = ParseXmlAttribute( element, "simulatePhysics", definition.simulatePhysics );
	}
	else
	{
		definition.simulatePhysics = ParseXmlAttribute( element, "simulate", definition.simulatePhysics );
	}

	definition.flying = ParseXmlAttribute( element, "flying", definition.flying );
	definition.walkSpeed = ParseXmlAttribute( element, "walkSpeed", definition.walkSpeed );
	definition.runSpeed = ParseXmlAttribute( element, "runSpeed", definition.runSpeed );
	definition.chargeSpeed = ParseXmlAttribute( element, "chargeSpeed", definition.chargeSpeed );
	definition.turnSpeed = ParseXmlAttribute( element, "turnSpeed", definition.turnSpeed );
	definition.drag = ParseXmlAttribute( element, "drag", definition.drag );
}

static void ParseAIGroup( XmlElement const& element, EntityDefinition& definition )
{
	definition.aiEnabled = ParseXmlAttribute( element, "aiEnabled", definition.aiEnabled );
	definition.sightRadius = ParseXmlAttribute( element, "sightRadius", definition.sightRadius );
	definition.sightAngle = ParseXmlAttribute( element, "sightAngle", definition.sightAngle );
}

static void ParseCameraGroup( XmlElement const& element, EntityDefinition& definition )
{
	definition.eyeHeight = ParseXmlAttribute( element, "eyeHeight", definition.eyeHeight );
	definition.cameraFOV = ParseXmlAttribute( element, "cameraFOV", definition.cameraFOV );
}

static BillboardType ParseBillboardType( std::string const& billboardTypeText )
{
	if (billboardTypeText == "FullOpposing")
	{
		return BillboardType::FULL_OPPOSING;
	}
	if (billboardTypeText == "WorldUpFacing")
	{
		return BillboardType::WORLD_UP_FACING;
	}
	if (billboardTypeText == "WorldUpOpposing")
	{
		return BillboardType::WORLD_UP_OPPOSING;
	}
	if (billboardTypeText == "FullFacing")
	{
		return BillboardType::FULL_FACING;
	}
	return BillboardType::NONE;
}

static SpriteAnimPlaybackType ParsePlaybackMode( std::string const& playbackModeText )
{
	if (playbackModeText == "Loop")
	{
		return SpriteAnimPlaybackType::LOOP;
	}
	if (playbackModeText == "Pingpong")
	{
		return SpriteAnimPlaybackType::PINGPONG;
	}
	return SpriteAnimPlaybackType::ONCE;
}

static void ParseVisualGroup( XmlElement const& element, EntityDefinition& definition )
{
	definition.size = ParseXmlAttribute( element, "size", definition.size );
	definition.pivot = ParseXmlAttribute( element, "pivot", definition.pivot );
	definition.billboardType = ParseBillboardType( ParseXmlAttribute( element, "billboardType", "" ) );
	definition.renderLit = ParseXmlAttribute( element, "renderLit", definition.renderLit );
	definition.renderRounded = ParseXmlAttribute( element, "renderRounded", definition.renderRounded );

	std::string shaderPath = ParseXmlAttribute( element, "shader", "" );
	if (!shaderPath.empty())
	{
		bool isDiffuse = shaderPath.find( "Diffuse" ) != std::string::npos;
		definition.shader = g_theRenderer->CreateShader( shaderPath.c_str(), isDiffuse ? VertexType::VERTEX_PCUTBN : VertexType::VERTEX_PCU );
	}

	std::string texturePath = ParseXmlAttribute( element, "spriteSheet", "" );
	definition.cellCount = ParseXmlAttribute( element, "cellCount", definition.cellCount );
	if (!texturePath.empty())
	{
		Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( texturePath.c_str() );
		definition.spriteSheet = new SpriteSheet( *texture, definition.cellCount );
	}

	XmlElement const* animationGroupElem = element.FirstChildElement( "AnimationGroup" );
	while (animationGroupElem)
	{
		AnimationGroup* animationGroup = new AnimationGroup();
		animationGroup->name = ParseXmlAttribute( *animationGroupElem, "name", "" );
		animationGroup->scaleBySpeed = ParseXmlAttribute( *animationGroupElem, "scaleBySpeed", false );
		float secondsPerFrame = ParseXmlAttribute( *animationGroupElem, "secondsPerFrame", 0.f );
		animationGroup->playbackMode = ParsePlaybackMode( ParseXmlAttribute( *animationGroupElem, "playbackMode", "" ) );

		XmlElement const* directionElement = animationGroupElem->FirstChildElement( "Direction" );
		while (directionElement)
		{
			ActorAnimation* animationDirection = new ActorAnimation();
			animationDirection->direction = ParseXmlAttribute( *directionElement, "vector", Vec3::ZERO ).GetNormalized();

			XmlElement const* animationElement = directionElement->FirstChildElement( "Animation" );
			GUARANTEE_OR_DIE( animationElement != nullptr, Stringf( "AnimationGroup '%s' Direction missing Animation child", animationGroup->name.c_str() ).c_str() );

			int startFrame = ParseXmlAttribute( *animationElement, "startFrame", 0 );
			int endFrame = ParseXmlAttribute( *animationElement, "endFrame", 0 );
			int totalCells = definition.cellCount.x * definition.cellCount.y;
			int totalFrames = endFrame - startFrame + 1 + totalCells;
			totalFrames = (totalFrames >= totalCells) ? totalFrames - totalCells : totalFrames;

			GUARANTEE_OR_DIE( definition.spriteSheet != nullptr, Stringf( "Entity '%s' Visuals missing spriteSheet for animations", definition.name.c_str() ).c_str() );
			animationDirection->animationDef = new SpriteAnimDefinition(
				*definition.spriteSheet,
				startFrame,
				endFrame,
				totalFrames * secondsPerFrame,
				animationGroup->playbackMode );

			animationGroup->directions.push_back( animationDirection );
			directionElement = directionElement->NextSiblingElement( "Direction" );
		}

		definition.animations.push_back( animationGroup );
		animationGroupElem = animationGroupElem->NextSiblingElement( "AnimationGroup" );
	}
}

static void ParseSoundsGroup( XmlElement const& element, EntityDefinition& definition )
{
	XmlElement const* soundElem = element.FirstChildElement( "Sound" );
	while (soundElem)
	{
		ActorSound* actorSound = new ActorSound();
		actorSound->soundType = ParseXmlAttribute( *soundElem, "sound", "" );
		actorSound->soundName = ParseXmlAttribute( *soundElem, "name", "" );
		definition.sounds.push_back( actorSound );
		soundElem = soundElem->NextSiblingElement( "Sound" );
	}
}

static void ParseInventoryGroup( XmlElement const& element, EntityDefinition& definition )
{
	XmlElement const* weaponElem = element.FirstChildElement( "Weapon" );
	while (weaponElem)
	{
		std::string weaponName = ParseXmlAttribute( *weaponElem, "name", "" );
		GUARANTEE_OR_DIE( !weaponName.empty(), Stringf( "Entity '%s' Inventory Weapon missing name", definition.name.c_str() ).c_str() );
		GUARANTEE_OR_DIE( WeaponDefinition::GetWeaponDef( weaponName ) != nullptr,
			Stringf( "Entity '%s' Inventory references unknown weapon '%s'", definition.name.c_str(), weaponName.c_str() ).c_str() );
		definition.weaponNames.push_back( weaponName );
		weaponElem = weaponElem->NextSiblingElement( "Weapon" );
	}
}

void EntityDefinition::DefineEntityType( XmlElement const& entityXmlElement )
{
	EntityDefinition* definition = new EntityDefinition();
	ParseDefaultGroup( entityXmlElement, *definition );

	XmlElement const* childElement = entityXmlElement.FirstChildElement();
	while (childElement)
	{
		std::string tagName = childElement->Name();
		if (tagName == "Collision")
		{
			ParseCollisionGroup( *childElement, *definition );
		}
		else if (tagName == "Explosion")
		{
			ParseExplosionGroup( *childElement, *definition );
		}
		else if (tagName == "Physics")
		{
			ParsePhysicsGroup( *childElement, *definition );
		}
		else if (tagName == "AI")
		{
			ParseAIGroup( *childElement, *definition );
		}
		else if (tagName == "Camera")
		{
			ParseCameraGroup( *childElement, *definition );
		}
		else if (tagName == "Visuals")
		{
			ParseVisualGroup( *childElement, *definition );
		}
		else if (tagName == "Sounds")
		{
			ParseSoundsGroup( *childElement, *definition );
		}
		else if (tagName == "Inventory")
		{
			ParseInventoryGroup( *childElement, *definition );
		}

		childElement = childElement->NextSiblingElement();
	}

	GUARANTEE_OR_DIE( !definition->name.empty(), "EntityDefinition missing required name attribute" );
	s_EntityDefs.push_back( definition );
}

void EntityDefinition::InitializeEntityDefs( char const* path )
{
	if (!s_EntityDefs.empty())
	{
		return;
	}

	XmlDocument document;
	XmlError result = document.LoadFile( path );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to load entity definitions: %s", path ).c_str() );

	XmlElement const* root = document.RootElement();
	GUARANTEE_OR_DIE( root != nullptr, Stringf( "Entity definitions missing root: %s", path ).c_str() );

	XmlElement const* defElement = root->FirstChildElement();
	while (defElement)
	{
		std::string tagName = defElement->Name();
		if (tagName == "EntityDefinition" || tagName == "ActorDefinition")
		{
			DefineEntityType( *defElement );
		}
		defElement = defElement->NextSiblingElement();
	}
}

void EntityDefinition::InitializeProjectileEntityDef( char const* path )
{
	static bool s_projectileDefsInitialized = false;
	if (s_projectileDefsInitialized)
	{
		return;
	}
	s_projectileDefsInitialized = true;

	XmlDocument document;
	XmlError result = document.LoadFile( path );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to load projectile entity definitions: %s", path ).c_str() );

	XmlElement const* root = document.RootElement();
	GUARANTEE_OR_DIE( root != nullptr, Stringf( "Projectile entity definitions missing root: %s", path ).c_str() );

	XmlElement const* defElement = root->FirstChildElement();
	while (defElement)
	{
		std::string tagName = defElement->Name();
		if (tagName == "EntityDefinition" || tagName == "ProjectileDefinition")
		{
			DefineEntityType( *defElement );
		}
		defElement = defElement->NextSiblingElement();
	}
}

EntityDefinition* EntityDefinition::GetEntityDef( std::string entityName )
{
	for (EntityDefinition* definition : s_EntityDefs)
	{
		if (definition != nullptr && definition->name == entityName)
		{
			return definition;
		}
	}
	return nullptr;
}

Entity::Entity()
{
}

Entity::Entity( Game* owner )
	: m_game( owner )
{
}

Entity::Entity( Map* map, EntityDefinition const* definition )
	: m_map( map )
	, m_definition( definition )
{
	if (definition != nullptr)
	{
		m_health = definition->health;
		m_isDead = definition->dieOnSpawn;
		m_simulatePhysics = definition->simulatePhysics;
		m_damageOnCollide = definition->damageOnCollide;
		m_impulseOnCollide = definition->impulseOnCollide;
		m_scale = 1.f;
		m_previousPosition = m_position;
		if (!definition->animations.empty())
		{
			if (definition->dieOnSpawn)
			{
				m_currentState = EntityState::DEATH;
				m_currentAnimationGroup = definition->GetAnimationGroupByName( "Death" );
				m_corpseTimer = definition->corpseLifetime;
				m_simulatePhysics = false;
			}
			else
			{
				m_currentState = EntityState::IDLE;
				m_currentAnimationGroup = definition->GetAnimationGroupByName( "Idle" );
				if (m_currentAnimationGroup == nullptr)
				{
					m_currentAnimationGroup = definition->GetAnimationGroupByName( "Walk" );
				}
			}
			m_animationTimer = 0.f;
		}
		InitializeCollision();
		InitializeWeapons();
	}
}

Entity::~Entity()
{
	for (Weapon* weapon : m_weapons)
	{
		delete weapon;
	}
	m_weapons.clear();

	delete m_controller;
	m_controller = nullptr;

	delete m_collision;
	m_collision = nullptr;
}

static char const* GetAnimationGroupNameForState( EntityState state )
{
	switch (state)
	{
	case EntityState::IDLE:   return "Idle";
	case EntityState::WALK:   return "Walk";
	case EntityState::ATTACK: return "Attack";
	case EntityState::HURT:   return "Hurt";
	case EntityState::DEATH:  return "Death";
	default:                  return "Idle";
	}
}

void Entity::Update( float deltaSeconds )
{
	if (m_hitStunTimer > 0.f)
	{
		m_hitStunTimer -= deltaSeconds;
		if (m_hitStunTimer < 0.f)
		{
			m_hitStunTimer = 0.f;
		}
	}

	UpdatePhysics( deltaSeconds );
	m_animationTimer += deltaSeconds * m_animationPlaybackSpeed;
	UpdateAnimation();
	SyncCollisionTransform();

	for (Weapon* weapon : m_weapons)
	{
		if (weapon != nullptr)
		{
			weapon->Update( deltaSeconds );
		}
	}
}

void Entity::UpdatePhysics( float deltaSeconds )
{
	if (m_definition == nullptr || !m_simulatePhysics)
	{
		return;
	}

	m_previousPosition = m_position;

	if (!m_definition->flying)
	{
		// Grounded from last frame: do not re-apply gravity before integrate,
		// or we sink into slopes and world MTV slides us downhill.
		if (m_isGrounded)
		{
			if (m_velocity.z < 0.f)
			{
				m_velocity.z = 0.f;
			}
		}
		else
		{
			m_velocity.z += ENTITY_GRAVITY_Z * deltaSeconds;
		}
	}

	m_position += m_velocity * deltaSeconds;

	if (!m_definition->flying)
	{
		ResolveGroundContact();
	}
	else
	{
		m_isGrounded = false;
	}

	float drag = m_definition->drag;
	if (drag > 0.f && !IsHitStunned())
	{
		float const horizontalDamp = 1.f / (1.f + drag * deltaSeconds);
		m_velocity.x *= horizontalDamp;
		m_velocity.y *= horizontalDamp;
	}
}

Vec3 Entity::GetCollisionBottomPosition() const
{
	// Standing setup places feet / capsule bottom at m_position.
	return m_position;
}

void Entity::ResolveGroundContact()
{
	m_isGrounded = false;

	if (m_map == nullptr)
	{
		return;
	}

	auto tryLandFromHit = [this]( RaycastResult3D const& groundHit, float startNudge ) -> bool
	{
		if (!groundHit.m_didImpact)
		{
			return false;
		}

		float const distanceToGround = groundHit.m_impactDist - startNudge;
		if (distanceToGround > ENTITY_GROUND_LAND_DISTANCE)
		{
			return false;
		}

		// Negative distance: already penetrating / overlapping ground — count as landed, do not snap upward.
		if (distanceToGround >= 0.f)
		{
			m_position.z -= distanceToGround;
		}

		if (m_velocity.z < 0.f)
		{
			m_velocity.z = 0.f;
		}
		m_isGrounded = true;
		return true;
	};

	Vec3 const bottom = GetCollisionBottomPosition();
	constexpr float startNudge = 0.01f;
	Vec3 const down = Vec3( 0.f, 0.f, -1.f );

	// Layer 1: center ray from capsule bottom.
	Vec3 const rayStart = bottom + Vec3( 0.f, 0.f, startNudge );
	RaycastResult3D const rayHit = m_map->RaycastWorld( rayStart, down, ENTITY_GROUND_RAY_LENGTH );
	if (tryLandFromHit( rayHit, startNudge ))
	{
		return;
	}

	// Layer 2: only if the center ray missed entirely — spherecast the capsule cross-section down.
	if (rayHit.m_didImpact)
	{
		return;
	}

	float radius = 0.f;
	if (m_definition != nullptr)
	{
		radius = m_definition->radius;
	}
	if (radius <= 0.f)
	{
		return;
	}

	// Sphere sits on the feet plane so its bottom matches the capsule bottom.
	Vec3 const sphereStart = bottom + Vec3( 0.f, 0.f, radius + startNudge );
	RaycastResult3D const sphereHit = m_map->SpherecastWorld( sphereStart, down, ENTITY_GROUND_RAY_LENGTH, radius );
	tryLandFromHit( sphereHit, startNudge );
}

void Entity::ApplyCollisionDisplacement( Vec3 const& displacement )
{
	if (displacement == Vec3::ZERO)
	{
		return;
	}
	if (!isfinite( displacement.x ) || !isfinite( displacement.y ) || !isfinite( displacement.z ))
	{
		return;
	}
	m_position += displacement;
	SyncCollisionTransform();
}

bool Entity::WantsWorldCollision() const
{
	return m_definition != nullptr
		&& m_definition->collidesWithWorld
		&& m_collision != nullptr
		&& m_collision->IsCollisionEnabled();
}

bool Entity::WantsEntityCollision() const
{
	return m_definition != nullptr
		&& m_definition->collidesWithEntitys
		&& !m_isDead
		&& m_collision != nullptr
		&& m_collision->IsCollisionEnabled();
}

void Entity::InitializeWeapons()
{
	for (Weapon* weapon : m_weapons)
	{
		delete weapon;
	}
	m_weapons.clear();
	m_currentWeaponIndex = 0;

	if (m_definition == nullptr)
	{
		return;
	}

	for (std::string const& weaponName : m_definition->weaponNames)
	{
		WeaponDefinition const* weaponDef = WeaponDefinition::GetWeaponDef( weaponName );
		GUARANTEE_OR_DIE( weaponDef != nullptr, Stringf( "Entity '%s' missing weapon def '%s'", m_definition->name.c_str(), weaponName.c_str() ).c_str() );
		m_weapons.push_back( new Weapon( this, weaponDef ) );
	}
}

void Entity::TakeDamage( float damage )
{
	if (m_isDead || damage <= 0.f || m_isInvincible)
	{
		return;
	}
	// Held charging projectiles keep collision disabled and must not be destroyed by melee/splash.
	if (m_collision != nullptr && !m_collision->IsCollisionEnabled())
	{
		return;
	}

	float const appliedDamage = (damage < m_health) ? damage : m_health;
	if (m_map != nullptr)
	{
		m_map->NotifyPlayerDamaged( this, appliedDamage );
	}

	m_health -= damage;
	if (m_health <= 0.f)
	{
		m_health = 0.f;
		m_isDead = true;
		m_hitStunTimer = 0.f;
		m_corpseTimer = (m_definition != nullptr) ? m_definition->corpseLifetime : 0.f;
		SetAnimationGroup( EntityState::DEATH );
		PlayDefinitionSound( "Death" );
		if (m_map != nullptr)
		{
			m_map->NotifyEntityKilled( this );
		}
	}
	else
	{
		ApplyHitStun( ENTITY_HIT_STUN_SECONDS );
		SetAnimationGroup( EntityState::HURT );
		PlayDefinitionSound( "Hurt" );
	}
}

void Entity::PlayDefinitionSound( std::string const& soundType ) const
{
	if (m_definition == nullptr || g_theAudio == nullptr || soundType.empty())
	{
		return;
	}

	ActorSound const* actorSound = m_definition->GetSoundByType( soundType );
	if (actorSound == nullptr || actorSound->soundName.empty())
	{
		return;
	}

	bool const isPlayer = (m_map != nullptr && m_map->IsPlayerEntity( this ));
	SoundID const soundId = g_theAudio->CreateOrGetSound( actorSound->soundName, !isPlayer );
	float const volume = GetSfxVolume();
	if (isPlayer)
	{
		g_theAudio->StartSound( soundId, false, volume );
	}
	else
	{
		g_theAudio->StartSoundAt( soundId, m_position, false, volume );
	}
}

void Entity::AddHealth( float amount )
{
	if (m_isDead || amount <= 0.f)
	{
		return;
	}

	float maxHealth = amount;
	if (m_definition != nullptr && m_definition->health > 0.f)
	{
		maxHealth = m_definition->health;
	}
	m_health += amount;
	if (m_health > maxHealth)
	{
		m_health = maxHealth;
	}
}

void Entity::ApplyHitStun( float durationSeconds )
{
	if (durationSeconds <= 0.f)
	{
		return;
	}
	if (durationSeconds > m_hitStunTimer)
	{
		m_hitStunTimer = durationSeconds;
	}
}

void Entity::ApplyKnockback( Vec3 const& worldDirection, float distance )
{
	if (m_isDead || m_isInvincible || distance <= 0.f)
	{
		return;
	}

	Vec3 dir( worldDirection.x, worldDirection.y, 0.f );
	if (dir.GetLengthSquared() < 0.0001f)
	{
		return;
	}
	dir = dir.GetNormalized();

	m_position += dir * distance;

	// Keep sliding for roughly the stun window so it doesn't look like a pure teleport.
	float const slideSpeed = distance / MAX( ENTITY_HIT_STUN_SECONDS, 0.05f );
	m_velocity.x = dir.x * slideSpeed;
	m_velocity.y = dir.y * slideSpeed;

	SyncCollisionTransform();
}

void Entity::EquipWeapon( int weaponIndex )
{
	if (m_weapons.empty())
	{
		m_currentWeaponIndex = 0;
		return;
	}

	if (weaponIndex < 0 || weaponIndex >= (int)m_weapons.size())
	{
		return;
	}

	if (weaponIndex == m_currentWeaponIndex)
	{
		return;
	}

	if (Weapon* current = GetCurrentWeapon())
	{
		current->CancelCharge();
		current->CancelReload();
	}

	m_currentWeaponIndex = weaponIndex;
	if (g_theAudio != nullptr && g_soundIds[AUDIO_WEAPON_SWITCH] != MISSING_SOUND_ID)
	{
		g_theAudio->StartSound( g_soundIds[AUDIO_WEAPON_SWITCH], false, GetSfxVolume() );
	}
}

Weapon* Entity::GetCurrentWeapon() const
{
	if (m_currentWeaponIndex < 0 || m_currentWeaponIndex >= (int)m_weapons.size())
	{
		return nullptr;
	}
	return m_weapons[m_currentWeaponIndex];
}

void Entity::ClearChargingProjectileRef( Entity* destroyedProjectile )
{
	for (Weapon* weapon : m_weapons)
	{
		if (weapon != nullptr)
		{
			weapon->ClearChargingProjectileRef( destroyedProjectile );
		}
	}
}

void Entity::SetScale( float scale )
{
	m_scale = MAX( scale, 0.f );
	if (m_collision != nullptr)
	{
		m_collision->SetLocalScale( Vec3( m_scale, m_scale, m_scale ) );
	}
}

void Entity::InitializeCollision()
{
	delete m_collision;
	m_collision = nullptr;

	if (m_definition == nullptr)
	{
		return;
	}

	bool const hasSize = (m_definition->radius > 0.f) || (m_definition->height > 0.f);
	if (!hasSize)
	{
		return;
	}

	bool const collisionEnabled = m_definition->collidesWithWorld || m_definition->collidesWithEntitys;
	CollisionChannel const channel = CollisionChannel::PAWN;

	switch (m_definition->collisionShape)
	{
	case CollisionShape::CAPSULE:
	{
		// Definition height is total capsule height; CapsuleComponent height excludes hemispheres.
		// Keep a tiny bone length so degenerate capsules (height == 2*radius) stay well-defined.
		float const cylinderHeight = MAX( 0.001f, m_definition->height - (2.f * m_definition->radius) );
		m_collision = new CapsuleComponent( m_definition->radius, cylinderHeight, collisionEnabled, channel );
		break;
	}
	case CollisionShape::SPHERE:
	{
		m_collision = new SphereComponent( m_definition->radius, collisionEnabled, channel );
		break;
	}
	case CollisionShape::CUBE:
	{
		float const length = m_definition->radius * 2.f;
		float const width = m_definition->radius * 2.f;
		m_collision = new CubeComponent( length, width, m_definition->height, collisionEnabled, channel );
		break;
	}
	default:
		break;
	}

	SyncCollisionTransform();
}

void Entity::SyncCollisionTransform()
{
	if (m_collision == nullptr)
	{
		return;
	}

	float centerZ = 0.f;
	bool const useFeetOrigin = (m_definition == nullptr) || !m_definition->flying;
	if (useFeetOrigin)
	{
		switch (m_collision->GetCollisionShape())
		{
		case CollisionShape::CAPSULE:
			centerZ = (m_definition != nullptr) ? (m_definition->height * 0.5f) : 0.f;
			break;
		case CollisionShape::SPHERE:
			centerZ = (m_definition != nullptr) ? m_definition->radius : 0.f;
			break;
		case CollisionShape::CUBE:
			centerZ = (m_definition != nullptr) ? (m_definition->height * 0.5f) : 0.f;
			break;
		default:
			break;
		}
	}

	m_collision->SetLocalPosition( m_position + Vec3( 0.f, 0.f, centerZ ) );
	m_collision->SetLocalOrientation( Quat( m_orientation ) );
}

void Entity::UpdateAnimation()
{
	if (m_definition == nullptr || m_currentAnimationGroup == nullptr)
	{
		return;
	}

	if (m_map == nullptr || m_map->GetPlayerController() == nullptr)
	{
		return;
	}

	Camera const& camera = m_map->GetPlayerController()->m_camera;
	Vec3 cameraToEntityOn2D = Vec3( (m_position - camera.m_position).GetXY() );
	if (cameraToEntityOn2D.GetLengthSquared() > 0.0001f)
	{
		cameraToEntityOn2D = m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetOrthonormalInverse()
			.TransformVectorQuantity3D( cameraToEntityOn2D ).GetNormalized();
		m_currentAnimation = m_definition->GetAnimation( m_currentAnimationGroup, cameraToEntityOn2D );
	}
	else if (!m_currentAnimationGroup->directions.empty())
	{
		m_currentAnimation = m_currentAnimationGroup->directions[0];
	}

	if (m_currentAnimation == nullptr || m_currentAnimation->animationDef == nullptr)
	{
		return;
	}

	if (m_currentAnimationGroup->scaleBySpeed)
	{
		m_animationPlaybackSpeed = Clamp( m_velocity.GetLengthXY(), 0.f, 8.f );
	}
	else
	{
		m_animationPlaybackSpeed = 1.f;
	}

	if (m_isDead || m_currentState == EntityState::DEATH)
	{
		return;
	}

	if (m_animationTimer > m_currentAnimation->animationDef->m_durationSeconds)
	{
		m_currentState = EntityState::IDLE;
		m_currentAnimationGroup = m_definition->GetAnimationGroupByName( "Idle" );
		m_animationTimer = 0.f;
	}
}

void Entity::SetAnimationGroup( EntityState state )
{
	if ((int)state <= (int)m_currentState)
	{
		return;
	}
	if (m_definition == nullptr)
	{
		return;
	}

	m_currentState = state;
	m_currentAnimationGroup = m_definition->GetAnimationGroupByName( GetAnimationGroupNameForState( state ) );
	m_animationTimer = 0.f;
}

void Entity::Render() const
{
	if (m_definition == nullptr || !m_definition->visible)
	{
		return;
	}
	if (m_currentAnimation == nullptr || m_currentAnimation->animationDef == nullptr)
	{
		return;
	}
	if (m_map == nullptr || m_map->GetPlayerController() == nullptr)
	{
		return;
	}

	AABB2 spriteUVs = m_currentAnimation->animationDef->GetSpriteDefAtTime( m_animationTimer ).GetUVs();

	Vec2 size = m_definition->size * m_scale;
	Vec2 const& pivot = m_definition->pivot;
	AABB2 localBounds(
		Vec2( -size.x * pivot.x, -size.y * pivot.y ),
		Vec2( size.x * (1.f - pivot.x), size.y * (1.f - pivot.y) )
	);

	Mat44 localBasis = Mat44::CreateZRotationDegrees( 90.f );
	localBasis.Append( Mat44::CreateXRotationDegrees( 90.f ) );

	Camera const& camera = m_map->GetPlayerController()->m_camera;
	Mat44 transformMat = GetBillboardMatrix(
		m_definition->billboardType,
		camera.GetTransformMatrix(),
		m_position,
		Vec2::ONE );

	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( transformMat, m_color );
	g_theRenderer->BindTexture( &m_currentAnimation->animationDef->m_spriteSheet.GetTexture() );

	if (m_definition->renderLit || m_definition->renderRounded)
	{
		std::vector<Vertex_PCUTBN> verts;
		std::vector<unsigned int> indexes;
		Vec3 bottomLeft( localBounds.m_mins.x, localBounds.m_mins.y, 0.f );
		Vec3 bottomRight( localBounds.m_maxs.x, localBounds.m_mins.y, 0.f );
		Vec3 topRight( localBounds.m_maxs.x, localBounds.m_maxs.y, 0.f );
		Vec3 topLeft( localBounds.m_mins.x, localBounds.m_maxs.y, 0.f );

		if (m_definition->renderRounded)
		{
			AddVertsForRoundedQuad3D( verts, indexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, spriteUVs );
		}
		else
		{
			AddVertsForQuad3D( verts, indexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, spriteUVs );
		}

		TransformVertexArray3D( verts, localBasis );
		// Full billboards already face the camera; do not re-apply entity pitch (plasma launch pitch was shearing them).
		if (m_definition->billboardType != BillboardType::FULL_FACING &&
			m_definition->billboardType != BillboardType::FULL_OPPOSING)
		{
			TransformVertexArray3D( verts, EulerAngles( 0.f, m_orientation.m_pitchDegrees, 0.f ).GetAsMatrix_IFwd_JLeft_KUp() );
		}

		if (m_definition->shader != nullptr)
		{
			g_theRenderer->BindShader( m_definition->shader );
		}
		else
		{
			g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );
		}
		g_theRenderer->DrawVertexArray( verts, indexes );
	}
	else
	{
		std::vector<Vertex_PCU> verts;
		AddVertsForAABB2D( verts, localBounds, m_color, spriteUVs.m_mins, spriteUVs.m_maxs );
		TransformVertexArray3D( verts, localBasis );
		if (m_definition->billboardType != BillboardType::FULL_FACING &&
			m_definition->billboardType != BillboardType::FULL_OPPOSING)
		{
			TransformVertexArray3D( verts, EulerAngles( 0.f, m_orientation.m_pitchDegrees, 0.f ).GetAsMatrix_IFwd_JLeft_KUp() );
		}

		g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );
		g_theRenderer->DrawVertexArray( verts );
	}
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 mat = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	mat.SetTranslation3D( m_position );
	return mat;
}

EntityUID Entity::GetUID() const
{
	return m_uid;
}

void Entity::SetUID( EntityUID const& uid )
{
	m_uid = uid;
}

void Entity::PossessedBy( Controller* controller )
{
	m_controller = controller;
}

void Entity::Unpossessed()
{
	m_controller = nullptr;
}
