#pragma once
#pragma warning( disable : 26451 26812 26495 )

#define UNUSED(x) (void)(x);

#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

extern NamedStrings g_gameConfig; 
extern EventSystem* g_eventSystem;
extern DevConsole* g_devConsole;
extern JobSystem* g_jobSystem;
extern NetSystem* g_netSystem;
extern Renderer* g_theRenderer;

// Engine math helper (not for weapon damage ranges)
typedef Vec2 FloatRange;
bool DoFloatRangeOverlap( FloatRange A, FloatRange B );

static float TARGET_FRAMERATE = 60.f;

// double k = GetCurrentTimeSeconds();
// DebugAddMessage( Stringf( "%f", GetCurrentTimeSeconds() - k ), 1.f, Rgba8::YELLOW, Rgba8::YELLOW );