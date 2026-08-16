#include <vector>

#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
// #include "Engine/Renderer/Camera.hpp"
// #include "Engine/Math/MathUtils.hpp"

std::mutex debugWorldMutex;
std::mutex debugScreenMutex;

DebugRenderConfig g_config = {};

bool g_debugRenderVisible = false;
Camera* g_playerCamera = nullptr;

BillboardType g_billBoardType = BillboardType::FULL_FACING;

struct DebugPoint
{
	Vec3 pos;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugLine
{
	Vec3 start;
	Vec3 end;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugSolidCylinder
{
	Vec3 base;
	Vec3 top;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugWireCylinder
{
	Vec3 base;
	Vec3 top;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugSolidSphere
{
	Vec3 center;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugWireSphere
{
	Vec3 center;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugSolidCapsule
{
	Vec3 base;
	Vec3 top;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugWireCapsule
{
	Vec3 base;
	Vec3 top;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugSolidBox
{
	OBB3 bound;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugWireBox
{
	OBB3 bound;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugArrow
{
	Vec3 start;
	Vec3 end;
	float radius;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> verts;
};

struct DebugWorldText
{
	std::string text;
	Mat44 transform;
	float textHeight;
	Vec2 alignment;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	std::vector<Vertex_PCU> verts;
};

struct DebugBillboardText
{
	std::string text;
	Vec3 origin;
	float textHeight;
	Vec2 alignment;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
	std::vector<Vertex_PCU> verts;
};

struct DebugWorldBasis
{
	Mat44 transform;
	float remain;
	float duration;
	DebugRenderMode mode;
	std::vector<Vertex_PCU> verts;
};

struct DebugScreenText
{
	std::string text;
	Vec2 position;
	float size;
	Vec2 alignment;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
};

struct DebugMessage
{
	std::string text;
	float remain;
	float duration;
	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::WHITE;
};

std::vector<DebugPoint*>			g_debugPoint;
std::vector<DebugLine*>				g_debugLine;
std::vector<DebugSolidCylinder*>	g_debugSolidCylinder;
std::vector<DebugWireCylinder*>		g_debugWireCylinder;
std::vector<DebugSolidSphere*>		g_debugSolidSphere;
std::vector<DebugWireSphere*>		g_debugWireSphere;
std::vector<DebugSolidCapsule*>		g_debugSolidCapsule;
std::vector<DebugWireCapsule*>		g_debugWireCapsule;
std::vector<DebugSolidBox*>			g_debugSolidBox;
std::vector<DebugWireBox*>			g_debugWireBox;
std::vector<DebugArrow*>			g_debugArrow;
std::vector<DebugWorldText*>		g_debugWorldText;
std::vector<DebugBillboardText*>	g_debugBillboardText;
std::vector<DebugWorldBasis*>		g_debugWorldBasis;
std::vector<DebugScreenText*>		g_debugScreenText;
std::vector<DebugMessage*>			g_debugMessage;

void DebugRenderSystemStartup( DebugRenderConfig const& config )
{
	g_config = config;

	g_debugPoint.reserve( 10 );
	g_debugLine.reserve( 10 );
	g_debugWireCylinder.reserve( 10 );
	g_debugWireSphere.reserve( 10 );
	g_debugArrow.reserve( 10 );
	g_debugWorldText.reserve( 10 );
	g_debugBillboardText.reserve( 10 );
	g_debugScreenText.reserve( 10 );
	g_debugMessage.reserve( 10 );

	g_eventSystem->SubscribeEventCallBackFunc( "DRClear", reinterpret_cast<void(*)()>(Command_DebugRenderClear), 1 );
	g_eventSystem->SubscribeEventCallBackFunc( "DRToggle", reinterpret_cast<void(*)()>(Command_DebugRenderToggle), 1 );
}

void DebugRenderSystemShutdown()
{
	DebugRenderSetHidden();
	DebugRenderClear();

	g_eventSystem->UnsubscribeEventCallbackFunc( "DRClear", reinterpret_cast<void(*)()>(Command_DebugRenderClear) );
	g_eventSystem->UnsubscribeEventCallbackFunc( "DRToggle", reinterpret_cast<void(*)()>(Command_DebugRenderToggle) );
}

void DebugRenderSetVisible()
{
	g_debugRenderVisible = true;
}

void DebugRenderSetHidden()
{
	g_debugRenderVisible = false;
}

void DebugRenderClear()
{
	g_debugPoint.clear();
	g_debugLine.clear();
	g_debugSolidCylinder.clear();
	g_debugWireCylinder.clear();
	g_debugSolidSphere.clear();
	g_debugWireSphere.clear();
	g_debugArrow.clear();
	g_debugWorldText.clear();
	g_debugBillboardText.clear();
	g_debugScreenText.clear();
	g_debugMessage.clear();
}

bool DebugRenderIsVisible()
{
	return g_debugRenderVisible;
}

void DebugRenderBeginFrame()
{
}

void DebugRenderWorld( Camera const& camera, DebugRenderMode renderingMode )
{
	g_playerCamera = const_cast<Camera*>(&camera);
	if (g_debugRenderVisible == false)
		return;


	debugWorldMutex.lock();

	float deltaSeconds = Clock::s_systemClock.GetDeltaSeconds();
	g_config.m_renderer->BeginCamera( camera );
	g_config.m_renderer->BindShader( g_config.m_renderer->CreateShader( "Data/Shaders/Default.hlsl", VertexType::VERTEX_PCU ) );

	{
		for (DebugPoint*& shape : g_debugPoint)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugLine*& shape : g_debugLine)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugSolidCylinder*& shape : g_debugSolidCylinder)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugWireCylinder*& shape : g_debugWireCylinder)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->remain == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugSolidSphere*& shape : g_debugSolidSphere)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugWireSphere*& shape : g_debugWireSphere)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
				g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugSolidCapsule*& shape : g_debugSolidCapsule)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugWireCapsule*& shape : g_debugWireCapsule)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->remain == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugSolidBox*& shape : g_debugSolidBox)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugWireBox*& shape : g_debugWireBox)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->remain == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugArrow*& shape : g_debugArrow)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					Rgba8 tColor = color;
					tColor.a = 45;
					g_config.m_renderer->SetModelConstants( Mat44(), tColor );
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );

					g_config.m_renderer->SetModelConstants( Mat44(), color );
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		BitmapFont* font = g_config.m_renderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
		for (DebugWorldText*& shape : g_debugWorldText)
		{
			if (shape == nullptr)
				continue;
			if (renderingMode != DebugRenderMode::NUL)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				g_config.m_renderer->SetModelConstants( Mat44(), color );
				g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
				g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
				g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
				g_config.m_renderer->BindTexture( &font->GetTexture() );
				g_config.m_renderer->DrawVertexArray( shape->verts );
				shape->remain -= deltaSeconds;
				g_config.m_renderer->BindTexture( nullptr );
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}
	
	{
		BitmapFont* font = g_config.m_renderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
		for (DebugBillboardText*& shape : g_debugBillboardText)
		{
			if (shape == nullptr)
				continue;
			if (renderingMode != DebugRenderMode::NUL)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				Mat44 tranform = GetBillboardMatrix( g_billBoardType, g_playerCamera->GetTransformMatrix(), shape->origin, Vec2::ONE );
				
				g_config.m_renderer->SetModelConstants( tranform, color );
				g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
				g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
				g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
				g_config.m_renderer->BindTexture( &font->GetTexture() );
				g_config.m_renderer->DrawVertexArray( shape->verts );
				shape->remain -= deltaSeconds;
				g_config.m_renderer->BindTexture( nullptr );
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	{
		for (DebugWorldBasis*& shape : g_debugWorldBasis)
		{
			if (shape == nullptr)
				continue;
			if (shape->mode != renderingMode)
				continue;

			if (shape->duration == -1.f || shape->remain > 0.f)
			{
				switch (renderingMode)
				{
				case DebugRenderMode::ALWAYS:
				{
					g_config.m_renderer->SetModelConstants();
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::USE_DEPTH:
				{
					g_config.m_renderer->SetModelConstants();
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::NUL:
				{
					g_config.m_renderer->SetModelConstants();
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				case DebugRenderMode::X_RAY:
				{
					std::vector<Vertex_PCU> tVerts = shape->verts;
					for (Vertex_PCU& vert : tVerts)
					{
						vert.m_color.a = 45;
					}
					g_config.m_renderer->SetModelConstants();
					g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
					g_config.m_renderer->SetDepthMode( DepthMode::DISABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( tVerts );

					g_config.m_renderer->SetModelConstants();
					g_config.m_renderer->SetBlendMode( BlendMode::OPAQUE );
					g_config.m_renderer->SetDepthMode( DepthMode::ENABLED );
					g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
					g_config.m_renderer->BindTexture( nullptr );
					g_config.m_renderer->DrawVertexArray( shape->verts );
					break;
				}
				}
				shape->remain -= deltaSeconds;
			}
			else if (shape->duration != -1.f && shape->remain <= 0)
			{
				delete shape;
				shape = nullptr;
			}
		}
	}

	g_config.m_renderer->EndCamera( camera );

	debugWorldMutex.unlock();
}

void DebugRenderScreen( Camera const& camera )
{
	if (g_debugRenderVisible == false)
		return;

	debugScreenMutex.lock();

	float deltaSeconds = Clock::s_systemClock.GetDeltaSeconds();
	BitmapFont* font = g_config.m_renderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	g_config.m_renderer->BeginCamera( camera );

	{
		std::vector<Vertex_PCU> verts;
		int counter = 0;
		for (DebugScreenText*& shape : g_debugScreenText)
		{
			if (shape == nullptr)
				continue;

			if (shape->duration == -1.f)
			{
				float boxWidth = static_cast<float>(shape->text.length()) * shape->size * 0.7f;
				float boxHeight = shape->size;
				Vec2 pivot = shape->position - Vec2( boxWidth * (1.f - shape->alignment.x), boxHeight * (1.f - shape->alignment.y) ) - Vec2( 0.f, counter * boxHeight );
				font->AddVertsForText2D( verts, pivot, shape->size, shape->text, shape->startColor, 0.7f );
			}
			else if (shape->remain >= 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				float boxWidth = static_cast<float>(shape->text.length()) * shape->size * 0.7f;
				float boxHeight = shape->size;
				Vec2 pivot = shape->position - Vec2( boxWidth * (1.f - shape->alignment.x), boxHeight * (1.f - shape->alignment.y) ) - Vec2( 0.f, counter * boxHeight );
				font->AddVertsForText2D( verts, pivot, shape->size, shape->text, shape->startColor, 0.7f );
			}
			else
			{
				delete shape;
				shape = nullptr;
				continue;
			}
			shape->remain -= deltaSeconds;
			counter++;
		}
		g_config.m_renderer->SetModelConstants();
		g_config.m_renderer->SetLightingConstants( Vec3( 2.f, 1.f, -1.f ).GetNormalized(), 1.f, 1.f );
		g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
		g_config.m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
		g_config.m_renderer->BindTexture( &font->GetTexture() );
		g_config.m_renderer->DrawVertexArray( verts );
	}

	{
		std::vector<Vertex_PCU> verts;
		int counter = 0;
		for (DebugMessage*& shape : g_debugMessage)
		{
			if (shape == nullptr)
				continue;

			float height = 20.f;

			if (shape->duration == -1.f)
			{
				//float boxWidth = static_cast<float>(shape->text.length()) * height * 0.7f;
				float boxHeight = height;
				Vec2 pivot = Vec2(0.f, camera.GetBounds().m_maxs.y - camera.GetBounds().m_mins.y) - Vec2( 0.f, boxHeight ) - Vec2( 0.f, counter * boxHeight );
				font->AddVertsForText2D( verts, pivot, height, shape->text, shape->startColor, 0.7f );
			}
			else if (shape->remain >= 0.f)
			{
				float fraction = Clamp( (shape->duration - shape->remain) / shape->duration, 0.f, 1.f );
				Rgba8 startColor = shape->startColor;
				Rgba8 endColor = shape->endColor;
				Rgba8 color(
					static_cast<unsigned char>((int)startColor.r + (int)(((float)endColor.r - (float)startColor.r) * fraction)),
					static_cast<unsigned char>((int)startColor.g + (int)(((float)endColor.g - (float)startColor.g) * fraction)),
					static_cast<unsigned char>((int)startColor.b + (int)(((float)endColor.b - (float)startColor.b) * fraction)),
					static_cast<unsigned char>((int)startColor.a + (int)(((float)endColor.a - (float)startColor.a) * fraction))
				);
				//float boxWidth = static_cast<float>(shape->text.length()) * height * 0.7f;
				float boxHeight = height;
				Vec2 pivot = Vec2( 0.f, camera.GetBounds().m_maxs.y - camera.GetBounds().m_mins.y ) - Vec2( 0.f, boxHeight ) - Vec2( 0.f, counter * boxHeight );
				font->AddVertsForText2D( verts, pivot, height, shape->text, shape->startColor, 0.7f );
			}
			else
			{
				delete shape;
				shape = nullptr;
				continue;
			}
			shape->remain -= deltaSeconds;
			counter++;
		}
		g_config.m_renderer->SetModelConstants();
		g_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
		g_config.m_renderer->BindTexture( &font->GetTexture() );
		g_config.m_renderer->DrawVertexArray( verts );
	}

	g_config.m_renderer->EndCamera( camera );

	debugScreenMutex.unlock();
}

void DebugRenderEndFrame()
{
}

void DebugAddWorldPoint( Vec3 const& pos, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	DebugPoint* newPoint = new DebugPoint{ pos, radius, duration, duration, startColor, endColor, mode };
	newPoint->verts.reserve( 1000 );
	AddVertsForSphere3D( newPoint->verts, pos, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16, 8 );
	for (DebugPoint*& shape : g_debugPoint)
	{
		if (shape == nullptr)
		{
			shape = newPoint;
			return;
		}
	}
	debugWorldMutex.lock();
	g_debugPoint.push_back( newPoint );
	debugWorldMutex.unlock();
}

void DebugAddWorldLine( Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	DebugLine* newLine = new DebugLine{ start, end, radius, duration, duration, startColor, endColor, mode };
	newLine->verts.reserve( 200 );
	AddVertsForCylinder3D( newLine->verts, start, end, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16 );
	for (DebugLine*& shape : g_debugLine)
	{
		if (shape == nullptr)
		{
			shape = newLine;
			return;
		}
	}
	debugWorldMutex.lock();
	g_debugLine.push_back( newLine );
	debugWorldMutex.unlock();
}

void DebugAddWorldWireCylinder( bool isWired, Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	if (isWired)
	{
		DebugWireCylinder* newWireCylinder = new DebugWireCylinder{ base, top, radius, duration, duration, startColor, endColor, mode };
		newWireCylinder->verts.reserve( 200 );
		AddVertsForCylinder3D( newWireCylinder->verts, base, top, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16 );
		for (DebugWireCylinder*& shape : g_debugWireCylinder)
		{
			if (shape == nullptr)
			{
				shape = newWireCylinder;
				return;
			}
		}
		debugWorldMutex.lock();
		g_debugWireCylinder.push_back( newWireCylinder );
		debugWorldMutex.unlock();
	}
	else
	{
		DebugSolidCylinder* newSolidCylinder = new DebugSolidCylinder{ base, top, radius, duration, duration, startColor, endColor, mode };
		newSolidCylinder->verts.reserve( 200 );
		AddVertsForCylinder3D( newSolidCylinder->verts, base, top, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16 );
		for (DebugSolidCylinder*& shape : g_debugSolidCylinder)
		{
			if (shape == nullptr)
			{
				shape = newSolidCylinder;
				return;
			}
		}
		debugWorldMutex.lock();
		g_debugSolidCylinder.push_back( newSolidCylinder );
		debugWorldMutex.unlock();
	}
}

void DebugAddWorldWireSphere( bool isWired, Vec3 const& center, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	if (isWired)
	{
		DebugWireSphere* newWireSphere = new DebugWireSphere{ center, radius, duration, duration, startColor, endColor, mode };
		newWireSphere->verts.reserve( 1000 );
		AddVertsForSphere3D( newWireSphere->verts, center, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 8, 4 );
		for (DebugWireSphere*& shape : g_debugWireSphere)
		{
			if (shape == nullptr)
			{
				shape = newWireSphere;
				return;
			}
		}
		debugWorldMutex.lock();
		g_debugWireSphere.push_back( newWireSphere );
		debugWorldMutex.unlock();
	}
	else
	{
		DebugSolidSphere* newSolidSphere = new DebugSolidSphere{ center, radius, duration, duration, startColor, endColor, mode };
		newSolidSphere->verts.reserve( 1000 );
		AddVertsForSphere3D( newSolidSphere->verts, center, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 8, 4 );
		for (DebugSolidSphere*& shape : g_debugSolidSphere)
		{
			if (shape == nullptr)
			{
				shape = newSolidSphere;
				return;
			}
		}
		debugWorldMutex.lock();
		g_debugSolidSphere.push_back( newSolidSphere );
		debugWorldMutex.unlock();
	}
}

void DebugAddWorldWireCapsule( bool isWired, Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	if (isWired)
	{
		// Create a new wireframe capsule
		DebugWireCapsule* newWireCapsule = new DebugWireCapsule{ base, top, radius, duration, duration, startColor, endColor, mode };
		newWireCapsule->verts.reserve( 200 );

		// Add the vertices for the capsule's wireframe (similar to capsule)
		AddVertsForCapsule3D( newWireCapsule->verts, base, top, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 8 );

		// Look for the first empty slot in the global debug capsule list
		for (DebugWireCapsule*& shape : g_debugWireCapsule)
		{
			if (shape == nullptr)
			{
				shape = newWireCapsule;
				return;
			}
		}

		// Lock the mutex and push the new capsule into the global list
		debugWorldMutex.lock();
		g_debugWireCapsule.push_back( newWireCapsule );
		debugWorldMutex.unlock();
	}
	else
	{
		// Create a new solid capsule
		DebugSolidCapsule* newSolidCapsule = new DebugSolidCapsule{ base, top, radius, duration, duration, startColor, endColor, mode };
		newSolidCapsule->verts.reserve( 200 );

		// Add the vertices for the capsule (solid)
		AddVertsForCapsule3D( newSolidCapsule->verts, base, top, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 8 );

		// Look for the first empty slot in the global solid capsule list
		for (DebugSolidCapsule*& shape : g_debugSolidCapsule)
		{
			if (shape == nullptr)
			{
				shape = newSolidCapsule;
				return;
			}
		}

		// Lock the mutex and push the new capsule into the global list
		debugWorldMutex.lock();
		g_debugSolidCapsule.push_back( newSolidCapsule );
		debugWorldMutex.unlock();
	}
}

void DebugAddWorldWireBox( bool isWired, OBB3 const& bound, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	if (isWired)
	{
		// Create a new wireframe box
		DebugWireBox* newWireBox = new DebugWireBox{ bound, duration, duration, startColor, endColor, mode };
		newWireBox->verts.reserve( 200 );

		// Add the vertices for the box's wireframe (similar to box)
		AddVertsForOBB3D( newWireBox->verts, bound, Rgba8::WHITE, AABB2::ZERO_TO_ONE );

		// Look for the first empty slot in the global debug box list
		for (DebugWireBox*& shape : g_debugWireBox)
		{
			if (shape == nullptr)
			{
				shape = newWireBox;
				return;
			}
		}

		// Lock the mutex and push the new box into the global list
		debugWorldMutex.lock();
		g_debugWireBox.push_back( newWireBox );
		debugWorldMutex.unlock();
	}
	else
	{
		// Create a new solid box
		DebugSolidBox* newSolidBox = new DebugSolidBox{ bound, duration, duration, startColor, endColor, mode };
		newSolidBox->verts.reserve( 200 );

		// Add the vertices for the box (solid)
		AddVertsForOBB3D( newSolidBox->verts, bound, Rgba8::WHITE, AABB2::ZERO_TO_ONE );

		// Look for the first empty slot in the global solid box list
		for (DebugSolidBox*& shape : g_debugSolidBox)
		{
			if (shape == nullptr)
			{
				shape = newSolidBox;
				return;
			}
		}

		// Lock the mutex and push the new box into the global list
		debugWorldMutex.lock();
		g_debugSolidBox.push_back( newSolidBox );
		debugWorldMutex.unlock();
	}
}

void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	DebugArrow* newArrow = new DebugArrow{ start, end, radius, duration, duration, startColor, endColor, mode };
	newArrow->verts.reserve( 400 );
	Vec3 coneStart = end - (end - start) * 0.3f;
	AddVertsForCone3D( newArrow->verts, coneStart, end, radius * 2.f, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16 );
	AddVertsForCylinder3D( newArrow->verts, start, coneStart, radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16 );
	for (DebugArrow*& shape : g_debugArrow)
	{
		if (shape == nullptr)
		{
			shape = newArrow;
			return;
		}
	}
	debugWorldMutex.lock();
	g_debugArrow.push_back( newArrow );
	debugWorldMutex.unlock();
}

void DebugAddWorldText( std::string const& text, Mat44 const& transform, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	DebugWorldText* newWorldText = new DebugWorldText{ text, transform, textHeight, alignment, duration, duration, startColor, endColor };
	newWorldText->verts.reserve( text.size() * 6 );
	BitmapFont* font = g_config.m_renderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	font->AddvertsForText3DAtOriginXForward( newWorldText->verts, textHeight, text, startColor, 0.7f, alignment );
	TransformVertexArray3D( newWorldText->verts, transform );
	for (DebugWorldText*& shape : g_debugWorldText)
	{
		if (shape == nullptr)
		{
			shape = newWorldText;
			return;
		}
	}
	debugWorldMutex.lock();
	g_debugWorldText.push_back( newWorldText );
	debugWorldMutex.unlock();
}

void DebugAddWorldBillboardText( std::string const& text, Vec3 const& origin, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	DebugBillboardText* newBillboardText = new DebugBillboardText{ text, origin, textHeight, alignment, duration, duration, startColor, endColor };
	newBillboardText->verts.reserve( text.size() * 6 );
	BitmapFont* font = g_config.m_renderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	font->AddvertsForText3DAtOriginXForward( newBillboardText->verts, textHeight, text, startColor, 0.7f, alignment );
	for (DebugBillboardText*& shape : g_debugBillboardText)
	{
		if (shape == nullptr)
		{
			shape = newBillboardText;
			return;
		}
	}
	debugWorldMutex.lock();
	g_debugBillboardText.push_back( newBillboardText );
	debugWorldMutex.unlock();
}

void DebugAddWorldBasis( Mat44 const& transform, float duration, DebugRenderMode mode )
{
	DebugWorldBasis* newWorldBasis = new DebugWorldBasis{ transform, duration, duration, mode };
	newWorldBasis->verts.reserve( 1200 );
	Mat44 mat = transform;
	Vec3 origin = mat.GetTranslation3D();
	AddVertsForCylinder3D( newWorldBasis->verts, origin, origin + mat.GetIBasis3D() * 0.7f, 0.12f, Rgba8::RED, AABB2::ZERO_TO_ONE, 16 );
	AddVertsForCone3D( newWorldBasis->verts, origin + mat.GetIBasis3D() * 0.7f, origin + mat.GetIBasis3D(), 0.2f, Rgba8::RED, AABB2::ZERO_TO_ONE, 16 );
	AddVertsForCylinder3D( newWorldBasis->verts, origin, origin + mat.GetJBasis3D() * 0.7f, 0.12f, Rgba8::GREEN, AABB2::ZERO_TO_ONE, 16 );
	AddVertsForCone3D( newWorldBasis->verts, origin + mat.GetJBasis3D() * 0.7f, origin + mat.GetJBasis3D(), 0.2f, Rgba8::GREEN, AABB2::ZERO_TO_ONE, 16 );
	AddVertsForCylinder3D( newWorldBasis->verts, origin, origin + mat.GetKBasis3D() * 0.7f, 0.12f, Rgba8::BLUE, AABB2::ZERO_TO_ONE, 16 );
	AddVertsForCone3D( newWorldBasis->verts, origin + mat.GetKBasis3D() * 0.7f, origin + mat.GetKBasis3D(), 0.2f, Rgba8::BLUE, AABB2::ZERO_TO_ONE, 16 );
	for (DebugWorldBasis*& shape : g_debugWorldBasis)
	{
		if (shape == nullptr)
		{
			shape = newWorldBasis;
			return;
		}
	}
	debugWorldMutex.lock();
	g_debugWorldBasis.push_back( newWorldBasis );
	debugWorldMutex.unlock();
}

void DebugAddScreenText( std::string const& text, Vec2 const& position, float size, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	DebugScreenText* newScreenText = new DebugScreenText{ text, position, size, alignment, duration, duration, startColor, endColor };
// 	for (DebugScreenText*& shape : g_debugScreenText)
// 	{
// 		if (shape == nullptr)
// 		{
// 			shape = newScreenText;
// 			return;
// 		}
// 	}
	debugScreenMutex.lock();
	g_debugScreenText.push_back( newScreenText );
	debugScreenMutex.unlock();
}

void DebugAddMessage( std::string const& text, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	DebugMessage* newDebugMessage = new DebugMessage{ text, duration, duration, startColor, endColor };
// 	for (DebugMessage*& shape : g_debugMessage)
// 	{
// 		if (shape == nullptr)
// 		{
// 			shape = newDebugMessage;
// 			return;
// 		}
// 	}
	debugScreenMutex.lock();
	g_debugMessage.push_back( newDebugMessage );
	debugScreenMutex.unlock();
}

bool Command_DebugRenderClear()
{
	g_debugPoint.clear();
	g_debugLine.clear();
	g_debugSolidCylinder.clear();
	g_debugWireCylinder.clear();
	g_debugSolidSphere.clear();
	g_debugWireSphere.clear();
	g_debugArrow.clear();
	g_debugWorldText.clear();
	g_debugBillboardText.clear();
	g_debugScreenText.clear();
	g_debugMessage.clear();
	return true;
}

bool Command_DebugRenderToggle()
{
	g_debugRenderVisible = !g_debugRenderVisible;
	return true;
}