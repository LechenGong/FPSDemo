#pragma once

#include <vector>
#include <string>

#include "Engine/Model/Vertex_Anim.hpp"
#include "Engine/Model/ModelUtility.hpp"
#include "Engine/Math/AABB3.hpp"

class VertexBuffer;
class IndexBuffer;
class Texture;
class AnimationSequence;
class Renderer;
class Material;

class MeshT
{
public:
	std::string name;
	bool isVisible = true;
	std::vector<Vertex_PCUTBN> vertexes;
	std::vector<Vertex_Anim> jointInfluences;
	std::vector<unsigned int> indexes;
	mutable VertexBuffer* vertexBuffer = nullptr;
	mutable VertexBuffer* jointBuffer = nullptr;
	mutable IndexBuffer* indexBuffer = nullptr;
	mutable VertexBuffer* debugVertexBuffer = nullptr;
	Material* material = nullptr;

	MeshT();
	MeshT( std::vector<Vertex_PCUTBN> pVerts, std::vector<unsigned int> pIndexes );
	~MeshT();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void DebugRender();

	virtual void SetMaterial( Material* const& pMaterial );

protected:
	float m_currentTimeSecond = 0.f;
	int m_debugVertSize = 0;
	Mat44 m_pivotTransform; // world to local
};
