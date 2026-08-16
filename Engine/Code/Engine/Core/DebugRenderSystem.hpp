#pragma once

#include <string>
#include <vector>
#include <mutex>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Renderer;
class BitmapFont;

enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY,
	NUL
};

struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	BitmapFont* font = nullptr;
};

void DebugRenderSystemStartup( DebugRenderConfig const& config );
void DebugRenderSystemShutdown();

void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();
bool DebugRenderIsVisible();

void DebugRenderBeginFrame();
void DebugRenderWorld( Camera const& camera, DebugRenderMode renderingMode );
void DebugRenderScreen( Camera const& camera );
void DebugRenderEndFrame();

void DebugAddWorldPoint( Vec3 const& pos,
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldLine( Vec3 const& start, Vec3 const& end,
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldWireCylinder( bool isWired,
	Vec3 const& base, Vec3 const& top,
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldWireSphere( bool isWired,
	Vec3 const& center,
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldWireCapsule( bool isWired,
	Vec3 const& base, Vec3 const& top,
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldWireBox( bool isWired,
	OBB3 const& bound, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end,
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldText( std::string const& text,
	Mat44 const& transform, float textHeight,
	Vec2 const& alignment, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE );
void DebugAddWorldBillboardText( std::string const& text,
	Vec3 const& origin, float textHeight,
	Vec2 const& alignment, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE );
void DebugAddWorldBasis( Mat44 const& transform, float duration,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddScreenText( std::string const& text,
	Vec2 const& position, float size,
	Vec2 const& alignment, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE );
void DebugAddMessage( std::string const& text,
	float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor = Rgba8::WHITE );

bool Command_DebugRenderClear();
bool Command_DebugRenderToggle(); 