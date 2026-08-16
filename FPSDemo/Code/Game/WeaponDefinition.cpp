#include "Game/WeaponDefinition.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sprite.hpp"

std::vector<WeaponDefinition*> WeaponDefinition::s_WeaponDefs;

WeaponHudAnimationDefinition const* WeaponHudDefinition::GetAnimationByName( std::string const& name ) const
{
	for (WeaponHudAnimationDefinition const& animation : animations)
	{
		if (animation.name == name)
		{
			return &animation;
		}
	}
	return nullptr;
}

static void ParseAttackGroup( XmlElement const& element, WeaponAttackDefinition& attack )
{
	attack.refireTime = ParseXmlAttribute( element, "refireTime", attack.refireTime );

	attack.rayCount = ParseXmlAttribute( element, "rayCount", attack.rayCount );
	attack.rayCone = ParseXmlAttribute( element, "rayCone", attack.rayCone );
	attack.rayRange = ParseXmlAttribute( element, "rayRange", attack.rayRange );
	attack.rayDamage = ParseXmlAttribute( element, "rayDamage", attack.rayDamage );
	attack.rayImpulse = ParseXmlAttribute( element, "rayImpulse", attack.rayImpulse );

	attack.projectileCount = ParseXmlAttribute( element, "projectileCount", attack.projectileCount );
	attack.projectileActor = ParseXmlAttribute( element, "projectileActor", attack.projectileActor );
	attack.projectileSpeed = ParseXmlAttribute( element, "projectileSpeed", attack.projectileSpeed );

	attack.meleeCount = ParseXmlAttribute( element, "meleeCount", attack.meleeCount );
	attack.meleeArc = ParseXmlAttribute( element, "meleeArc", attack.meleeArc );
	attack.meleeRange = ParseXmlAttribute( element, "meleeRange", attack.meleeRange );
	attack.meleeDamage = ParseXmlAttribute( element, "meleeDamage", attack.meleeDamage );
	attack.meleeImpulse = ParseXmlAttribute( element, "meleeImpulse", attack.meleeImpulse );
	attack.damageDelay = ParseXmlAttribute( element, "damageDelay", attack.damageDelay );

	attack.doesCharge = ParseXmlAttribute( element, "doesCharge", attack.doesCharge );
	attack.chargeTime = ParseXmlAttribute( element, "chargeTime", attack.chargeTime );
	attack.duration = ParseXmlAttribute( element, "duration", attack.duration );

	attack.ammoPerShot = ParseXmlAttribute( element, "ammoPerShot", attack.ammoPerShot );
	attack.chargeAmmoCost = ParseXmlAttribute( element, "chargeAmmoCost", attack.chargeAmmoCost );
}

static void ParseHudGroup( XmlElement const& element, WeaponHudDefinition& hud )
{
	hud.shaderName = ParseXmlAttribute( element, "shader", "" );
	hud.baseTexturePath = ParseXmlAttribute( element, "baseTexture", "" );
	hud.reticleTexturePath = ParseXmlAttribute( element, "reticleTexture", "" );
	hud.reticleSize = ParseXmlAttribute( element, "reticleSize", hud.reticleSize );
	hud.spriteSize = ParseXmlAttribute( element, "spriteSize", hud.spriteSize );
	hud.spritePivot = ParseXmlAttribute( element, "spritePivot", hud.spritePivot );

	if (!hud.shaderName.empty())
	{
		hud.shader = g_theRenderer->CreateShader( hud.shaderName.c_str(), VertexType::VERTEX_PCU );
	}
	if (!hud.baseTexturePath.empty())
	{
		hud.baseTexture = g_theRenderer->CreateOrGetTextureFromFile( hud.baseTexturePath.c_str() );
	}
	if (!hud.reticleTexturePath.empty())
	{
		hud.reticleTexture = g_theRenderer->CreateOrGetTextureFromFile( hud.reticleTexturePath.c_str() );
	}

	XmlElement const* animationElem = element.FirstChildElement( "Animation" );
	while (animationElem)
	{
		WeaponHudAnimationDefinition animation;
		animation.name = ParseXmlAttribute( *animationElem, "name", "" );
		animation.shaderName = ParseXmlAttribute( *animationElem, "shader", "" );
		animation.spriteSheetPath = ParseXmlAttribute( *animationElem, "spriteSheet", "" );
		animation.cellCount = ParseXmlAttribute( *animationElem, "cellCount", IntVec2::ZERO );
		animation.secondsPerFrame = ParseXmlAttribute( *animationElem, "secondsPerFrame", 0.f );
		animation.startFrame = ParseXmlAttribute( *animationElem, "startFrame", 0 );
		animation.endFrame = ParseXmlAttribute( *animationElem, "endFrame", 0 );

		std::string const& shaderToUse = !animation.shaderName.empty() ? animation.shaderName : hud.shaderName;
		if (!shaderToUse.empty())
		{
			animation.shader = g_theRenderer->CreateShader( shaderToUse.c_str(), VertexType::VERTEX_PCU );
		}

		if (!animation.spriteSheetPath.empty() && animation.cellCount.x > 0 && animation.cellCount.y > 0)
		{
			Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( animation.spriteSheetPath.c_str() );
			animation.spriteSheet = new SpriteSheet( *texture, animation.cellCount );

			int frameCount = animation.endFrame - animation.startFrame + 1;
			if (frameCount < 1)
			{
				frameCount = 1;
			}
			float durationSeconds = frameCount * animation.secondsPerFrame;
			SpriteAnimPlaybackType playback = (animation.name == "Idle")
				? SpriteAnimPlaybackType::LOOP
				: SpriteAnimPlaybackType::ONCE;

			animation.animDef = new SpriteAnimDefinition(
				*animation.spriteSheet,
				animation.startFrame,
				animation.endFrame,
				durationSeconds,
				playback );
		}

		hud.animations.push_back( animation );
		animationElem = animationElem->NextSiblingElement( "Animation" );
	}
}

static void ParseSoundsGroup( XmlElement const& element, WeaponDefinition& definition )
{
	XmlElement const* soundElem = element.FirstChildElement( "Sound" );
	while (soundElem)
	{
		WeaponSoundDefinition sound;
		sound.soundType = ParseXmlAttribute( *soundElem, "sound", "" );
		sound.soundName = ParseXmlAttribute( *soundElem, "name", "" );
		definition.sounds.push_back( sound );
		soundElem = soundElem->NextSiblingElement( "Sound" );
	}
}

void WeaponDefinition::DefineWeaponType( XmlElement const& weaponXmlElement )
{
	WeaponDefinition* definition = new WeaponDefinition();
	definition->name = ParseXmlAttribute( weaponXmlElement, "name", "" );
	definition->magazineSize = ParseXmlAttribute( weaponXmlElement, "magazineSize", 0 );
	definition->startingReserveAmmo = ParseXmlAttribute( weaponXmlElement, "reserveAmmo", 0 );
	definition->reloadTime = ParseXmlAttribute( weaponXmlElement, "reloadTime", 0.f );

	XmlElement const* childElement = weaponXmlElement.FirstChildElement();
	while (childElement)
	{
		std::string tagName = childElement->Name();
		if (tagName == "MainAttack")
		{
			ParseAttackGroup( *childElement, definition->mainAttack );
		}
		else if (tagName == "SubAttack")
		{
			definition->hasSubAttack = true;
			ParseAttackGroup( *childElement, definition->subAttack );
		}
		else if (tagName == "HUD")
		{
			definition->hasHud = true;
			ParseHudGroup( *childElement, definition->hud );
		}
		else if (tagName == "Sounds")
		{
			ParseSoundsGroup( *childElement, *definition );
		}

		childElement = childElement->NextSiblingElement();
	}

	GUARANTEE_OR_DIE( !definition->name.empty(), "WeaponDefinition missing required name attribute" );
	s_WeaponDefs.push_back( definition );
}

WeaponSoundDefinition const* WeaponDefinition::GetSoundByType( std::string const& soundType ) const
{
	for (WeaponSoundDefinition const& sound : sounds)
	{
		if (sound.soundType == soundType)
		{
			return &sound;
		}
	}
	return nullptr;
}

void WeaponDefinition::InitializeWeaponDefs( char const* path )
{
	if (!s_WeaponDefs.empty())
	{
		return;
	}

	XmlDocument document;
	XmlError result = document.LoadFile( path );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to load weapon definitions: %s", path ).c_str() );

	XmlElement const* root = document.RootElement();
	GUARANTEE_OR_DIE( root != nullptr, Stringf( "Weapon definitions missing root: %s", path ).c_str() );

	XmlElement const* defElement = root->FirstChildElement();
	while (defElement)
	{
		std::string tagName = defElement->Name();
		if (tagName == "WeaponDefinition")
		{
			DefineWeaponType( *defElement );
		}
		defElement = defElement->NextSiblingElement();
	}
}

WeaponDefinition* WeaponDefinition::GetWeaponDef( std::string const& weaponName )
{
	for (WeaponDefinition* definition : s_WeaponDefs)
	{
		if (definition != nullptr && definition->name == weaponName)
		{
			return definition;
		}
	}
	return nullptr;
}
