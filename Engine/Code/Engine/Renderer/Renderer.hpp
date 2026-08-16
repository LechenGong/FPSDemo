#pragma once

#include <vector>

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/EngineBuildPreferences.hpp"


#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#define DX_SAFE_RELEASE(dxObject)		\
{										\
	if ((dxObject) != nullptr)			\
	{									\
		(dxObject)->Release();			\
		(dxObject) = nullptr;			\
	}									\
}	

#if defined(OPAQUE)
#undef OPAQUE
#endif

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilState;

class Window;
class Shader;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Image;
struct Joint;

constexpr int k_blurDownTextureCount = 4;
const int k_blurUpTextureCount = k_blurDownTextureCount - 1;

struct LightingConstants
{
	float sunDirection[3];
	float sunIntensity;
	float ambientIntensity;
	float worldEyePosition[3] = { 0.f, 0.f, 0.f };

	float minFalloff = 0.0f;
	float maxFalloff = 0.1f;
	float minFalloffMultiplier = 0.0f;
	float maxFalloffMultiplier = 1.0f;

	int RenderAmbient = true;
	int RenderDiffuse = true;
	int RenderSpecular = true;
	int RenderEmissive = true;
	int UseDiffuseMap = true;
	int UseNormalMap = true;
	int UseSpecularMap = true;
	int UseGlossinessMap = true;
	int UseEmissiveMap = true;
	float padding[3];
};

enum class BlendMode
{
	OPAQUE,
	ALPHA,
	ADDITIVE,
	COUNT
};

enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WARP,
	BILINEAR_CLAMP,
	COUNT
};

enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	SOLID_CULL_FRONT,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	ENABLED,
	COUNT
};

enum class VertexType
{
	VERTEX_PCU,
	VERTEX_PCUTBN,
	VERTEX_ANIM,
	COUNT
};

struct RenderConfig
{
	Window* m_window = nullptr;
	bool m_emissiveEnabled = false;
};

class Renderer
{
public:
	Renderer();
	Renderer( RenderConfig const& config );
	~Renderer();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void DrawVertexArray( int numVertexes, Vertex_PCU const* vertexArray );
	void DrawVertexArray( std::vector<Vertex_PCU> const vertexArray );
	void DrawVertexArray( std::vector<Vertex_PCU> const vertexArray, std::vector<unsigned int> const IndexArray );
	void DrawVertexArray( VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, std::vector<Vertex_PCU> const vertexArray, std::vector<unsigned int> const indexArray );
	void DrawVertexArray( std::vector<Vertex_PCUTBN> const vertexArray, std::vector<unsigned int> const indexArray );
	void DrawVertexArray( VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, std::vector<Vertex_PCUTBN> const vertexArray, std::vector<unsigned int> const indexArray );
	void ClearScreen( Rgba8 const& defaultColor );
	void BeginCamera( Camera const& camera );
	void EndCamera( Camera const& camera );

	void* m_apiRenderingContext = nullptr;

	Shader* CreateShader( char const* shaderName, VertexType vertexType = VertexType::VERTEX_PCU );
	Shader* CreateShader( char const* shaderName, char const* shaderSource, VertexType vertexType = VertexType::VERTEX_PCU );
	void CompileShaderToByteCode( std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target );
	void BindShader( Shader* shader );

	VertexBuffer* CreateVertexBuffer( size_t const size );
	IndexBuffer* CreateIndexBuffer( size_t const size );
	void CopyCPUToGPU( void const* data, size_t const size, VertexBuffer*& vbo );
	void CopyCPUToGPU( void const* data, size_t const size, IndexBuffer*& ibo );
	void CopyCPUToGPU( void const* dataV, size_t const sizeV, VertexBuffer*& vbo, void const* dataI, size_t const sizeI, IndexBuffer*& ibo );
	void CopyCPUToGPU( void const* dataV, size_t const sizeV, VertexBuffer*& vbo, void const* dataV2, size_t const sizeV2, VertexBuffer*& vbo2, void const* dataI, size_t const sizeI, IndexBuffer*& ibo );
	void BindVertexBuffer( VertexBuffer* vbo, VertexType vertexType = VertexType::VERTEX_PCU );
	void BindVertexBuffer( VertexBuffer* vbo, VertexBuffer* vbo2 );
	void BindIndexBuffer( IndexBuffer* ibo );

	ConstantBuffer* CreateConstantBuffer( size_t const size );
	void CopyCPUToGPU( void const* data, size_t const size, ConstantBuffer*& cbo );
	void BindConstantBuffer( int slot, ConstantBuffer* cbo );

	void DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset = 0 );
	void DrawVertexAndIndexBuffer( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, VertexType vertexType );
	void DrawVertexAndIndexBuffer( VertexBuffer* vbo, VertexBuffer* vbo2, IndexBuffer* ibo, int indexCount );

	void SetStateIfChanged();
 	void SetBlendMode( BlendMode blendMode );
	void SetRasterizerState( RasterizerMode rasterizerMode );

	void RenderEmissive();

	Texture* CreateTextureFromImage( Image const& image, char const* name );
	Texture* CreateTextureFromFile( char const* filePath );
 	Texture* CreateTextureFromData( char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData );
	bool RemoveLoadedTexture( Texture* texture );
	void BindTexture( Texture* textureMap, unsigned int slot = 0 );

	void SetSamplerMode( SamplerMode samplerMode, unsigned int slot = 0 );
	void SetDepthMode( DepthMode depthMode ) { m_desiredDepthMode = depthMode; }

	void SetLightingConstants( float*& sunDirection, float sunIntensity, float ambientIntensity );
	void SetLightingConstants( Vec3 const& sunDirection, float sunIntensity, float ambientIntensity );
	void SetLightingConstants( LightingConstants const& lightingConstants = LightingConstants() );
	void SetCameraConstants( Mat44 const& projectionMatrix = Mat44(), Mat44 const& viewMatrix = Mat44() );
	void SetModelConstants( Mat44 const& modelMatrix = Mat44(), Rgba8 const& modelColor = Rgba8::WHITE );
	void SetJointConstants( std::vector<Joint> const& joints );
	void SetJointConstants( std::vector<Mat44> const& globleTransforms, std::vector<Joint> const& joints );

	Texture* CreateRenderTexture( IntVec2 const& dimensions, const char* name );

 	Texture* GetTextureForFileName( char const* fileName );

 	BitmapFont* GetBitmapForFileName( char const* fileName );
// 
 	Texture* CreateOrGetTextureFromFile( char const* imageFilePath );

	BitmapFont* CreateOrGetBitmapFont( const char* imageFilePath );
 	BitmapFont* CreateBitmapFontFromFile( char const* imageFilePath );
	BitmapFont* CreateCustomBitmapFontFromXml( char const* xmlPath, char const* imagePath );

public:
	ID3D11Device* GetD3D11Device() { return m_device; }
	ID3D11DeviceContext* GetD3D11DeviceContext() { return m_deviceContext; }
	ID3D11RenderTargetView* GetD3D11RenderTargetView() { return m_renderTargetView; }

private:
// 	Texture* CreateTextureFromFile( char const* imageFilePath );

	RenderConfig m_config;
	void* m_displayDeviceContext = nullptr;

	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedFonts;

protected:
	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	VertexBuffer* m_immediateVBO = nullptr;
	IndexBuffer* m_immediateIBO = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightingCBO = nullptr;
	ConstantBuffer* m_blurCBO = nullptr;
	ConstantBuffer* m_jointCBO = nullptr;

	ID3D11BlendState* m_blendState = nullptr;
	BlendMode m_desiredBlendMode = BlendMode::ALPHA;
	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = {};

	ID3D11SamplerState* m_samplerState = nullptr;
	SamplerMode m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};

	ID3D11RasterizerState* m_rasterizerState = nullptr;
	RasterizerMode m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_NONE;
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};

	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;

	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	DepthMode m_desiredDepthMode = DepthMode::ENABLED;
	ID3D11DepthStencilState* m_depthStencilStates[(int)(DepthMode::COUNT)] = {};

	VertexBuffer* m_fullScreenQuadVBO_PCU = nullptr;

protected:
	std::vector<Shader*> m_loadedShaders;
	Shader* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;
	Texture* m_defaultTexture = nullptr;
	Texture* m_currentTexture = nullptr;

	Texture* m_emissiveRenderTexture = nullptr;

	std::vector<Texture*> m_emissiveBlurDownTexture;
	std::vector<Texture*> m_emissiveBlurUpTexture;
};
