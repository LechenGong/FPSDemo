#pragma once

#include <vector>

#include "Engine/Math/Plane2.hpp"

class ConvexPoly2
{
public:
	ConvexPoly2();
	~ConvexPoly2();
	ConvexPoly2( std::vector<Plane2>const& boundingPlanes );

	void SetVert( Vec2 const& vert, int index = -1 );
	void ClearVerts();
	std::vector<Vec2> GetVerts() const;

private:
	std::vector<Vec2> m_verts;
};

class ConvexHull2
{
public:
	ConvexHull2();
	~ConvexHull2();

	bool IsPointInside( Vec2 const& pos ) const;

public:
	std::vector<Plane2> m_boundingPlanes;
};
