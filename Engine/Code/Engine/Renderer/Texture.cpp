#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"

Texture::Texture()
{
}

Texture::~Texture()
{
	DX_SAFE_RELEASE( m_texture );
	DX_SAFE_RELEASE( m_renderTargetView );
	DX_SAFE_RELEASE( m_shaderResourceView );
}
