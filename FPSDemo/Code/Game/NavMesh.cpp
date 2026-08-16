#include "Game/NavMesh.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"

#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/GameCommon.hpp"

#include <cmath>

namespace
{
	constexpr float NAV_EDGE_MATCH_EPSILON = 0.2f;
	constexpr float NAV_EDGE_HEIGHT_EPSILON = 0.35f;
	constexpr float NAV_FIND_POLY_MAX_DIST = 1.5f;
	constexpr float NAV_DEBUG_DRAW_DURATION = 0.1f; // must be > 0; duration 0 never draws in DebugRenderSystem
	constexpr float NAV_DEBUG_LINE_RADIUS = 0.02f;
	constexpr float NAV_OVERLAY_OFFSET = 0.05f; // lift cover slightly above walkable surface
	constexpr float NAV_WAYPOINT_DEDUP_EPSILON = 0.08f;
	constexpr float NAV_PATH_DRAW_LIFT = 0.12f;
	constexpr float NAV_PATH_POINT_RADIUS = 0.06f;
	constexpr float NAV_FUNNEL_POINT_EPSILON = 0.001f;
	constexpr bool NAV_DEBUG_DRAW_PORTALS = true;
	// UE-like bright green with translucency
	Rgba8 const NAV_OVERLAY_COLOR( 64, 255, 96, 110 );
	Rgba8 const NAV_PATH_COLOR( 255, 48, 32, 255 );
	Rgba8 const NAV_PATH_POINT_COLOR( 255, 220, 64, 255 );
	Rgba8 const NAV_PORTAL_LEFT_COLOR( 80, 200, 255, 255 );
	Rgba8 const NAV_PORTAL_RIGHT_COLOR( 255, 160, 40, 255 );

	bool IsPointInsideAABB3Inclusive( Vec3 const& point, AABB3 const& box )
	{
		return point.x >= box.m_mins.x && point.x <= box.m_maxs.x
			&& point.y >= box.m_mins.y && point.y <= box.m_maxs.y
			&& point.z >= box.m_mins.z && point.z <= box.m_maxs.z;
	}

	AABB3 MakeWorldBoundsFromOBB( OBB3 const& obb )
	{
		Vec3 const& h = obb.m_halfDimensions;
		bool hasPoint = false;
		Vec3 mins;
		Vec3 maxs;
		for (int iz = -1; iz <= 1; iz += 2)
		{
			for (int iy = -1; iy <= 1; iy += 2)
			{
				for (int ix = -1; ix <= 1; ix += 2)
				{
					Vec3 world = obb.LocalPosToWorldPos( Vec3( (float)ix * h.x, (float)iy * h.y, (float)iz * h.z ) );
					if (!hasPoint)
					{
						mins = world;
						maxs = world;
						hasPoint = true;
					}
					else
					{
						mins.x = (world.x < mins.x) ? world.x : mins.x;
						mins.y = (world.y < mins.y) ? world.y : mins.y;
						mins.z = (world.z < mins.z) ? world.z : mins.z;
						maxs.x = (world.x > maxs.x) ? world.x : maxs.x;
						maxs.y = (world.y > maxs.y) ? world.y : maxs.y;
						maxs.z = (world.z > maxs.z) ? world.z : maxs.z;
					}
				}
			}
		}
		return AABB3( mins, maxs );
	}

	NavSurfaceKind ClassifyFlatSurface( float topZ )
	{
		if (topZ < 1.5f)
		{
			return NavSurfaceKind::Ground;
		}
		if (topZ < 5.f)
		{
			return NavSurfaceKind::Gallery;
		}
		return NavSurfaceKind::Other;
	}

	Rgba8 GetSurfaceDebugColor( NavSurfaceKind kind )
	{
		switch (kind)
		{
			case NavSurfaceKind::Ground:	return Rgba8( 40, 220, 80, 255 );
			case NavSurfaceKind::Gallery:	return Rgba8( 240, 200, 40, 255 );
			case NavSurfaceKind::Slope:		return Rgba8( 40, 200, 255, 255 );
			default:						return Rgba8( 200, 200, 200, 255 );
		}
	}

	float DistPointToSegmentSquared( Vec3 const& point, Vec3 const& a, Vec3 const& b )
	{
		Vec3 ab = b - a;
		float abLenSq = ab.GetLengthSquared();
		if (abLenSq < 0.000001f)
		{
			return (point - a).GetLengthSquared();
		}

		float t = DotProduct3D( point - a, ab ) / abLenSq;
		if (t < 0.f)
		{
			t = 0.f;
		}
		else if (t > 1.f)
		{
			t = 1.f;
		}
		Vec3 closest = a + ab * t;
		return (point - closest).GetLengthSquared();
	}

	bool IsPointNearSegment( Vec3 const& point, Vec3 const& a, Vec3 const& b, float epsilon )
	{
		return DistPointToSegmentSquared( point, a, b ) <= (epsilon * epsilon);
	}

	bool AreEdgesNearlyParallel( Vec3 const& a0, Vec3 const& a1, Vec3 const& b0, Vec3 const& b1 )
	{
		Vec3 aDir = a1 - a0;
		Vec3 bDir = b1 - b0;
		float aLenSq = aDir.GetLengthSquared();
		float bLenSq = bDir.GetLengthSquared();
		if (aLenSq < 0.000001f || bLenSq < 0.000001f)
		{
			return false;
		}
		aDir *= (1.f / sqrtf( aLenSq ));
		bDir *= (1.f / sqrtf( bLenSq ));
		float absDot = fabsf( DotProduct3D( aDir, bDir ) );
		return absDot >= 0.9f;
	}

	bool DoEdgesMatch( Vec3 const& a0, Vec3 const& a1, Vec3 const& b0, Vec3 const& b1 )
	{
		float const eps = NAV_EDGE_MATCH_EPSILON;
		float const epsSq = eps * eps;

		// Exact-ish shared endpoints (same or reversed winding).
		bool sameOrder = (a0 - b0).GetLengthSquared() <= epsSq && (a1 - b1).GetLengthSquared() <= epsSq;
		bool reversed = (a0 - b1).GetLengthSquared() <= epsSq && (a1 - b0).GetLengthSquared() <= epsSq;
		if (sameOrder || reversed)
		{
			return true;
		}

		// Partial shared edge (e.g. ramp width 4 against gallery edge length 15).
		if (!AreEdgesNearlyParallel( a0, a1, b0, b1 ))
		{
			return false;
		}

		Vec3 aMid = (a0 + a1) * 0.5f;
		Vec3 bMid = (b0 + b1) * 0.5f;
		if (fabsf( aMid.z - bMid.z ) > NAV_EDGE_HEIGHT_EPSILON)
		{
			return false;
		}

		int aOnB = (IsPointNearSegment( a0, b0, b1, eps ) ? 1 : 0) + (IsPointNearSegment( a1, b0, b1, eps ) ? 1 : 0);
		int bOnA = (IsPointNearSegment( b0, a0, a1, eps ) ? 1 : 0) + (IsPointNearSegment( b1, a0, a1, eps ) ? 1 : 0);
		if (aOnB == 2 || bOnA == 2)
		{
			return true;
		}
		return aOnB >= 1 && bOnA >= 1;
	}

	bool AlreadyNeighbors( NavPoly const& poly, int neighborId )
	{
		for (int id : poly.m_neighbors)
		{
			if (id == neighborId)
			{
				return true;
			}
		}
		return false;
	}

	Vec3 GetPolyNormal( NavPoly const& poly )
	{
		if (poly.m_verts.size() < 3)
		{
			return Vec3( 0.f, 0.f, 1.f );
		}
		Vec3 e0 = poly.m_verts[1] - poly.m_verts[0];
		Vec3 e1 = poly.m_verts[2] - poly.m_verts[0];
		Vec3 n = CrossProduct3D( e0, e1 );
		float lenSq = n.GetLengthSquared();
		if (lenSq < 0.000001f)
		{
			return Vec3( 0.f, 0.f, 1.f );
		}
		return n * (1.f / sqrtf( lenSq ));
	}

	bool IsPointInsideConvexPoly( Vec3 const& pointOnPlane, NavPoly const& poly, Vec3 const& normal )
	{
		int const vertCount = (int)poly.m_verts.size();
		if (vertCount < 3)
		{
			return false;
		}

		for (int i = 0; i < vertCount; ++i)
		{
			Vec3 const& a = poly.m_verts[i];
			Vec3 const& b = poly.m_verts[(i + 1) % vertCount];
			Vec3 edge = b - a;
			Vec3 toPoint = pointOnPlane - a;
			Vec3 cross = CrossProduct3D( edge, toPoint );
			if (DotProduct3D( cross, normal ) < -0.0001f)
			{
				return false;
			}
		}
		return true;
	}

	bool DoesEdgeLieOnPoly( Vec3 const& a0, Vec3 const& a1, NavPoly const& poly )
	{
		if (poly.m_verts.size() < 3)
		{
			return false;
		}

		Vec3 normal = GetPolyNormal( poly );
		Vec3 center = poly.GetCenter();
		float d0 = DotProduct3D( a0 - center, normal );
		float d1 = DotProduct3D( a1 - center, normal );
		if (fabsf( d0 ) > NAV_EDGE_HEIGHT_EPSILON || fabsf( d1 ) > NAV_EDGE_HEIGHT_EPSILON)
		{
			return false;
		}

		Vec3 p0 = a0 - normal * d0;
		Vec3 p1 = a1 - normal * d1;
		return IsPointInsideConvexPoly( p0, poly, normal ) && IsPointInsideConvexPoly( p1, poly, normal );
	}

	void TryLinkPolys( NavPoly& polyA, NavPoly& polyB )
	{
		if (!AlreadyNeighbors( polyA, polyB.m_id ))
		{
			polyA.m_neighbors.push_back( polyB.m_id );
		}
		if (!AlreadyNeighbors( polyB, polyA.m_id ))
		{
			polyB.m_neighbors.push_back( polyA.m_id );
		}
	}

	Vec3 ClosestPointOnSegment3D( Vec3 const& point, Vec3 const& a, Vec3 const& b )
	{
		Vec3 ab = b - a;
		float abLenSq = ab.GetLengthSquared();
		if (abLenSq < 0.000001f)
		{
			return a;
		}
		float t = DotProduct3D( point - a, ab ) / abLenSq;
		if (t < 0.f)
		{
			t = 0.f;
		}
		else if (t > 1.f)
		{
			t = 1.f;
		}
		return a + ab * t;
	}

	Vec3 ClampPointToConvexPoly( Vec3 const& pointOnPlane, NavPoly const& poly, Vec3 const& normal )
	{
		if (IsPointInsideConvexPoly( pointOnPlane, poly, normal ))
		{
			return pointOnPlane;
		}

		float bestDistSq = 1e30f;
		Vec3 bestPoint = poly.GetCenter();
		int const vertCount = (int)poly.m_verts.size();
		for (int i = 0; i < vertCount; ++i)
		{
			Vec3 closest = ClosestPointOnSegment3D( pointOnPlane, poly.m_verts[i], poly.m_verts[(i + 1) % vertCount] );
			float distSq = (closest - pointOnPlane).GetLengthSquared();
			if (distSq < bestDistSq)
			{
				bestDistSq = distSq;
				bestPoint = closest;
			}
		}
		return bestPoint;
	}

	// Signed area in XY (Z-up engine). >0 means c is left of directed edge a->b.
	float TriArea2D( Vec3 const& a, Vec3 const& b, Vec3 const& c )
	{
		float const abx = b.x - a.x;
		float const aby = b.y - a.y;
		float const acx = c.x - a.x;
		float const acy = c.y - a.y;
		return acx * aby - abx * acy;
	}

	bool VEqual2D( Vec3 const& a, Vec3 const& b )
	{
		float const dx = a.x - b.x;
		float const dy = a.y - b.y;
		return (dx * dx + dy * dy) <= (NAV_FUNNEL_POINT_EPSILON * NAV_FUNNEL_POINT_EPSILON);
	}

	bool ComputeEdgeOverlapSegment( Vec3 const& a0, Vec3 const& a1, Vec3 const& b0, Vec3 const& b1,
		Vec3& outP0, Vec3& outP1 )
	{
		Vec3 ab = a1 - a0;
		float abLen = ab.GetLength();
		if (abLen < 0.0001f)
		{
			return false;
		}
		Vec3 dir = ab * (1.f / abLen);

		auto projectT = [&]( Vec3 const& p ) -> float
		{
			return DotProduct3D( p - a0, dir );
		};

		float tA0 = 0.f;
		float tA1 = abLen;
		float tB0 = projectT( b0 );
		float tB1 = projectT( b1 );
		float tMin = (tA0 < tA1 ? tA0 : tA1);
		float tMax = (tA0 > tA1 ? tA0 : tA1);
		float tBMin = (tB0 < tB1 ? tB0 : tB1);
		float tBMax = (tB0 > tB1 ? tB0 : tB1);
		tMin = (tMin > tBMin) ? tMin : tBMin;
		tMax = (tMax < tBMax) ? tMax : tBMax;

		if (tMax < tMin - 0.05f)
		{
			return false;
		}
		if (tMax < tMin)
		{
			float t = 0.5f * (tMin + tMax);
			outP0 = a0 + dir * t;
			outP1 = outP0;
			return true;
		}

		outP0 = a0 + dir * tMin;
		outP1 = a0 + dir * tMax;
		return true;
	}

	void OrientPortalEnds( Vec3 const& p0, Vec3 const& p1, Vec3 const& travelDir, Vec3& outLeft, Vec3& outRight )
	{
		Vec3 travel = travelDir;
		if (travel.GetLengthSquared() < 0.000001f)
		{
			travel = Vec3( 1.f, 0.f, 0.f );
		}
		else
		{
			travel = travel.GetNormalized();
		}

		Vec3 up( 0.f, 0.f, 1.f );
		Vec3 leftDir = CrossProduct3D( up, travel );
		if (leftDir.GetLengthSquared() < 0.000001f)
		{
			leftDir = CrossProduct3D( Vec3( 1.f, 0.f, 0.f ), travel );
		}
		leftDir = leftDir.GetNormalized();

		Vec3 mid = (p0 + p1) * 0.5f;
		if (DotProduct3D( p0 - mid, leftDir ) >= DotProduct3D( p1 - mid, leftDir ))
		{
			outLeft = p0;
			outRight = p1;
		}
		else
		{
			outLeft = p1;
			outRight = p0;
		}
	}

	// Corridor = A* poly sequence. Final path = string-pull (funnel) on portals — UE/Detour style.
	void StringPull( Vec3 const& start, Vec3 const& goal, std::vector<NavPortal> const& portals,
		std::vector<Vec3>& outWaypoints )
	{
		outWaypoints.clear();
		outWaypoints.push_back( start );

		struct PortalPts
		{
			Vec3 left;
			Vec3 right;
		};

		std::vector<PortalPts> pts;
		pts.reserve( portals.size() + 1 );
		for (NavPortal const& portal : portals)
		{
			pts.push_back( PortalPts{ portal.m_left, portal.m_right } );
		}
		// Treat goal as a degenerate portal so the funnel tightens onto the destination.
		pts.push_back( PortalPts{ goal, goal } );

		Vec3 portalApex = start;
		Vec3 portalLeft = start;
		Vec3 portalRight = start;
		int apexIndex = 0;
		int leftIndex = 0;
		int rightIndex = 0;

		for (int i = 0; i < (int)pts.size(); ++i)
		{
			Vec3 const& left = pts[i].left;
			Vec3 const& right = pts[i].right;

			// Update right funnel side.
			if (TriArea2D( portalApex, portalRight, right ) <= 0.f)
			{
				if (VEqual2D( portalApex, portalRight ) || TriArea2D( portalApex, portalLeft, right ) > 0.f)
				{
					portalRight = right;
					rightIndex = i;
				}
				else
				{
					// Right crossed left: make left the new apex / corner.
					if (!VEqual2D( outWaypoints.back(), portalLeft ))
					{
						outWaypoints.push_back( portalLeft );
					}
					portalApex = portalLeft;
					apexIndex = leftIndex;
					portalLeft = portalApex;
					portalRight = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;
					i = apexIndex;
					continue;
				}
			}

			// Update left funnel side.
			if (TriArea2D( portalApex, portalLeft, left ) >= 0.f)
			{
				if (VEqual2D( portalApex, portalLeft ) || TriArea2D( portalApex, portalRight, left ) < 0.f)
				{
					portalLeft = left;
					leftIndex = i;
				}
				else
				{
					// Left crossed right: make right the new apex / corner.
					if (!VEqual2D( outWaypoints.back(), portalRight ))
					{
						outWaypoints.push_back( portalRight );
					}
					portalApex = portalRight;
					apexIndex = rightIndex;
					portalLeft = portalApex;
					portalRight = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;
					i = apexIndex;
					continue;
				}
			}
		}

		if (outWaypoints.empty() || !VEqual2D( outWaypoints.back(), goal ))
		{
			// Keep full 3D goal (Z matters on ramps/gallery).
			if (outWaypoints.empty() || (outWaypoints.back() - goal).GetLengthSquared() > NAV_WAYPOINT_DEDUP_EPSILON * NAV_WAYPOINT_DEDUP_EPSILON)
			{
				outWaypoints.push_back( goal );
			}
			else
			{
				outWaypoints.back() = goal;
			}
		}
	}
}

Vec3 NavPoly::GetCenter() const
{
	if (m_verts.empty())
	{
		return Vec3::ZERO;
	}

	Vec3 sum = Vec3::ZERO;
	for (Vec3 const& vert : m_verts)
	{
		sum += vert;
	}
	return sum * (1.f / (float)m_verts.size());
}

void NavMesh::Clear()
{
	m_name.clear();
	m_volumeBounds = AABB3();
	m_polys.clear();
	m_debugOverlayVerts.clear();
}

void NavMesh::BakeFrom( Map const& map, NavMeshVolumeDefinition const& volumeDef )
{
	Clear();
	m_name = volumeDef.m_name;
	m_volumeBounds = volumeDef.GetBounds();
	BuildCandidates( map );
	BuildAdjacency();
	RebuildDebugOverlayVerts();

	DebugAddMessage( Stringf( "NavMesh '%s' bake: %d polys", m_name.c_str(), GetPolyCount() ),
		5.f, Rgba8::CYAN, Rgba8::CYAN );
}

void NavMesh::BuildCandidates( Map const& map )
{
	AABB3 const& volume = m_volumeBounds;

	std::vector<AABB3> const& solidAabbs = map.GetSolidBounds();
	std::vector<bool> const& aabbWalkable = map.GetSolidBoundsNavWalkable();
	GUARANTEE_OR_DIE( solidAabbs.size() == aabbWalkable.size(), "AABB solid/navWalkable size mismatch" );

	for (int i = 0; i < (int)solidAabbs.size(); ++i)
	{
		if (!aabbWalkable[i])
		{
			continue;
		}

		AABB3 const& bounds = solidAabbs[i];
		if (!DoAABBsOverlap3D( bounds, volume ))
		{
			continue;
		}

		float topZ = bounds.m_maxs.z;
		Vec3 topCenter( (bounds.m_mins.x + bounds.m_maxs.x) * 0.5f,
			(bounds.m_mins.y + bounds.m_maxs.y) * 0.5f,
			topZ );
		if (!IsPointInsideAABB3Inclusive( topCenter, volume ))
		{
			continue;
		}

		NavPoly poly;
		poly.m_id = (int)m_polys.size();
		poly.m_kind = ClassifyFlatSurface( topZ );
		poly.m_verts.push_back( Vec3( bounds.m_mins.x, bounds.m_mins.y, topZ ) );
		poly.m_verts.push_back( Vec3( bounds.m_maxs.x, bounds.m_mins.y, topZ ) );
		poly.m_verts.push_back( Vec3( bounds.m_maxs.x, bounds.m_maxs.y, topZ ) );
		poly.m_verts.push_back( Vec3( bounds.m_mins.x, bounds.m_maxs.y, topZ ) );
		m_polys.push_back( poly );
	}

	std::vector<OBB3> const& solidObbs = map.GetSolidObbs();
	std::vector<bool> const& obbWalkable = map.GetSolidObbsNavWalkable();
	GUARANTEE_OR_DIE( solidObbs.size() == obbWalkable.size(), "OBB solid/navWalkable size mismatch" );

	for (int i = 0; i < (int)solidObbs.size(); ++i)
	{
		if (!obbWalkable[i])
		{
			continue;
		}

		OBB3 const& obb = solidObbs[i];
		AABB3 worldBounds = MakeWorldBoundsFromOBB( obb );
		if (!DoAABBsOverlap3D( worldBounds, volume ))
		{
			continue;
		}

		Vec3 const& h = obb.m_halfDimensions;
		// +K face, matching Map FACE_TOP origin (c - hi - hj + hk) then +I/+J.
		Vec3 v0 = obb.LocalPosToWorldPos( Vec3( -h.x, -h.y, h.z ) );
		Vec3 v1 = obb.LocalPosToWorldPos( Vec3(  h.x, -h.y, h.z ) );
		Vec3 v2 = obb.LocalPosToWorldPos( Vec3(  h.x,  h.y, h.z ) );
		Vec3 v3 = obb.LocalPosToWorldPos( Vec3( -h.x,  h.y, h.z ) );
		Vec3 topCenter = (v0 + v1 + v2 + v3) * 0.25f;
		if (!IsPointInsideAABB3Inclusive( topCenter, volume ))
		{
			continue;
		}

		NavPoly poly;
		poly.m_id = (int)m_polys.size();
		poly.m_kind = NavSurfaceKind::Slope;
		poly.m_verts.push_back( v0 );
		poly.m_verts.push_back( v1 );
		poly.m_verts.push_back( v2 );
		poly.m_verts.push_back( v3 );
		m_polys.push_back( poly );
	}
}

void NavMesh::BuildAdjacency()
{
	int const polyCount = (int)m_polys.size();
	for (int i = 0; i < polyCount; ++i)
	{
		NavPoly& polyA = m_polys[i];
		int const edgeCountA = (int)polyA.m_verts.size();
		for (int j = i + 1; j < polyCount; ++j)
		{
			NavPoly& polyB = m_polys[j];
			int const edgeCountB = (int)polyB.m_verts.size();
			bool linked = false;

			for (int ea = 0; ea < edgeCountA && !linked; ++ea)
			{
				Vec3 const& a0 = polyA.m_verts[ea];
				Vec3 const& a1 = polyA.m_verts[(ea + 1) % edgeCountA];
				for (int eb = 0; eb < edgeCountB; ++eb)
				{
					Vec3 const& b0 = polyB.m_verts[eb];
					Vec3 const& b1 = polyB.m_verts[(eb + 1) % edgeCountB];
					if (DoEdgesMatch( a0, a1, b0, b1 ))
					{
						linked = true;
						break;
					}
				}
			}

			// Ramp low edge sits on Floor interior (not on Floor's outer border).
			if (!linked)
			{
				for (int ea = 0; ea < edgeCountA && !linked; ++ea)
				{
					Vec3 const& a0 = polyA.m_verts[ea];
					Vec3 const& a1 = polyA.m_verts[(ea + 1) % edgeCountA];
					if (DoesEdgeLieOnPoly( a0, a1, polyB ))
					{
						linked = true;
					}
				}
			}
			if (!linked)
			{
				for (int eb = 0; eb < edgeCountB && !linked; ++eb)
				{
					Vec3 const& b0 = polyB.m_verts[eb];
					Vec3 const& b1 = polyB.m_verts[(eb + 1) % edgeCountB];
					if (DoesEdgeLieOnPoly( b0, b1, polyA ))
					{
						linked = true;
					}
				}
			}

			if (linked)
			{
				TryLinkPolys( polyA, polyB );
			}
		}
	}
}

int NavMesh::FindPoly( Vec3 const& position ) const
{
	int bestId = -1;
	float bestDistSq = NAV_FIND_POLY_MAX_DIST * NAV_FIND_POLY_MAX_DIST;

	for (NavPoly const& poly : m_polys)
	{
		if (poly.m_verts.size() < 3)
		{
			continue;
		}

		Vec3 normal = GetPolyNormal( poly );
		Vec3 center = poly.GetCenter();
		float planeDist = DotProduct3D( position - center, normal );
		Vec3 onPlane = position - normal * planeDist;
		if (!IsPointInsideConvexPoly( onPlane, poly, normal ))
		{
			continue;
		}

		float distSq = (position - onPlane).GetLengthSquared();
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			bestId = poly.m_id;
		}
	}

	return bestId;
}

NavPoly const* NavMesh::GetPoly( int polyId ) const
{
	if (polyId < 0 || polyId >= (int)m_polys.size())
	{
		return nullptr;
	}
	return &m_polys[polyId];
}

Vec3 NavMesh::ProjectOntoPoly( NavPoly const& poly, Vec3 const& position ) const
{
	if (poly.m_verts.size() < 3)
	{
		return poly.GetCenter();
	}

	Vec3 normal = GetPolyNormal( poly );
	Vec3 center = poly.GetCenter();
	float planeDist = DotProduct3D( position - center, normal );
	Vec3 onPlane = position - normal * planeDist;
	return ClampPointToConvexPoly( onPlane, poly, normal );
}

bool NavMesh::TryGetPortal( NavPoly const& polyA, NavPoly const& polyB, Vec3 const& travelDir, NavPortal& outPortal ) const
{
	int const edgeCountA = (int)polyA.m_verts.size();
	int const edgeCountB = (int)polyB.m_verts.size();

	Vec3 bestP0;
	Vec3 bestP1;
	float bestLenSq = 1e30f;
	bool found = false;

	// 1) Shared / partially-overlapping edges → portal = overlap sub-segment.
	for (int ea = 0; ea < edgeCountA; ++ea)
	{
		Vec3 const& a0 = polyA.m_verts[ea];
		Vec3 const& a1 = polyA.m_verts[(ea + 1) % edgeCountA];
		for (int eb = 0; eb < edgeCountB; ++eb)
		{
			Vec3 const& b0 = polyB.m_verts[eb];
			Vec3 const& b1 = polyB.m_verts[(eb + 1) % edgeCountB];
			if (!DoEdgesMatch( a0, a1, b0, b1 ))
			{
				continue;
			}

			Vec3 p0;
			Vec3 p1;
			if (!ComputeEdgeOverlapSegment( a0, a1, b0, b1, p0, p1 ))
			{
				// Near-match but no clear projection overlap: use the shorter edge.
				float lenASq = (a1 - a0).GetLengthSquared();
				float lenBSq = (b1 - b0).GetLengthSquared();
				if (lenASq <= lenBSq)
				{
					p0 = a0;
					p1 = a1;
				}
				else
				{
					p0 = b0;
					p1 = b1;
				}
			}

			float lenSq = (p1 - p0).GetLengthSquared();
			if (!found || lenSq < bestLenSq)
			{
				found = true;
				bestLenSq = lenSq;
				bestP0 = p0;
				bestP1 = p1;
			}
		}
	}

	// 2) Edge of one poly lying on the other (ramp bottom on floor).
	if (!found)
	{
		for (int ea = 0; ea < edgeCountA; ++ea)
		{
			Vec3 const& a0 = polyA.m_verts[ea];
			Vec3 const& a1 = polyA.m_verts[(ea + 1) % edgeCountA];
			if (DoesEdgeLieOnPoly( a0, a1, polyB ))
			{
				found = true;
				bestP0 = a0;
				bestP1 = a1;
				break;
			}
		}
	}
	if (!found)
	{
		for (int eb = 0; eb < edgeCountB; ++eb)
		{
			Vec3 const& b0 = polyB.m_verts[eb];
			Vec3 const& b1 = polyB.m_verts[(eb + 1) % edgeCountB];
			if (DoesEdgeLieOnPoly( b0, b1, polyA ))
			{
				found = true;
				bestP0 = b0;
				bestP1 = b1;
				break;
			}
		}
	}

	if (!found)
	{
		return false;
	}

	OrientPortalEnds( bestP0, bestP1, travelDir, outPortal.m_left, outPortal.m_right );
	return true;
}

bool NavMesh::TryGetPortalMidpoint( NavPoly const& polyA, NavPoly const& polyB, Vec3& outMid ) const
{
	Vec3 travel = polyB.GetCenter() - polyA.GetCenter();
	NavPortal portal;
	if (!TryGetPortal( polyA, polyB, travel, portal ))
	{
		return false;
	}
	outMid = (portal.m_left + portal.m_right) * 0.5f;
	return true;
}

void NavMesh::AppendDedupedWaypoint( std::vector<Vec3>& waypoints, Vec3 const& point ) const
{
	if (!waypoints.empty())
	{
		float const epsSq = NAV_WAYPOINT_DEDUP_EPSILON * NAV_WAYPOINT_DEDUP_EPSILON;
		if ((waypoints.back() - point).GetLengthSquared() <= epsSq)
		{
			return;
		}
	}
	waypoints.push_back( point );
}

bool NavMesh::RunAStar( int startPolyId, int goalPolyId, std::vector<int>& outCorridor ) const
{
	outCorridor.clear();
	int const polyCount = (int)m_polys.size();
	if (startPolyId < 0 || goalPolyId < 0 || startPolyId >= polyCount || goalPolyId >= polyCount)
	{
		return false;
	}
	if (startPolyId == goalPolyId)
	{
		outCorridor.push_back( startPolyId );
		return true;
	}

	std::vector<float> gScore( polyCount, 1e30f );
	std::vector<int> cameFrom( polyCount, -1 );
	std::vector<bool> closed( polyCount, false );
	std::vector<bool> inOpen( polyCount, false );
	std::vector<int> open;
	open.reserve( polyCount );

	Vec3 const goalCenter = m_polys[goalPolyId].GetCenter();
	gScore[startPolyId] = 0.f;
	open.push_back( startPolyId );
	inOpen[startPolyId] = true;

	while (!open.empty())
	{
		int bestIndex = 0;
		float bestF = gScore[open[0]] + (m_polys[open[0]].GetCenter() - goalCenter).GetLength();
		for (int i = 1; i < (int)open.size(); ++i)
		{
			int id = open[i];
			float f = gScore[id] + (m_polys[id].GetCenter() - goalCenter).GetLength();
			if (f < bestF)
			{
				bestF = f;
				bestIndex = i;
			}
		}

		int current = open[bestIndex];
		open[bestIndex] = open.back();
		open.pop_back();
		inOpen[current] = false;

		if (current == goalPolyId)
		{
			int cursor = goalPolyId;
			while (cursor != -1)
			{
				outCorridor.push_back( cursor );
				cursor = cameFrom[cursor];
			}
			// reverse start->goal
			for (int i = 0, j = (int)outCorridor.size() - 1; i < j; ++i, --j)
			{
				int tmp = outCorridor[i];
				outCorridor[i] = outCorridor[j];
				outCorridor[j] = tmp;
			}
			return true;
		}

		closed[current] = true;
		NavPoly const& currentPoly = m_polys[current];
		Vec3 currentCenter = currentPoly.GetCenter();

		for (int neighborId : currentPoly.m_neighbors)
		{
			if (neighborId < 0 || neighborId >= polyCount || closed[neighborId])
			{
				continue;
			}

			float tentativeG = gScore[current] + (m_polys[neighborId].GetCenter() - currentCenter).GetLength();
			if (tentativeG >= gScore[neighborId])
			{
				continue;
			}

			cameFrom[neighborId] = current;
			gScore[neighborId] = tentativeG;
			if (!inOpen[neighborId])
			{
				open.push_back( neighborId );
				inOpen[neighborId] = true;
			}
		}
	}

	return false;
}

NavPath NavMesh::FindPath( Vec3 const& startPos, Vec3 const& goalPos ) const
{
	NavPath path;

	int startPolyId = FindPoly( startPos );
	int goalPolyId = FindPoly( goalPos );
	if (startPolyId < 0 || goalPolyId < 0)
	{
		return path;
	}

	NavPoly const* startPoly = GetPoly( startPolyId );
	NavPoly const* goalPoly = GetPoly( goalPolyId );
	if (startPoly == nullptr || goalPoly == nullptr)
	{
		return path;
	}

	Vec3 startProjected = ProjectOntoPoly( *startPoly, startPos );
	Vec3 goalProjected = ProjectOntoPoly( *goalPoly, goalPos );

	if (startPolyId == goalPolyId)
	{
		AppendDedupedWaypoint( path.m_waypoints, startProjected );
		AppendDedupedWaypoint( path.m_waypoints, goalProjected );
		path.m_isValid = !path.m_waypoints.empty();
		return path;
	}

	std::vector<int> corridor;
	if (!RunAStar( startPolyId, goalPolyId, corridor ) || corridor.size() < 2)
	{
		return path;
	}

	// A* builds the poly corridor; string-pull (funnel) produces the final walkable path.
	std::vector<NavPortal> portals;
	portals.reserve( corridor.size() - 1 );
	for (int i = 0; i + 1 < (int)corridor.size(); ++i)
	{
		NavPoly const* a = GetPoly( corridor[i] );
		NavPoly const* b = GetPoly( corridor[i + 1] );
		if (a == nullptr || b == nullptr)
		{
			continue;
		}

		Vec3 travel = b->GetCenter() - a->GetCenter();
		NavPortal portal;
		if (!TryGetPortal( *a, *b, travel, portal ))
		{
			// Fallback: degenerate point portal between centers (keeps funnel alive).
			Vec3 mid = (a->GetCenter() + b->GetCenter()) * 0.5f;
			portal.m_left = mid;
			portal.m_right = mid;
			DebuggerPrintf( "NavMesh '%s': portal fallback between poly %d and %d (neighbors but no shared edge)\n",
				m_name.c_str(), a->m_id, b->m_id );
		}
		portals.push_back( portal );
	}

	path.m_portals = portals;
	StringPull( startProjected, goalProjected, portals, path.m_waypoints );

	// Dedup near-duplicates from funnel corners.
	std::vector<Vec3> cleaned;
	cleaned.reserve( path.m_waypoints.size() );
	for (Vec3 const& wp : path.m_waypoints)
	{
		AppendDedupedWaypoint( cleaned, wp );
	}
	path.m_waypoints.swap( cleaned );

	path.m_isValid = path.m_waypoints.size() >= 2;
	return path;
}

void NavMesh::DebugRender() const
{
	if (!DebugRenderIsVisible())
	{
		return;
	}

	for (NavPoly const& poly : m_polys)
	{
		Rgba8 color = GetSurfaceDebugColor( poly.m_kind );
		Vec3 normal = GetPolyNormal( poly );
		Vec3 lift = normal * NAV_OVERLAY_OFFSET;
		int const vertCount = (int)poly.m_verts.size();
		for (int i = 0; i < vertCount; ++i)
		{
			Vec3 a = poly.m_verts[i] + lift;
			Vec3 b = poly.m_verts[(i + 1) % vertCount] + lift;
			DebugAddWorldLine( a, b, NAV_DEBUG_LINE_RADIUS, NAV_DEBUG_DRAW_DURATION, color, color, DebugRenderMode::USE_DEPTH );
		}

		// Neighbor links: short tick from center toward neighbor center.
		Vec3 center = poly.GetCenter() + lift + normal * 0.02f;
		for (int neighborId : poly.m_neighbors)
		{
			if (neighborId < 0 || neighborId >= (int)m_polys.size())
			{
				continue;
			}
			if (neighborId < poly.m_id)
			{
				continue; // draw each undirected link once
			}
			Vec3 neighborNormal = GetPolyNormal( m_polys[neighborId] );
			Vec3 neighborCenter = m_polys[neighborId].GetCenter() + neighborNormal * (NAV_OVERLAY_OFFSET + 0.02f);
			DebugAddWorldLine( center, neighborCenter, NAV_DEBUG_LINE_RADIUS * 0.75f, NAV_DEBUG_DRAW_DURATION,
				Rgba8( 255, 80, 255, 255 ), Rgba8( 255, 80, 255, 255 ), DebugRenderMode::USE_DEPTH );
		}
	}
}

void NavMesh::DebugRenderPath( std::vector<Vec3> const& waypoints ) const
{
	if (!DebugRenderIsVisible() || waypoints.size() < 2)
	{
		return;
	}

	Vec3 const lift( 0.f, 0.f, NAV_PATH_DRAW_LIFT );
	for (int i = 0; i < (int)waypoints.size(); ++i)
	{
		Vec3 p = waypoints[i] + lift;
		DebugAddWorldPoint( p, NAV_PATH_POINT_RADIUS, NAV_DEBUG_DRAW_DURATION, NAV_PATH_POINT_COLOR, NAV_PATH_POINT_COLOR, DebugRenderMode::USE_DEPTH );
		if (i + 1 < (int)waypoints.size())
		{
			Vec3 q = waypoints[i + 1] + lift;
			DebugAddWorldLine( p, q, NAV_DEBUG_LINE_RADIUS * 1.5f, NAV_DEBUG_DRAW_DURATION, NAV_PATH_COLOR, NAV_PATH_COLOR, DebugRenderMode::USE_DEPTH );
		}
	}
}

void NavMesh::DebugRenderPortals( std::vector<NavPortal> const& portals ) const
{
	if (!NAV_DEBUG_DRAW_PORTALS || !DebugRenderIsVisible())
	{
		return;
	}

	Vec3 const lift( 0.f, 0.f, NAV_PATH_DRAW_LIFT * 0.5f );
	for (NavPortal const& portal : portals)
	{
		Vec3 left = portal.m_left + lift;
		Vec3 right = portal.m_right + lift;
		DebugAddWorldLine( left, right, NAV_DEBUG_LINE_RADIUS * 0.6f, NAV_DEBUG_DRAW_DURATION,
			Rgba8( 200, 200, 255, 255 ), Rgba8( 200, 200, 255, 255 ), DebugRenderMode::USE_DEPTH );
		DebugAddWorldPoint( left, NAV_PATH_POINT_RADIUS * 0.7f, NAV_DEBUG_DRAW_DURATION,
			NAV_PORTAL_LEFT_COLOR, NAV_PORTAL_LEFT_COLOR, DebugRenderMode::USE_DEPTH );
		DebugAddWorldPoint( right, NAV_PATH_POINT_RADIUS * 0.7f, NAV_DEBUG_DRAW_DURATION,
			NAV_PORTAL_RIGHT_COLOR, NAV_PORTAL_RIGHT_COLOR, DebugRenderMode::USE_DEPTH );
	}
}

void NavMesh::RebuildDebugOverlayVerts()
{
	m_debugOverlayVerts.clear();
	m_debugOverlayVerts.reserve( m_polys.size() * 6 );

	for (NavPoly const& poly : m_polys)
	{
		if (poly.m_verts.size() < 3)
		{
			continue;
		}

		Vec3 normal = GetPolyNormal( poly );
		// Prefer upward-facing cover (match UE nav overlay looking down onto walkable).
		if (DotProduct3D( normal, Vec3( 0.f, 0.f, 1.f ) ) < 0.f)
		{
			normal = -normal;
		}
		Vec3 lift = normal * NAV_OVERLAY_OFFSET;

		// Fan triangulation from vert 0; quads become two tris via AddVertsForQuad3D when size==4.
		if (poly.m_verts.size() == 4)
		{
			AddVertsForQuad3D( m_debugOverlayVerts,
				poly.m_verts[0] + lift,
				poly.m_verts[1] + lift,
				poly.m_verts[2] + lift,
				poly.m_verts[3] + lift,
				NAV_OVERLAY_COLOR );
		}
		else
		{
			Vec3 const& v0 = poly.m_verts[0] + lift;
			for (int i = 1; i + 1 < (int)poly.m_verts.size(); ++i)
			{
				Vec3 const& v1 = poly.m_verts[i] + lift;
				Vec3 const& v2 = poly.m_verts[i + 1] + lift;
				m_debugOverlayVerts.push_back( Vertex_PCU( v0, NAV_OVERLAY_COLOR, Vec2::ZERO ) );
				m_debugOverlayVerts.push_back( Vertex_PCU( v1, NAV_OVERLAY_COLOR, Vec2::ZERO ) );
				m_debugOverlayVerts.push_back( Vertex_PCU( v2, NAV_OVERLAY_COLOR, Vec2::ZERO ) );
			}
		}
	}
}

void NavMesh::RenderWalkableOverlay() const
{
	if (!DebugRenderIsVisible() || m_debugOverlayVerts.empty())
	{
		return;
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( m_debugOverlayVerts );
}
