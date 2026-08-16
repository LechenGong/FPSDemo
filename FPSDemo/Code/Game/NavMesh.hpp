#pragma once

#include <string>
#include <vector>

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Map;
struct NavMeshVolumeDefinition;

enum class NavSurfaceKind
{
	Ground,
	Gallery,
	Slope,
	Other
};

struct NavPoly
{
	int m_id = -1;
	std::vector<Vec3> m_verts; // convex quad (4 verts), CCW when viewed from above the face
	std::vector<int> m_neighbors;
	NavSurfaceKind m_kind = NavSurfaceKind::Other;

	Vec3 GetCenter() const;
};

// Shared doorway between two corridor polys (oriented left/right along travel).
struct NavPortal
{
	Vec3 m_left = Vec3::ZERO;
	Vec3 m_right = Vec3::ZERO;
};

struct NavPath
{
	std::vector<Vec3> m_waypoints;
	std::vector<NavPortal> m_portals; // corridor doors used by string-pull (debug)
	bool m_isValid = false;
};

// One baked walkable mesh for a single NavMeshVolume.
class NavMesh
{
public:
	void Clear();
	void BakeFrom( Map const& map, NavMeshVolumeDefinition const& volumeDef );
	int FindPoly( Vec3 const& position ) const;
	NavPoly const* GetPoly( int polyId ) const;
	Vec3 ProjectOntoPoly( NavPoly const& poly, Vec3 const& position ) const;
	bool TryGetPortal( NavPoly const& polyA, NavPoly const& polyB, Vec3 const& travelDir, NavPortal& outPortal ) const;
	bool TryGetPortalMidpoint( NavPoly const& polyA, NavPoly const& polyB, Vec3& outMid ) const;
	NavPath FindPath( Vec3 const& startPos, Vec3 const& goalPos ) const;

	void DebugRender() const;
	void DebugRenderPath( std::vector<Vec3> const& waypoints ) const;
	void DebugRenderPortals( std::vector<NavPortal> const& portals ) const;
	void RenderWalkableOverlay() const; // UE-style translucent green cover; call from Map::Render

	std::string const& GetName() const { return m_name; }
	AABB3 const& GetVolumeBounds() const { return m_volumeBounds; }
	std::vector<NavPoly> const& GetPolys() const { return m_polys; }
	int GetPolyCount() const { return (int)m_polys.size(); }

private:
	void BuildCandidates( Map const& map );
	void BuildAdjacency();
	void RebuildDebugOverlayVerts();
	bool RunAStar( int startPolyId, int goalPolyId, std::vector<int>& outCorridor ) const;
	void AppendDedupedWaypoint( std::vector<Vec3>& waypoints, Vec3 const& point ) const;

private:
	std::string m_name;
	AABB3 m_volumeBounds;
	std::vector<NavPoly> m_polys;
	std::vector<Vertex_PCU> m_debugOverlayVerts;
};
