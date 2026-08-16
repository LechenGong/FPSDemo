#pragma once

#include <vector>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
// #include "Engine/Input/InputSystem.hpp"
//#include "Engine/Math/RandomNumberGenerator.hpp"

class Renderer;
class App;
class RandomNumberGenerator;
class InputSystem;
class VertexMap;
class AudioSystem;
class Window;
class VertexMap;
class Texture;
class SpriteSheet;

enum TextureList
{
	TEXTURE_INVALID = -1,
	TEXTURE_ACTOR_MARINE,
	TEXTURE_ACTOR_PINKY,
	TEXTURE_HUD_BASE,
	TEXTURE_PLASMA,
	TEXTURE_PROJECTILE_BLOODSPLATTER,
	TEXTURE_PROJECTILE_PISTOLHIT,
	TEXTURE_PROJECTILE_PLASMA,
	TEXTURE_PROJECTILE_PLASMAHIT,
	TEXTURE_PROJECTILE_FIREBALL,
	TEXTURE_EXPLOSION,
	TEXTURE_GROUNDCRACK,
	TEXTURE_RETICLE,
	TEXTURE_SPRITESHEET_TERRAIN,
	TEXTURE_TEST_UV,
	TEXTURE_WEAPON_PISTOL,
	TEXTURE_WEAPON_PLASMA,
	TEXTURE_HUD_AMMO,
	TEXTURE_HUD_HEART,
	TEXTURE_HUD_COIN,
	TEXTURE_COUNT
};

enum AudioList
{
	AUDIO_CLICK,
	AUDIO_WEAPON_SWITCH,
	AUDIO_COUNT
};

extern Renderer* g_theRenderer;
extern App* g_theApp;
extern RandomNumberGenerator* g_theRNG;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern VertexMap* g_theMap;

extern Texture* g_textures[TEXTURE_COUNT];
extern SoundID g_soundIds[AUDIO_COUNT];
extern SpriteSheet* g_terrainSprite;

constexpr float ENTITY_GRAVITY_Z = -10.f;
constexpr float ENTITY_GROUND_RAY_LENGTH = 5.f;
constexpr float ENTITY_GROUND_LAND_DISTANCE = 0.02f;
constexpr float ENTITY_JUMP_SPEED = 2.5f;
constexpr float ENTITY_HIT_STUN_SECONDS = 0.2f;
constexpr float ENTITY_MELEE_KNOCKBACK_DISTANCE = 1.0f;	// Plasma SubAttack bayonet only

constexpr int MAP_MONEY_PER_KILL = 5;
constexpr int SHOP_HEALTH_COST = 100;
constexpr int SHOP_HEALTH_RESTORE = 20;
constexpr int SHOP_AMMO_COST = 80;
constexpr int SHOP_AMMO_MAGAZINES = 2;

void LoadAsset();
void LoadUiSounds();
void PlayUiClick();
float GetSfxVolume();