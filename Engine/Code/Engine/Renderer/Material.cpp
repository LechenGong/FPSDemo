#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

Material::Material()
{
}

Material::Material( std::string XmlPath )
{
	XmlDocument xml;
	const char* filePath = XmlPath.c_str();
	XmlError xmlResult = xml.LoadFile( filePath );
	GUARANTEE_OR_DIE( xmlResult == tinyxml2::XML_SUCCESS, Stringf( "failed to load xml file" ) );
	XmlElement* rootElement = xml.RootElement();
	GUARANTEE_OR_DIE( rootElement, Stringf( "rootElement is nullptr" ) );
	m_name = ParseXmlAttribute( *rootElement, "name", "" );
	std::string shaderPath = ParseXmlAttribute( *rootElement, "shader", "" );
	std::string vertexTypeName = ParseXmlAttribute( *rootElement, "vertexType", "Vertex_PCUTBN" );
	std::string diffuseTexturePath = ParseXmlAttribute( *rootElement, "diffuseTexture", "" );
	std::string normalTexturePath = ParseXmlAttribute( *rootElement, "normalTexture", "" );
	std::string specTexturePath = ParseXmlAttribute( *rootElement, "specGlossEmitTexture", "" );
	std::string transparencyPath = ParseXmlAttribute( *rootElement, "transparencyTexture", "" );
	Rgba8 color = ParseXmlAttribute( *rootElement, "color", Rgba8::WHITE );

	if (vertexTypeName == "Vertex_PCU")
	{
		m_vertexType = VertexType::VERTEX_PCU;
	}
	else if (vertexTypeName == "Vertex_PCUTBN")
	{
		m_vertexType = VertexType::VERTEX_PCUTBN;
	}
	else
	{
		m_vertexType = VertexType::VERTEX_ANIM;
	}
	if (shaderPath != "")
	{
		m_shader = g_theRenderer->CreateShader( shaderPath.c_str(), m_vertexType );
		//m_shader = g_theRenderer->CreateShader( "Data/Shaders/Diffuse.hlsl", m_vertexType);
	}
	if (diffuseTexturePath != "")
	{
		m_diffuseMap = g_theRenderer->CreateOrGetTextureFromFile( diffuseTexturePath.c_str() );
	}
	if (normalTexturePath != "")
	{
		m_normalMap = g_theRenderer->CreateOrGetTextureFromFile( normalTexturePath.c_str() );
	}
	if (specTexturePath != "")
	{
		m_specGlossEmitMap = g_theRenderer->CreateOrGetTextureFromFile( specTexturePath.c_str() );
	}
	if (transparencyPath != "")
	{
		m_transparencyMap = g_theRenderer->CreateOrGetTextureFromFile( transparencyPath.c_str() );
	}
}

Material::~Material()
{
}
