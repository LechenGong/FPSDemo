#pragma once

#include <string>
#include <vector>

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/General/ShapeComponents/ShapeComponent.hpp"
#include "Game/EntityUID.hpp"

class Game;
class Map;
class Texture;
class Controller;
class Shader;
class SpriteSheet;
class Weapon;

enum class EntityState
{
	IDLE,
	WALK,
	ATTACK,
	HURT,
	DEATH,
	COUNT
};

enum class Faction
{
	INVALID = -1,
	MARINE,
	DEMON,
	NEUTRAL,
	COUNT
};

struct ActorSound
{
	std::string soundType;
	std::string soundName;
};

struct ActorAnimation
{
	Vec3 direction = Vec3::ZERO;
	SpriteAnimDefinition* animationDef = nullptr;
};

struct AnimationGroup
{
	std::string name = "";
	bool scaleBySpeed = false;
	SpriteAnimPlaybackType playbackMode = SpriteAnimPlaybackType::ONCE;
	std::vector<ActorAnimation*> directions;
};

struct EntityDefinition
{
public:
	//default group
	std::string name = "";
	std::string controllerName = "";
	Faction faction = Faction::NEUTRAL;
	float health = 0.f;
	bool canBePossessed = false;
	float corpseLifetime = 0.f; // < 0: corpse is never destroyed
	bool visible = false;
	bool dieOnSpawn = false;

	//Collision group
	CollisionShape collisionShape = CollisionShape::CAPSULE;
	float radius = 0.f;
	float height = 0.f; // Capsule: total height including hemispheres; Cube: full height; Sphere: unused
	bool collidesWithWorld = false;
	bool collidesWithEntitys = false;
	float damageOnCollide = 0.f;
	float impulseOnCollide = 0.f;
	bool dieOnCollide = false;

	//Explosion group
	bool doesExplode = false;
	float range = 0.f;
	float damageOnExplosion = 0;
	float impulseOnExplosion = 0;
	std::string explosionAnimationActor = "";

	//Physics group
	bool simulatePhysics = false;
	bool flying = false;
	float walkSpeed = 0.f;
	float runSpeed = 0.f;
	float chargeSpeed = 0.f;
	float turnSpeed = 0.f;
	float drag = 0.f;

	//AI group
	bool aiEnabled = false;
	float sightRadius = 0.f;
	float sightAngle = 0.f;

	//Camera group
	float eyeHeight = 0.f;
	float cameraFOV = 0.f;
	
	//Visual Group
	Vec2 size = Vec2::ZERO;
	Vec2 pivot = Vec2::ZERO;
	BillboardType billboardType = BillboardType::NONE;
	bool renderLit = false;
	bool renderRounded = false;
	Shader* shader = nullptr;
	SpriteSheet* spriteSheet = nullptr;
	IntVec2 cellCount = IntVec2::ZERO;

	std::vector<AnimationGroup*> animations;

	std::vector<ActorSound*> sounds;

	std::vector<std::string> weaponNames;

	ActorAnimation* GetAnimation( AnimationGroup* animationGroup, Vec3 direction ) const;
	AnimationGroup* GetAnimationGroupByName( std::string name ) const;
	ActorSound* GetSoundByType( std::string type ) const;
	
public:
	static void InitializeEntityDefs( char const* path = "Data/Definitions/EntityDefinitions.xml" );
	static void InitializeProjectileEntityDef( char const* path = "Data/Definitions/ProjectileEntityDefinitions.xml" );
	static std::vector<EntityDefinition*> s_EntityDefs;
	static void DefineEntityType( XmlElement const& entityXmlElement );
	static EntityDefinition* GetEntityDef( std::string entityName );
};

struct SpawnInfo
{
	std::string m_entityTypeName;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation;
};

class Entity
{
public:
	Entity();
	explicit Entity( Game* owner );
	Entity( Map* map, EntityDefinition const* definition );
	virtual ~Entity();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void UpdatePhysics( float deltaSeconds );
	void UpdateAnimation();
	void SetAnimationGroup( EntityState state );

	void InitializeCollision();
	void InitializeWeapons();
	void SyncCollisionTransform();
	void ResolveGroundContact();
	void ApplyCollisionDisplacement( Vec3 const& displacement );
	Vec3 GetCollisionBottomPosition() const;
	ShapeComponent* GetCollision() const { return m_collision; }
	bool IsGrounded() const { return m_isGrounded; }
	bool WantsWorldCollision() const;
	bool WantsEntityCollision() const;

	void TakeDamage( float damage );
	void PlayDefinitionSound( std::string const& soundType ) const;
	void AddHealth( float amount );
	void SetInvincible( bool isInvincible ) { m_isInvincible = isInvincible; }
	bool IsInvincible() const { return m_isInvincible; }
	void ApplyHitStun( float durationSeconds );
	void ApplyKnockback( Vec3 const& worldDirection, float distance );
	bool IsHitStunned() const { return m_hitStunTimer > 0.f; }
	void EquipWeapon( int weaponIndex );
	Weapon* GetCurrentWeapon() const;
	void ClearChargingProjectileRef( Entity* destroyedProjectile );
	int GetCurrentWeaponIndex() const { return m_currentWeaponIndex; }
	int GetWeaponCount() const { return (int)m_weapons.size(); }

	void SetSimulatePhysics( bool enabled ) { m_simulatePhysics = enabled; }
	bool IsSimulatePhysics() const { return m_simulatePhysics; }
	void SetScale( float scale );
	float GetScale() const { return m_scale; }

	Mat44 GetModelMatrix() const;

	EntityUID GetUID() const;
	void SetUID( EntityUID const& uid );

	EntityDefinition const* GetDefinition() const { return m_definition; }
	Controller* GetController() const { return m_controller; }
	void PossessedBy( Controller* controller );
	void Unpossessed();

public:
	std::vector<Vertex_PCU> m_vertexes;
	Texture* m_texture = nullptr;
	Game* m_game = nullptr;
	Map* m_map = nullptr;
	EntityDefinition const* m_definition = nullptr;
	EntityUID m_uid;
	Controller* m_controller = nullptr;
	ShapeComponent* m_collision = nullptr;
	std::vector<Weapon*> m_weapons;
	int m_currentWeaponIndex = 0;

	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;

	EntityState m_currentState = EntityState::IDLE;
	float m_animationTimer = 0.f;
	float m_animationPlaybackSpeed = 1.f;
	AnimationGroup* m_currentAnimationGroup = nullptr;
	ActorAnimation* m_currentAnimation = nullptr;

	float m_health = 0.f;
	bool m_isDead = false;
	bool m_isInvincible = false;
	bool m_isGrounded = false;
	bool m_simulatePhysics = false;
	float m_scale = 1.f;
	float m_damageOnCollide = 0.f;
	float m_impulseOnCollide = 0.f;
	float m_corpseTimer = 0.f;
	float m_hitStunTimer = 0.f;
	Vec3 m_previousPosition = Vec3::ZERO;
	Vec3 m_moveIntentXY = Vec3::ZERO;
	bool m_ignoreEmbeddedHits = false;
	Rgba8 m_color = Rgba8::WHITE;
};
