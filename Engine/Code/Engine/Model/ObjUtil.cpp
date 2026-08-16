#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>

#include "Engine/Model/ObjUtil.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Material.hpp"
//#include "Engine/Model/Vertex_Anim.hpp"

struct VertexIndex
{
	int m_posIndex = -1;
	int m_textureIndex = -1;
	int m_normalIndex = -1;
};

struct vertexHash
{
	std::size_t operator()( VertexIndex const& k ) const
	{
		return ((size_t)k.m_posIndex) ^ (((size_t)k.m_textureIndex) << 1) ^ (((size_t)k.m_normalIndex) << 2);
	}
};

struct vertexEqual
{
	bool operator()( VertexIndex const& lhs, VertexIndex const& rhs ) const
	{
		return lhs.m_posIndex == rhs.m_posIndex &&
			lhs.m_textureIndex == rhs.m_textureIndex &&
			lhs.m_normalIndex == rhs.m_normalIndex;
	}
};

OBJ::OBJ( std::string objPath )
{
	double startTime = GetCurrentTimeSeconds();

	std::string b = std::filesystem::current_path().string();
	m_initTransformMatrix = Mat44();
	Material* newMaterial = nullptr;
	if (objPath.find( ".xml" ) != std::string::npos || objPath.find( ".XML" ) != std::string::npos)
	{
		XmlDocument objXml;
		const char* filePath = objPath.c_str();
		XmlError xmlResult = objXml.LoadFile( filePath );
		GUARANTEE_OR_DIE( xmlResult == tinyxml2::XML_SUCCESS, Stringf( "failed to load xml file" ) );
		XmlElement* rootElement = objXml.RootElement();
		GUARANTEE_OR_DIE( rootElement, Stringf( "rootElement is nullptr" ) );
		objPath = ParseXmlAttribute( *rootElement, "path", "" );
		std::string materialPath = ParseXmlAttribute( *rootElement, "material", "" );
		if (materialPath != "")
		{
			newMaterial = new Material( materialPath );
		}
		XmlElement* transformElement = rootElement->FirstChildElement( "Transform" );
		m_initTransformMatrix.SetIJKT3D(
			ParseXmlAttribute( *transformElement, "x", m_initTransformMatrix.GetIBasis3D() ),
			ParseXmlAttribute( *transformElement, "y", m_initTransformMatrix.GetJBasis3D() ),
			ParseXmlAttribute( *transformElement, "z", m_initTransformMatrix.GetKBasis3D() ),
			ParseXmlAttribute( *transformElement, "t", m_initTransformMatrix.GetTranslation3D() )
		);
		m_scale = ParseXmlAttribute( *transformElement, "scale", 1.f );
	}

	if (objPath.find_last_of( "/" ) != std::string::npos)
	{
		m_rootPath = objPath.substr( 0, objPath.find_last_of( "/" ) );
	}
	else
	{
		m_rootPath = objPath.substr( 0, objPath.find_last_of( "\\" ) );
	}
	
	LoadRawFile( objPath );
	ProcessMTL();
	AddVerts();

	if (newMaterial)
	{
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i]->material = newMaterial;
		}
	}

	for (int i = 0; i < m_meshes.size(); i++)
	{
		vertsCount += (int)m_meshes[i]->vertexes.size();
		indexesCount += (int)m_meshes[i]->indexes.size();
	}

	double endTime = GetCurrentTimeSeconds();

	DebuggerPrintf( "/-------------------------------------------------------------------------/\n" );
	DebuggerPrintf( "Loaded .obj file %s\n", objPath.c_str() );
	DebuggerPrintf( "positions: %d  uvs: %d  normals: %d  faces: %d\n", positionsCount, uvsCount, normalsCount, facesCount );
	DebuggerPrintf( "vertexes: %d  indexes: %d  time: %fseconds\n", vertsCount, indexesCount, endTime - startTime );
	DebuggerPrintf( "/-------------------------------------------------------------------------/\n" );
}

OBJ::~OBJ()
{
}

void OBJ::Render() const
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i]->Render();
	}
}

void OBJ::LoadRawFile( std::string objPath )
{
	std::ifstream objFile;
	objFile.exceptions( std::fstream::failbit || std::fstream::badbit );
	try
	{
		objFile.open( objPath );
		std::stringstream objStream;
		objStream << objFile.rdbuf();
		objFile.close();
		m_rawOBJ = objStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		//ERROR_RECOVERABLE( Stringf( "Load Model %s Fail", objPath ) );
		UNUSED( e );
	}

	int mtlPathStart = (int)m_rawOBJ.find( "mtllib " );
	if (mtlPathStart != -1)
	{
		int mtlPathEnd = (int)m_rawOBJ.find( "\n", mtlPathStart );
		std::string mtlPath = m_rootPath + '/' + Split( m_rawOBJ.substr( mtlPathStart, mtlPathEnd - mtlPathStart ), ' ' )[1];
		std::ifstream mtlFile;
		mtlFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
		try
		{
			mtlFile.open( mtlPath );
			std::stringstream mtlStream;
			mtlStream << mtlFile.rdbuf();
			mtlFile.close();
			m_rawMTL = mtlStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			//ERROR_RECOVERABLE( Stringf( "Load Model %s Fail", mtlPath ) );
			UNUSED( e );
		}
	}
}

void OBJ::ProcessMTL()
{
	if (m_rawMTL == "")
		return;

	Strings lines = Split( m_rawMTL, '\n' );
	std::string currentMTLName = "";
	for (int i = 0; i < lines.size(); i++)
	{
		Strings contents = Split( lines[i], ' ', true, 1 );
		if (contents.size() != 2)
			continue;

		if (contents[0] == "newmtl")
		{
			currentMTLName = contents[1];
			m_mtlData[currentMTLName].name = currentMTLName;
		}
		else if (contents[0] == "Ns")
		{
			m_mtlData[currentMTLName].Ns = std::stof( contents[1] );
		}
		else if (contents[0] == "Ka")
		{
			m_mtlData[currentMTLName].Ka = Vec3( contents[1] );
		}
		else if (contents[0] == "Kd")
		{
			m_mtlData[currentMTLName].Kd = Vec3( contents[1] );
		}
		else if (contents[0] == "Ks")
		{
			m_mtlData[currentMTLName].Ks = Vec3( contents[1] );
		}
		else if (contents[0] == "Ke")
		{
			m_mtlData[currentMTLName].Ke = Vec3( contents[1] );
		}
		else if (contents[0] == "Ni")
		{
			m_mtlData[currentMTLName].Ni = std::stof( contents[1] );
		}
		else if (contents[0] == "d")
		{
			m_mtlData[currentMTLName].d = std::stof( contents[1] );
		}
		else if (contents[0] == "illum")
		{
			m_mtlData[currentMTLName].illum = std::stoi( contents[1] );
		}
	}
}

void OBJ::AddVerts()
{
	std::vector<Vec3> positions;
	std::vector<Vec2> uvs;
	std::vector<Vec3> normals;	

	std::unordered_map<VertexIndex, int, vertexHash, vertexEqual> vertexLookUpTable;

	std::string currentMTLName = "";
	std::string prevIdentifier = "";
	std::string name = "";

	Strings lines = Split( m_rawOBJ, '\n' );
	for (int i = 0; i < lines.size(); i++)
	{
		Strings contents = Split( lines[i], ' ', true, 1 );
		if (contents.size() != 2)
			continue;
		if (contents[0] == "#")
			continue;
		// Assume always valid data?

		if (contents[0] == "o")
		{
			if (parts >= 0)
			{
				if (m_meshes[parts]->vertexes.size() > 0)
				{
					CalculateTangantSpaceBasisVectors( m_meshes[parts]->vertexes, m_meshes[parts]->indexes, normals.size() > 0, uvs.size() > 0 );
					m_meshes[parts]->vertexBuffer = g_theRenderer->CreateVertexBuffer( m_meshes[parts]->vertexes.size() * (m_meshes[parts]->material->m_vertexType == VertexType::VERTEX_PCUTBN ? sizeof( Vertex_PCUTBN ) : sizeof( Vertex_PCU )) );
					m_meshes[parts]->indexBuffer = g_theRenderer->CreateIndexBuffer( m_meshes[parts]->indexes.size() * sizeof( unsigned int ) );
					g_theRenderer->CopyCPUToGPU( m_meshes[parts]->vertexes.data(), m_meshes[parts]->vertexes.size(), m_meshes[parts]->vertexBuffer, m_meshes[parts]->indexes.data(), m_meshes[parts]->indexes.size(), m_meshes[parts]->indexBuffer );
					MeshT* newMeshT = new MeshT();
					name = contents[1];
					newMeshT->name = name;
					m_meshes.push_back( newMeshT );
					parts++;
				}
			}
			name = contents[1];
		}

		if (contents[0] == "v")
		{
			positions.push_back( Vec3( contents[1] ) );
		}
		else if (contents[0] == "vt")
		{
			uvs.push_back( Vec2( contents[1] ) );
		}
		else if (contents[0] == "vn")
		{
			normals.push_back( Vec3( contents[1] ) );
		}
		else if (contents[0] == "f")
		{
			facesCount++;
			if (parts == -1)
			{
				MeshT* newMeshT = new MeshT();
				newMeshT->name = name;
				m_meshes.push_back( newMeshT );
				parts++;
			}

			Strings vertsString = Split( contents[1], ' ' );
			std::vector<Vertex_PCUTBN> verts;
			std::vector<VertexIndex> vertIndexs;
			for (int j = 0; j < vertsString.size(); j++)
			{
				Strings data = Split( vertsString[j], '/' );
				Vertex_PCUTBN vertex{
					m_initTransformMatrix.TransformPosition3D( positions[std::stoi( data[0] ) - 1] ) * m_scale,
					(m_rawMTL == "") ? Rgba8::WHITE : Rgba8(
						unsigned char( 255.f * m_mtlData[currentMTLName].Kd.x ),
						unsigned char( 255.f * m_mtlData[currentMTLName].Kd.y ),
						unsigned char( 255.f * m_mtlData[currentMTLName].Kd.z )
					),
					(data.size() > 1 && data[1] != "") ? uvs[std::stoi( data[1] ) - 1] : Vec2::ZERO,
					Vec3::ZERO,
					Vec3::ZERO,
					(data.size() > 2 && data[2] != "") ? normals[std::stoi( data[2] ) - 1] : Vec3::ZERO
				};
				verts.push_back( vertex );
				vertIndexs.push_back( VertexIndex{
					std::stoi( data[0] ) - 1,
					(data.size() > 1 && data[1] != "") ? std::stoi( data[1] ) - 1 : 0,
					(data.size() > 2 && data[2] != "") ? std::stoi( data[2] ) - 1 : 0,
					} );

			}
			for (int j = 1; j < verts.size() - 1; j++)
			{
				auto iter = vertexLookUpTable.find( vertIndexs[0] );
				if (iter == vertexLookUpTable.end())
				{
					int thisIndex = (int)m_meshes[parts]->vertexes.size();
					m_meshes[parts]->vertexes.push_back( verts[0] );
					m_meshes[parts]->indexes.push_back( thisIndex );
					vertexLookUpTable[vertIndexs[0]] = thisIndex;
				}
				else
				{
					m_meshes[parts]->indexes.push_back( iter->second );
				}

				iter = vertexLookUpTable.find( vertIndexs[j] );
				if (iter == vertexLookUpTable.end())
				{
					int thisIndex = (int)m_meshes[parts]->vertexes.size();
					m_meshes[parts]->vertexes.push_back( verts[j] );
					m_meshes[parts]->indexes.push_back( thisIndex );
					vertexLookUpTable[vertIndexs[j]] = thisIndex;
				}
				else
				{
					m_meshes[parts]->indexes.push_back( iter->second );
				}

				iter = vertexLookUpTable.find( vertIndexs[j + 1] );
				if (iter == vertexLookUpTable.end())
				{
					int thisIndex = (int)m_meshes[parts]->vertexes.size();
					m_meshes[parts]->vertexes.push_back( verts[j + 1] );
					m_meshes[parts]->indexes.push_back( thisIndex );
					vertexLookUpTable[vertIndexs[j + 1]] = thisIndex;
				}
				else
				{
					m_meshes[parts]->indexes.push_back( iter->second );
				}
			}
		}
		else if (contents[0] == "usemtl")
		{
			currentMTLName = contents[1];
		}
		prevIdentifier = contents[0];
	}
	CalculateTangantSpaceBasisVectors( m_meshes[parts]->vertexes, m_meshes[parts]->indexes, normals.size() > 0, uvs.size() > 0 );
	m_meshes[parts]->vertexBuffer = g_theRenderer->CreateVertexBuffer( m_meshes[parts]->vertexes.size() * (m_meshes[parts]->material->m_vertexType == VertexType::VERTEX_PCUTBN ? sizeof( Vertex_PCUTBN ) : sizeof( Vertex_PCU )) );
	m_meshes[parts]->indexBuffer = g_theRenderer->CreateIndexBuffer( m_meshes[parts]->indexes.size() * sizeof( unsigned int ) );
	m_meshes[parts]->name = name;
	g_theRenderer->CopyCPUToGPU( m_meshes[parts]->vertexes.data(), m_meshes[parts]->vertexes.size(), m_meshes[parts]->vertexBuffer, m_meshes[parts]->indexes.data(), m_meshes[parts]->indexes.size(), m_meshes[parts]->indexBuffer );

	positionsCount += (int)positions.size();
	uvsCount += (int)uvs.size();
	normalsCount += (int)normals.size();
}
