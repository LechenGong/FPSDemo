#pragma once

#include <vector>

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

typedef Vec2 FloatRange;

class TileHeatMap
{
public:
	TileHeatMap() {};
	TileHeatMap( IntVec2 const& dimensions );
	~TileHeatMap();
	
	void ResetAllValue( float maxCost = TileHeatMap::MAX_VALUE );

	void AddValue( float newValue );

	void SetValue( int tileIndex, float newValue );

	IntVec2 GetDimensions() { return m_dimensions; }
	float GetValue( int tileIndex ) const;

	void AddVertsForDebugDraw( std::vector<Vertex_PCU>& verts, AABB2 bounds, FloatRange valueRange = FloatRange( 0.f, 1.f ),
		Rgba8 lowColor = Rgba8( 0, 0, 0, 100 ), Rgba8 highColor = Rgba8( 255, 255, 255, 100 ), float specialValue = TileHeatMap::MAX_VALUE, Rgba8 specialColor = Rgba8( 255, 0, 255 ) ) const;

	static const float MAX_VALUE;

private:

protected:
	IntVec2 m_dimensions = IntVec2::ZERO;
	std::vector<float> m_values;
};
