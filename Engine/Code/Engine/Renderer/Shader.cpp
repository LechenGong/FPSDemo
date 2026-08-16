#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"

Shader::Shader( ShaderConfig const& config )
	:m_config( config )
{
}

Shader::Shader( Shader const& copy )
	: m_config( copy.m_config )
	, m_inputLayoutForVertex( copy.m_inputLayoutForVertex )
	, m_vertexShader( copy.m_vertexShader )
	, m_pixelShader( copy.m_pixelShader )
{
}

Shader::~Shader()
{
	DX_SAFE_RELEASE( m_inputLayoutForVertex );
	DX_SAFE_RELEASE( m_pixelShader );
	DX_SAFE_RELEASE( m_vertexShader );
}

std::string const& Shader::GetName() const
{
	return m_config.m_name;
}
