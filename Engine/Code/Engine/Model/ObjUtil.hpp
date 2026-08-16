#pragma once

#include <vector>
#include <string>
#include <map>

#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class VertexBuffer;
class IndexBuffer;
class Renderer;
class MeshT;

class OBJ
{
public:
	OBJ() {};
	OBJ( std::string objPath );
	~OBJ();

	virtual void Render() const;
	std::vector<MeshT*> m_meshes;
	
	Vec3 m_position;
	EulerAngles m_orientation = EulerAngles::ZERO;
	float m_scale = 1.f;

protected:
	void LoadRawFile( std::string objPath );
	void ProcessMTL();
	void AddVerts();


	int parts = -1;
	Mat44 m_initTransformMatrix;

	std::string m_rootPath;
	std::string m_rawOBJ;
	std::string m_rawMTL;

	struct MTLInfo
	{
		std::string name;
		float Ns;
		Vec3 Ka;
		Vec3 Kd;
		Vec3 Ks;
		Vec3 Ke;
		float Ni;
		float d;
		int illum;
	};
	std::map<std::string, MTLInfo> m_mtlData;

	int vertsCount = 0;
	int indexesCount = 0;

	int positionsCount = 0;
	int uvsCount = 0;
	int normalsCount = 0;
	int facesCount = 0;
};
