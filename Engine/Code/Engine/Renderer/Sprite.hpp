#pragma once

#include <vector>

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

class SpriteDefinition;

class SpriteSheet
{
public:
	explicit SpriteSheet( Texture& diffuseMap, IntVec2 const& simpleGridLayout );

	Texture& GetTexture() const;
	int GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef( int spriteIndex ) const;
	void GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	AABB2 GetSpriteUVs( int spriteIndex ) const;
	AABB2 GetSpriteUVs( IntVec2 spriteCoords ) const;

	IntVec2 m_dimensions;
protected:
	Texture& m_texture;
	std::vector<SpriteDefinition> m_spriteDefs;
};

class SpriteDefinition
{
public:
	explicit SpriteDefinition( SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs );
	void GetUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs ) const;
	AABB2 GetUVs() const;
	SpriteSheet const& GetSpriteSheet() const;
	Texture& GetTexture() const;
	float GetAspect() const;

protected:
	SpriteSheet const& m_spriteSheet;
	int m_spriteIndex = -1;
	Vec2 m_uvAtMins = Vec2::ZERO;
	Vec2 m_uvAtMaxs = Vec2::ONE;
};
