#pragma once

#include <string>
#include <vector>

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

class MapDefinition;
class Texture;

struct MapRenderBatch
{
	Texture* m_texture = nullptr;
	SamplerMode m_samplerMode = SamplerMode::POINT_CLAMP;
	std::vector<Vertex_PCU> m_verts;
};

struct MapGeometryBuildResult
{
	std::vector<AABB3> solidBounds;
	std::vector<OBB3> solidObbs;
	std::vector<bool> solidBoundsNavWalkable;
	std::vector<bool> solidObbsNavWalkable;
	std::vector<MapRenderBatch> renderBatches;
};

// Bakes map definition solids + render batches. No Map* dependency.
class MapGeometryBaker
{
public:
	static MapGeometryBuildResult Bake( MapDefinition const& definition, std::vector<std::string> const& skipGeometryNames = {} );
};
