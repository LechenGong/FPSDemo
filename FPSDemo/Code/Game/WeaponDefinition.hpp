#pragma once

#include <string>
#include <vector>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class Shader;
class SpriteSheet;
class Texture;

struct WeaponAttackDefinition
{
	float refireTime = 0.f;

	// Ray
	int rayCount = 0;
	float rayCone = 0.f;
	float rayRange = 0.f;
	float rayDamage = 0.f;
	float rayImpulse = 0.f;

	// Projectile
	int projectileCount = 0;
	std::string projectileActor;
	float projectileSpeed = 0.f;

	// Melee
	int meleeCount = 0;
	float meleeArc = 0.f;
	float meleeRange = 0.f;
	float meleeDamage = 0.f;
	float meleeImpulse = 0.f;
	float damageDelay = 0.f;

	// Charge / duration
	bool doesCharge = false;
	float chargeTime = 0.f; // max charge seconds; chargeFraction = heldTime / chargeTime
	float duration = 0.f;

	int ammoPerShot = 1;
	int chargeAmmoCost = 0; // full-charge ammo drain; 0 = charge does not consume extra
};

struct WeaponHudAnimationDefinition
{
	std::string name;
	std::string shaderName;
	std::string spriteSheetPath;
	IntVec2 cellCount = IntVec2::ZERO;
	float secondsPerFrame = 0.f;
	int startFrame = 0;
	int endFrame = 0;

	Shader* shader = nullptr;
	SpriteSheet* spriteSheet = nullptr;
	SpriteAnimDefinition* animDef = nullptr;
};

struct WeaponHudDefinition
{
	std::string shaderName;
	std::string baseTexturePath;
	std::string reticleTexturePath;
	Vec2 reticleSize = Vec2::ZERO;
	Vec2 spriteSize = Vec2::ZERO;
	Vec2 spritePivot = Vec2::ZERO;

	Shader* shader = nullptr;
	Texture* baseTexture = nullptr;
	Texture* reticleTexture = nullptr;
	std::vector<WeaponHudAnimationDefinition> animations;

	WeaponHudAnimationDefinition const* GetAnimationByName( std::string const& name ) const;
};

struct WeaponSoundDefinition
{
	std::string soundType;
	std::string soundName;
};

struct WeaponDefinition
{
public:
	std::string name;
	int magazineSize = 0;			// 0 = unlimited (no ammo system)
	int startingReserveAmmo = 0;
	float reloadTime = 0.f;

	WeaponAttackDefinition mainAttack;
	bool hasSubAttack = false;
	WeaponAttackDefinition subAttack;

	bool hasHud = false;
	WeaponHudDefinition hud;

	std::vector<WeaponSoundDefinition> sounds;

	WeaponSoundDefinition const* GetSoundByType( std::string const& soundType ) const;

public:
	static void InitializeWeaponDefs( char const* path = "Data/Definitions/WeaponDefinitions.xml" );
	static void DefineWeaponType( XmlElement const& weaponXmlElement );
	static WeaponDefinition* GetWeaponDef( std::string const& weaponName );

	static std::vector<WeaponDefinition*> s_WeaponDefs;
};
