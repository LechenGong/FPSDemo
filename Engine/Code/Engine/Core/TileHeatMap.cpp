#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"

const float TileHeatMap::MAX_VALUE = 999999.f;

TileHeatMap::TileHeatMap( IntVec2 const& dimensions )
	: m_dimensions( dimensions )
{
	m_values.resize( dimensions.x * dimensions.y );
;}

TileHeatMap::~TileHeatMap()
{
}

void TileHeatMap::ResetAllValue( float maxCost )
{
	for (float& valueIndex : m_values)
	{
		valueIndex = maxCost;
	}
}

void TileHeatMap::AddValue( float newValue )
{
	m_values.push_back( newValue );
}

void TileHeatMap::SetValue( int tileIndex, float newValue )
{
// 	if (tileIndex >= m_values.size())
// 		return;
	m_values[tileIndex] = newValue;
}

float TileHeatMap::GetValue( int tileIndex ) const
{
	return m_values[tileIndex];
}

void TileHeatMap::AddVertsForDebugDraw( std::vector<Vertex_PCU>& verts, AABB2 bounds, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor ) const
{
	Vec2 tileSize;
	tileSize.x = (bounds.m_maxs.x - bounds.m_mins.x) / m_dimensions.x;
	tileSize.y = (bounds.m_maxs.y - bounds.m_mins.y) / m_dimensions.y;

	for (int j = 0; j < m_dimensions.y; j++)
	{
		for (int i = 0; i < m_dimensions.x; i++)
		{
			int index = j * m_dimensions.x + i;
			float value = m_values[index];
			AABB2 tileBound = AABB2(
				bounds.m_mins + Vec2( i * tileSize.x, j * tileSize.y ),
				bounds.m_mins + Vec2( i * tileSize.x + tileSize.x, j * tileSize.y + tileSize.y )
			);
			if (value == specialValue)
			{
				AddVertsForAABB2D( verts, tileBound, specialColor );
				continue;
			}
			float clampedValue = RangeMapClamped( value, valueRange.x, valueRange.y, 0.f, 1.f );
			Rgba8 interpColor = Interpolate( lowColor, highColor, clampedValue );
			AddVertsForAABB2D( verts, tileBound, interpColor );
		}
	}
}
