#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

VertexBuffer::VertexBuffer( size_t size )
	:m_size( size )
{
}

VertexBuffer::~VertexBuffer()
{
	DX_SAFE_RELEASE( m_buffer );
}
