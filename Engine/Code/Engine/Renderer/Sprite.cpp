#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Renderer.hpp"

SpriteDefinition::SpriteDefinition( SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs )
	: m_spriteSheet( spriteSheet )
	, m_spriteIndex( spriteIndex )
	, m_uvAtMins( uvAtMins )
	, m_uvAtMaxs( uvAtMaxs )
{
}

void SpriteDefinition::GetUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs ) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}

AABB2 SpriteDefinition::GetUVs() const
{
	return AABB2( m_uvAtMins, m_uvAtMaxs );
}

SpriteSheet const& SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}

Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}

float SpriteDefinition::GetAspect() const
{
	return 0.0f;
}

SpriteSheet::SpriteSheet( Texture& diffuseMap, IntVec2 const& simpleGridLayout )
	: m_texture( diffuseMap )
	, m_dimensions( simpleGridLayout )
{
	Vec2 grid( 1.f / (float)simpleGridLayout.x, 1.f / (float)simpleGridLayout.y );
	Vec2 correctionOffset( grid.x / 128.f, grid.y / 128.f );

	m_spriteDefs.reserve( simpleGridLayout.x * simpleGridLayout.y );

	for (int gridIndex2DY = simpleGridLayout.y - 1; gridIndex2DY >= 0; gridIndex2DY--)
	{
		for (int gridIndex2DX = 0; gridIndex2DX < simpleGridLayout.x; gridIndex2DX++)
		{
			int reverseGridIndex2DY = simpleGridLayout.y - 1 - gridIndex2DY;
			Vec2 thisGridMin( gridIndex2DX * grid.x, gridIndex2DY * grid.y );
			int gridIndex = reverseGridIndex2DY * simpleGridLayout.x + gridIndex2DX;
			m_spriteDefs.push_back( SpriteDefinition( *this, gridIndex, thisGridMin + correctionOffset, thisGridMin + grid - correctionOffset ) );
		}
	}
}

Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return (int)m_spriteDefs.size();
}

SpriteDefinition const& SpriteSheet::GetSpriteDef( int spriteIndex ) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const
{
	m_spriteDefs[spriteIndex].GetUVs( out_uvAtMins, out_uvAtMaxs );
}

AABB2 SpriteSheet::GetSpriteUVs( int spriteIndex ) const
{
	return AABB2( m_spriteDefs[spriteIndex].GetUVs() );
}

AABB2 SpriteSheet::GetSpriteUVs( IntVec2 spriteCoords ) const
{
	int spriteIndex = spriteCoords.y * m_dimensions.x + spriteCoords.x;
	return AABB2( m_spriteDefs[spriteIndex].GetUVs() );
}
