#pragma once

#include <string>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Texture;
class Shader;
class Renderer;

class Material
{
public:
	Material();
	Material( std::string XmlPath );
	~Material();

	std::string m_name;
	Shader* m_shader = nullptr;
	VertexType m_vertexType = VertexType::VERTEX_PCUTBN;
	Texture* m_diffuseMap = nullptr;
	Texture* m_normalMap = nullptr;
	Texture* m_specGlossEmitMap = nullptr;

	Texture* m_transparencyMap = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
};
