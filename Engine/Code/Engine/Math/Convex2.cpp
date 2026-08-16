#include "Engine/Math/Convex2.hpp"

ConvexPoly2::ConvexPoly2()
{
}

ConvexPoly2::~ConvexPoly2()
{
}

ConvexPoly2::ConvexPoly2( std::vector<Plane2> const& boundingPlanes )
{
	for (int i = 0; i < (int)boundingPlanes.size(); i++)
	{
		int nextIndex = i + 1;
		if (nextIndex >= boundingPlanes.size())
			nextIndex = 0;
		if (Plane2::DoPlane2Intersect( boundingPlanes[i], boundingPlanes[nextIndex] ))
		{
			m_verts.push_back( Plane2::GetIntersectPos( boundingPlanes[i], boundingPlanes[nextIndex] ) );
		}
	}
}

void ConvexPoly2::SetVert( Vec2 const& vert, int index )
{
	if (index < 0 || index >= m_verts.size())
	{
		m_verts.push_back( vert );
	}
	else
	{
		m_verts[index] = vert;
	}
}

void ConvexPoly2::ClearVerts()
{
	m_verts.clear();
}

std::vector<Vec2> ConvexPoly2::GetVerts() const
{
	return m_verts;
}

ConvexHull2::ConvexHull2()
{
}

ConvexHull2::~ConvexHull2()
{
}

bool ConvexHull2::IsPointInside( Vec2 const& pos ) const
{
	for (int i = 0; i < m_boundingPlanes.size(); i++)
	{
		if (m_boundingPlanes[i].IsPointInFrontOfPlane( pos ))
		{
			return false;
		}
	}
	return true;
}
