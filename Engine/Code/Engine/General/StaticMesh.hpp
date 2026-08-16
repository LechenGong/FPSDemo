#pragma once

#include <string>
#include <vector>

#include "Engine/Model/ModelUtility.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"

class VertexBuffer;
class IndexBuffer;
class Texture;
class MeshT;
class Renderer;
class Material;

enum class StaticMeshPreset
{
	CUBE,
	SPHERE,
	COUNT
};

class StaticMesh
{
public:
	StaticMesh();
	StaticMesh( std::vector<MeshT*>&meshes );
	StaticMesh( StaticMeshPreset );
	~StaticMesh();

	void SetColor( Rgba8 const& color );
	Rgba8 GetColor() const;

	void SetMaterial( Material* material );
	std::vector<MeshT>& GetMeshes();

	virtual void Update( float deltaSeconds );
	virtual void Render();

public:
	void ExportToXML( std::string const& filePath ) const;
	static StaticMesh* ImportFromXML( std::string const& filePath );

protected:
	Rgba8 m_color = Rgba8::WHITE;

	std::vector<MeshT> m_meshes;

public:
	bool m_debugRender = false;
};
