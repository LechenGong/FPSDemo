#include "Engine/General/MeshT.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"

MeshT::MeshT()
{
	if (!material)
		material = new Material();
}

MeshT::MeshT( std::vector<Vertex_PCUTBN> pVerts, std::vector<unsigned int> pIndexes )
{
	if (!material)
		material = new Material();
	vertexes = pVerts;
	indexes = pIndexes;
	
}

MeshT::~MeshT()
{
	delete vertexBuffer;
	vertexBuffer = nullptr;
	delete indexBuffer;
	indexBuffer = nullptr;
	delete jointBuffer;
	jointBuffer = nullptr;

	delete debugVertexBuffer;
	debugVertexBuffer = nullptr;
}

void MeshT::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void MeshT::Render() const
{
	if (!material->m_shader)
	{
		g_theRenderer->BindShader( g_theRenderer->CreateShader( "Data/Shaders/Diffuse.hlsl", VertexType::VERTEX_PCUTBN ) );
		g_theRenderer->BindTexture( material->m_diffuseMap, 0 );
	}
	else
	{
		g_theRenderer->BindShader( material->m_shader );
		if (material->m_diffuseMap)
			g_theRenderer->BindTexture( material->m_diffuseMap, 0 );
		if (material->m_normalMap)
			g_theRenderer->BindTexture( material->m_normalMap, 1 );
		if (material->m_specGlossEmitMap)
			g_theRenderer->BindTexture( material->m_specGlossEmitMap, 2 );
	}

	if (material->m_shader)
	{
		if (material->m_vertexType == VertexType::VERTEX_ANIM)
		{
			if ((!vertexBuffer || !indexBuffer || !jointBuffer) && vertexes.size() > 0 && indexes.size() > 0)
			{
				delete vertexBuffer;
				delete indexBuffer;
				delete jointBuffer;
				vertexBuffer = g_theRenderer->CreateVertexBuffer( vertexes.size() * sizeof( Vertex_PCUTBN ) );
				jointBuffer = g_theRenderer->CreateVertexBuffer( jointInfluences.size() * sizeof( Vertex_Anim ) );
				indexBuffer = g_theRenderer->CreateIndexBuffer( indexes.size() * sizeof( unsigned int ) );
				g_theRenderer->CopyCPUToGPU( vertexes.data(), vertexes.size(), vertexBuffer, jointInfluences.data(), jointInfluences.size(), jointBuffer, indexes.data(), indexes.size(), indexBuffer );
			}
		}
		else
		{
			if ((!vertexBuffer || !indexBuffer) && vertexes.size() > 0 && indexes.size() > 0)
			{
				delete vertexBuffer;
				delete indexBuffer;
				vertexBuffer = g_theRenderer->CreateVertexBuffer( vertexes.size() * sizeof( Vertex_PCUTBN ) );
				indexBuffer = g_theRenderer->CreateIndexBuffer( indexes.size() * sizeof( unsigned int ) );
				g_theRenderer->CopyCPUToGPU( vertexes.data(), vertexes.size(), vertexBuffer, indexes.data(), indexes.size(), indexBuffer );
			}
		}
	}
	else
	{
		if ((!vertexBuffer || !indexBuffer) && vertexes.size() > 0 && indexes.size() > 0)
		{
			delete vertexBuffer;
			delete indexBuffer;
			vertexBuffer = g_theRenderer->CreateVertexBuffer( vertexes.size() * sizeof( Vertex_PCUTBN ) );
			indexBuffer = g_theRenderer->CreateIndexBuffer( indexes.size() * sizeof( unsigned int ) );
			g_theRenderer->CopyCPUToGPU( vertexes.data(), vertexes.size(), vertexBuffer, indexes.data(), indexes.size(), indexBuffer );
		}
	}

	if (material->m_shader && material->m_vertexType == VertexType::VERTEX_ANIM)
		g_theRenderer->DrawVertexAndIndexBuffer( vertexBuffer, jointBuffer, indexBuffer, (int)indexes.size() );
	else
		g_theRenderer->DrawVertexAndIndexBuffer( vertexBuffer, indexBuffer, (int)indexes.size(), VertexType::VERTEX_PCUTBN );
}

void MeshT::DebugRender()
{
	if (!debugVertexBuffer && vertexes.size() > 0)
	{
		std::vector<Vertex_PCU> verts;
		verts.reserve( 100000 );
		for (int i = 0; i < vertexes.size(); i++)
		{
			AddVertsForCylinder3D( verts, vertexes[i].m_position, vertexes[i].m_position + vertexes[i].m_tangent * 0.1f, 0.0025f, Rgba8::RED );
			AddVertsForCylinder3D( verts, vertexes[i].m_position, vertexes[i].m_position + vertexes[i].m_bitangent * 0.1f, 0.0025f, Rgba8::GREEN );
			AddVertsForCylinder3D( verts, vertexes[i].m_position, vertexes[i].m_position + vertexes[i].m_normal * 0.1f, 0.0025f, Rgba8::BLUE );
		}
		debugVertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ) );
		g_theRenderer->CopyCPUToGPU( verts.data(), verts.size(), debugVertexBuffer );
		m_debugVertSize = int( verts.size() );
	}
	else
	{
		g_theRenderer->DrawVertexBuffer( debugVertexBuffer, m_debugVertSize );
	}
}

void MeshT::SetMaterial( Material* const& pMaterial )
{
	delete material;
	material = pMaterial;
}
