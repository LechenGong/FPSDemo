#include "Engine/General/StaticMesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Shader.hpp"

StaticMesh::StaticMesh()
{
}

StaticMesh::StaticMesh( std::vector<MeshT*>& meshes )
{
	m_meshes.reserve( meshes.size() );
	for (int i = 0; i < meshes.size(); i++)
	{
		if (meshes[i])
		{
			m_meshes.push_back( std::move( *meshes[i] ) );
			//delete meshes[i];
			meshes[i] = nullptr;
		}
	}
}

StaticMesh::StaticMesh( StaticMeshPreset staticMeshType )
{
	MeshT* newMesh;
	if (staticMeshType == StaticMeshPreset::CUBE)
	{
		std::vector<Vertex_PCUTBN> verts;
		std::vector<unsigned int> indexes;
		AddVertsForAABB3D( verts, indexes, AABB3( Vec3( -0.5f, -0.5f, -0.5f ), Vec3( 0.5f, 0.5f, 0.5f ) ) );
		CalculateTangantSpaceBasisVectors( verts, indexes, true, true );
		newMesh = new MeshT( verts, indexes );
		m_meshes.push_back( *newMesh );
	}
	else if (staticMeshType == StaticMeshPreset::SPHERE)
	{
		std::vector<Vertex_PCUTBN> verts;
		std::vector<unsigned int> indexes;
		AddVertsForSphere3D( verts, indexes, Vec3::ZERO, 0.5f );
		CalculateTangantSpaceBasisVectors( verts, indexes, true, true );
		newMesh = new MeshT( verts, indexes );
		m_meshes.push_back( *newMesh );
	}
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::SetColor( Rgba8 const& color )
{
	m_color = color;
}

Rgba8 StaticMesh::GetColor() const
{
	return m_color;
}

void StaticMesh::SetMaterial( Material* material )
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i].SetMaterial( material );
	}
}

std::vector<MeshT>& StaticMesh::GetMeshes()
{
	return m_meshes;
}

void StaticMesh::Update( float deltaSeconds )
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i].Update( deltaSeconds );
	}
}

void StaticMesh::Render()
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i].Render();
		if (m_debugRender)
		{
			m_meshes[i].DebugRender();
		}
	}
}

void StaticMesh::ExportToXML( std::string const& filePath ) const
{
	using namespace tinyxml2;
	XmlDocument doc;

	// Root element
	XmlElement* root = doc.NewElement( "StaticMesh" );
	doc.InsertFirstChild( root );

	// Export meshes
	XmlElement* meshesElem = doc.NewElement( "Meshes" );
	for (const MeshT& mesh : m_meshes)
	{
		XmlElement* meshElem = doc.NewElement( "Mesh" );
		meshElem->SetAttribute( "name", mesh.name.c_str() );
		meshElem->SetAttribute( "isVisible", mesh.isVisible );

		// Export material
		XmlElement* materialElem = doc.NewElement( "Material" );
		materialElem->SetAttribute( "name", mesh.material->m_name.c_str() );
		if (mesh.material->m_vertexType == VertexType::VERTEX_PCU)
		{
			materialElem->SetAttribute( "vertexType", "Vertex_PCU" );
		}
		else if (mesh.material->m_vertexType == VertexType::VERTEX_PCUTBN)
		{
			materialElem->SetAttribute( "vertexType", "Vertex_PCUTBN" );
		}
		else
		{
			materialElem->SetAttribute( "vertexType", "Vertex_Anim" );
		}

		materialElem->SetAttribute( "color", mesh.material->m_color.ToString().c_str() );
 		if (mesh.material->m_shader)
 		{
 			materialElem->SetAttribute( "shader", mesh.material->m_shader->GetName().c_str() );
 		}
		if (mesh.material->m_diffuseMap)
		{
			materialElem->SetAttribute( "diffuseTexture", mesh.material->m_diffuseMap->GetImageFilePath().c_str() );
		}
		if (mesh.material->m_normalMap)
		{
			materialElem->SetAttribute( "normalTexture", mesh.material->m_normalMap->GetImageFilePath().c_str() );
		}
		if (mesh.material->m_specGlossEmitMap)
		{
			materialElem->SetAttribute( "specGlossEmitTexture", mesh.material->m_specGlossEmitMap->GetImageFilePath().c_str() );
		}
		if (mesh.material->m_transparencyMap)
		{
			materialElem->SetAttribute( "transparencyTexture", mesh.material->m_transparencyMap->GetImageFilePath().c_str() );
		}
		meshElem->InsertEndChild( materialElem );

		// Export vertexes
		XmlElement* vertexesElem = doc.NewElement( "Vertexes" );
		for (const Vertex_PCUTBN& vertex : mesh.vertexes)
		{
			XmlElement* vertexElem = doc.NewElement( "Vertex" );
			vertexElem->SetAttribute( "position", vertex.m_position.ToString().c_str() );
			vertexElem->SetAttribute( "color", vertex.m_color.ToString().c_str() );
			vertexElem->SetAttribute( "uv", vertex.m_uvTexCoords.ToString().c_str() );
			vertexElem->SetAttribute( "tangent", vertex.m_tangent.ToString().c_str() );
			vertexElem->SetAttribute( "bitangent", vertex.m_bitangent.ToString().c_str() );
			vertexElem->SetAttribute( "normal", vertex.m_normal.ToString().c_str() );
			vertexesElem->InsertEndChild( vertexElem );
		}
		meshElem->InsertEndChild( vertexesElem );

		XmlElement* indexesElem = doc.NewElement( "Indexes" );
		for (const unsigned int index : mesh.indexes)
		{
			XmlElement* indexElem = doc.NewElement( "Index" );
			indexElem->SetText( index );
			indexesElem->InsertEndChild( indexElem );
		}
		meshElem->InsertEndChild( indexesElem );

		meshesElem->InsertEndChild( meshElem );
	}
	root->InsertEndChild( meshesElem );

	doc.SaveFile( filePath.c_str() );
}

StaticMesh* StaticMesh::ImportFromXML( std::string const& filePath )
{
	using namespace tinyxml2;
	StaticMesh* staticMesh = new StaticMesh;
	XmlDocument doc;

	if (doc.LoadFile( filePath.c_str() ) == XML_SUCCESS)
	{
		XmlElement* root = doc.FirstChildElement( "StaticMesh" );

		// Import meshes
		XmlElement* meshesElem = root->FirstChildElement( "Meshes" );
		XmlElement* meshElem = meshesElem->FirstChildElement( "Mesh" );
		while (meshElem)
		{
			MeshT mesh;
			mesh.name = meshElem->Attribute( "name" );
			mesh.isVisible = meshElem->BoolAttribute( "isVisible" );

			// Import material
			mesh.material = new Material;
			XmlElement* materialElem = meshElem->FirstChildElement( "Material" );
			mesh.material->m_name = ParseXmlAttribute( *materialElem, "name", "" );
			std::string shaderPath = ParseXmlAttribute( *materialElem, "shader", "" );
			std::string vertexTypeName = ParseXmlAttribute( *materialElem, "vertexType", "Vertex_PCUTBN" );
			std::string diffuseTexturePath = ParseXmlAttribute( *materialElem, "diffuseTexture", "" );
			std::string normalTexturePath = ParseXmlAttribute( *materialElem, "normalTexture", "" );
			std::string specTexturePath = ParseXmlAttribute( *materialElem, "specGlossEmitTexture", "" );
			std::string transparencyPath = ParseXmlAttribute( *materialElem, "transparencyTexture", "" );
			Rgba8 color = ParseXmlAttribute( *materialElem, "color", Rgba8::WHITE );

			if (vertexTypeName == "Vertex_PCU")
			{
				mesh.material->m_vertexType = VertexType::VERTEX_PCU;
			}
			else if (vertexTypeName == "Vertex_PCUTBN")
			{
				mesh.material->m_vertexType = VertexType::VERTEX_PCUTBN;
			}
			else
			{
				mesh.material->m_vertexType = VertexType::VERTEX_ANIM;
			}
			if (shaderPath != "")
			{
				mesh.material->m_shader = g_theRenderer->CreateShader( shaderPath.c_str(), mesh.material->m_vertexType );
				//m_shader = g_theRenderer->CreateShader( "Data/Shaders/Diffuse.hlsl", m_vertexType);
			}
			if (diffuseTexturePath != "")
			{
				mesh.material->m_diffuseMap = g_theRenderer->CreateOrGetTextureFromFile( diffuseTexturePath.c_str() );
			}
			if (normalTexturePath != "")
			{
				mesh.material->m_normalMap = g_theRenderer->CreateOrGetTextureFromFile( normalTexturePath.c_str() );
			}
			if (specTexturePath != "")
			{
				mesh.material->m_specGlossEmitMap = g_theRenderer->CreateOrGetTextureFromFile( specTexturePath.c_str() );
			}
			if (transparencyPath != "")
			{
				mesh.material->m_transparencyMap = g_theRenderer->CreateOrGetTextureFromFile( transparencyPath.c_str() );
			}

			// Import vertexes
			XmlElement* vertexesElem = meshElem->FirstChildElement( "Vertexes" );
			XmlElement* vertexElem = vertexesElem->FirstChildElement( "Vertex" );
			while (vertexElem)
			{
				Vertex_PCUTBN vertex;
				vertex.m_position.SetFromText( vertexElem->Attribute( "position" ) );
				vertex.m_color.SetFromText( vertexElem->Attribute( "color" ) );
				vertex.m_uvTexCoords.SetFromText( vertexElem->Attribute( "uv" ) );
				vertex.m_tangent.SetFromText( vertexElem->Attribute( "tangent" ) );
				vertex.m_bitangent.SetFromText( vertexElem->Attribute( "bitangent" ) );
				vertex.m_normal.SetFromText( vertexElem->Attribute( "normal" ) );

				mesh.vertexes.push_back( vertex );
				vertexElem = vertexElem->NextSiblingElement( "Vertex" );
			}

			XmlElement* indexesElem = meshElem->FirstChildElement( "Indexes" );
			XmlElement* indexElem = indexesElem->FirstChildElement( "Index" );
			while (indexElem)
			{
				mesh.indexes.push_back( std::stoul( indexElem->GetText() ) );
				indexElem = indexElem->NextSiblingElement( "Index" );
			}
			staticMesh->m_meshes.push_back( mesh );
			meshElem = meshElem->NextSiblingElement( "Mesh" );
		}
	}
	return staticMesh;
}
