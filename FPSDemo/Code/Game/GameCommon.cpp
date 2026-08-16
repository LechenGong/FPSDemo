#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/IntVec2.hpp"

Texture* g_textures[TEXTURE_COUNT] = {};
SoundID g_soundIds[AUDIO_COUNT] = {};
SpriteSheet* g_terrainSprite = nullptr;

void LoadUiSounds()
{
	if (g_theAudio == nullptr)
	{
		return;
	}

	std::string clickPath = App::g_gameConfig.buttonClickSound;
	if (clickPath.empty())
	{
		clickPath = "Data/Audio/Click.mp3";
	}
	g_soundIds[AUDIO_CLICK] = g_theAudio->CreateOrGetSound( clickPath );
	g_soundIds[AUDIO_WEAPON_SWITCH] = g_theAudio->CreateOrGetSound( "Data/Audio/PickupWeapon.wav" );
}

float GetSfxVolume()
{
	return App::g_gameConfig.sfxVolume;
}

void PlayUiClick()
{
	if (g_theAudio == nullptr || g_soundIds[AUDIO_CLICK] == MISSING_SOUND_ID)
	{
		return;
	}

	g_theAudio->StartSound( g_soundIds[AUDIO_CLICK], false, GetSfxVolume() );
}

void LoadAsset()
{
	g_textures[TEXTURE_ACTOR_MARINE] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Actor_Marine_7x9.png" );
	g_textures[TEXTURE_ACTOR_PINKY] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Actor_Pinky_8x9.png" );
	g_textures[TEXTURE_HUD_BASE] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Hud_Base.png" );
	g_textures[TEXTURE_PLASMA] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Plasma.png" );
	g_textures[TEXTURE_PROJECTILE_BLOODSPLATTER] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Projectile_BloodSplatter.png" );
	g_textures[TEXTURE_PROJECTILE_PISTOLHIT] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Projectile_PistolHit.png" );
	g_textures[TEXTURE_PROJECTILE_PLASMA] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Projectile_Plasma.png" );
	g_textures[TEXTURE_PROJECTILE_PLASMAHIT] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Projectile_PlasmaHit.png" );
	g_textures[TEXTURE_PROJECTILE_FIREBALL] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Projectile_Fireball.png" );
	g_textures[TEXTURE_EXPLOSION] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion.png" );
	g_textures[TEXTURE_GROUNDCRACK] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/GroundCrack.png" );
	g_textures[TEXTURE_RETICLE] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Reticle.png" );
	g_textures[TEXTURE_SPRITESHEET_TERRAIN] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	g_textures[TEXTURE_TEST_UV] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	g_textures[TEXTURE_WEAPON_PISTOL] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Weapon_Pistol.png" );
	g_textures[TEXTURE_WEAPON_PLASMA] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Weapon_Plasma.png" );
	g_textures[TEXTURE_HUD_AMMO] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/ammo.jpg" );
	g_textures[TEXTURE_HUD_HEART] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/heart.png" );
	g_textures[TEXTURE_HUD_COIN] = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/coin.jpg" );

	if (g_terrainSprite == nullptr)
	{
		g_terrainSprite = new SpriteSheet( *g_textures[TextureList::TEXTURE_SPRITESHEET_TERRAIN], IntVec2( 8, 8 ) );
	}

	LoadUiSounds();
}
