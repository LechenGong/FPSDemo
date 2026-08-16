#pragma once

#include <string>
#include <vector>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct ShaderConfig
{
	std::string m_name;
	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";
};

class Shader
{
	friend class Renderer;

public:
	Shader( ShaderConfig const& config );
	Shader( Shader const& copy );
	~Shader();

	std::string const& GetName() const;

	ShaderConfig m_config;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayoutForVertex = nullptr;
};
