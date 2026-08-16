#include "Game/MapGeometryBaker.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Sprite.hpp"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr int FACE_TOP = 1 << 0;		// +Z / +K
	constexpr int FACE_BOTTOM = 1 << 1;	// -Z / -K
	constexpr int FACE_SIDES = 1 << 2;	// +/-X/Y or +/-I/J

	OBB3 MakeOBBFromDefinition( MapGeometryDefinition const& geometry )
	{
		Vec3 halfSize = geometry.m_size * 0.5f;
		return OBB3( geometry.m_center, geometry.m_iBasis, geometry.m_jBasis, geometry.m_kBasis, halfSize );
	}

	Texture* ResolveMaterialTexture( std::string const& materialName )
	{
		if (materialName == "Terrain" || materialName == "TEXTURE_SPRITESHEET_TERRAIN")
		{
			if (g_terrainSprite != nullptr)
			{
				return &g_terrainSprite->GetTexture();
			}
			return g_textures[TEXTURE_SPRITESHEET_TERRAIN];
		}

		if (materialName == "TestUV" || materialName == "TEXTURE_TEST_UV")
		{
			return g_textures[TEXTURE_TEST_UV];
		}

		ERROR_AND_DIE( Stringf( "Unknown map material: %s", materialName.c_str() ).c_str() );
	}

	AABB2 ResolveMaterialUVs( std::string const& materialName, IntVec2 const& spriteCoords )
	{
		if (materialName == "Terrain" || materialName == "TEXTURE_SPRITESHEET_TERRAIN")
		{
			GUARANTEE_OR_DIE( g_terrainSprite != nullptr, "g_terrainSprite was not loaded" );
			return g_terrainSprite->GetSpriteUVs( spriteCoords );
		}

		return AABB2::ZERO_TO_ONE;
	}

	MapRenderBatch& FindOrCreateBatch( MapGeometryBuildResult& result, Texture* texture, SamplerMode samplerMode )
	{
		for (MapRenderBatch& batch : result.renderBatches)
		{
			if (batch.m_texture == texture && batch.m_samplerMode == samplerMode)
			{
				return batch;
			}
		}

		MapRenderBatch newBatch;
		newBatch.m_texture = texture;
		newBatch.m_samplerMode = samplerMode;
		result.renderBatches.push_back( newBatch );
		return result.renderBatches.back();
	}

	void AppendStretchedAABBFace( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, AABB2 const& spriteUVs, int faceMask )
	{
		Vec3 point1( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );
		Vec3 point2( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
		Vec3 point3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
		Vec3 point4( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
		Vec3 point5( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
		Vec3 point6( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
		Vec3 point7( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );
		Vec3 point8( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );

		if (faceMask & FACE_TOP)
		{
			AddVertsForQuad3D( verts, point3, point4, point1, point2, Rgba8::WHITE, spriteUVs );
		}
		if (faceMask & FACE_BOTTOM)
		{
			AddVertsForQuad3D( verts, point6, point5, point8, point7, Rgba8::WHITE, spriteUVs );
		}
		if (faceMask & FACE_SIDES)
		{
			AddVertsForQuad3D( verts, point5, point6, point2, point1, Rgba8::WHITE, spriteUVs );
			AddVertsForQuad3D( verts, point7, point8, point4, point3, Rgba8::WHITE, spriteUVs );
			AddVertsForQuad3D( verts, point8, point5, point1, point4, Rgba8::WHITE, spriteUVs );
			AddVertsForQuad3D( verts, point6, point7, point3, point2, Rgba8::WHITE, spriteUVs );
		}
	}

	void AppendTiledAABBFace( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, AABB2 const& spriteUVs, int faceMask )
	{
		auto appendFaceTiles = [&]( Vec3 const& origin, Vec3 const& axisU, Vec3 const& axisV, float sizeU, float sizeV )
		{
			int tileCountU = (int)ceilf( sizeU );
			int tileCountV = (int)ceilf( sizeV );
			if (tileCountU < 1) tileCountU = 1;
			if (tileCountV < 1) tileCountV = 1;

			for (int v = 0; v < tileCountV; v++)
			{
				for (int u = 0; u < tileCountU; u++)
				{
					float u0 = (float)u;
					float v0 = (float)v;
					float u1 = Clamp( (float)(u + 1), 0.f, sizeU );
					float v1 = Clamp( (float)(v + 1), 0.f, sizeV );

					Vec3 p0 = origin + axisU * u0 + axisV * v0;
					Vec3 p1 = origin + axisU * u1 + axisV * v0;
					Vec3 p2 = origin + axisU * u1 + axisV * v1;
					Vec3 p3 = origin + axisU * u0 + axisV * v1;

					float fracU = u1 - u0;
					float fracV = v1 - v0;
					Vec2 uvMins = spriteUVs.m_mins;
					Vec2 uvMaxs = spriteUVs.m_mins + spriteUVs.GetDimensions() * Vec2( fracU, fracV );
					AddVertsForQuad3D( verts, p0, p1, p2, p3, Rgba8::WHITE, AABB2( uvMins, uvMaxs ) );
				}
			}
		};

		float sizeX = bounds.m_maxs.x - bounds.m_mins.x;
		float sizeY = bounds.m_maxs.y - bounds.m_mins.y;
		float sizeZ = bounds.m_maxs.z - bounds.m_mins.z;

		if (faceMask & FACE_TOP)
		{
			appendFaceTiles( Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), sizeX, sizeY );
		}
		if (faceMask & FACE_BOTTOM)
		{
			appendFaceTiles( Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, -1.f, 0.f ), sizeX, sizeY );
		}
		if (faceMask & FACE_SIDES)
		{
			appendFaceTiles( Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), sizeX, sizeZ );
			appendFaceTiles( Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), sizeX, sizeZ );
			appendFaceTiles( Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), sizeY, sizeZ );
			appendFaceTiles( Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z ), Vec3( 0.f, -1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), sizeY, sizeZ );
		}
	}

	void AppendStretchedOBBFace( std::vector<Vertex_PCU>& verts, OBB3 const& bounds, AABB2 const& spriteUVs, int faceMask )
	{
		Vec3 point1( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point2( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point3( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point4( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point5( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point6( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point7( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
		Vec3 point8( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );

		if (faceMask & FACE_TOP)
		{
			AddVertsForQuad3D( verts, point3, point4, point1, point2, Rgba8::WHITE, spriteUVs ); // +K
		}
		if (faceMask & FACE_BOTTOM)
		{
			AddVertsForQuad3D( verts, point6, point5, point8, point7, Rgba8::WHITE, spriteUVs ); // -K
		}
		if (faceMask & FACE_SIDES)
		{
			AddVertsForQuad3D( verts, point5, point6, point2, point1, Rgba8::WHITE, spriteUVs ); // +J
			AddVertsForQuad3D( verts, point7, point8, point4, point3, Rgba8::WHITE, spriteUVs ); // -J
			AddVertsForQuad3D( verts, point8, point5, point1, point4, Rgba8::WHITE, spriteUVs ); // +I
			AddVertsForQuad3D( verts, point6, point7, point3, point2, Rgba8::WHITE, spriteUVs ); // -I
		}
	}

	void AppendTiledOBBFace( std::vector<Vertex_PCU>& verts, OBB3 const& bounds, AABB2 const& spriteUVs, int faceMask )
	{
		auto appendFaceTiles = [&]( Vec3 const& origin, Vec3 const& axisU, Vec3 const& axisV, float sizeU, float sizeV )
		{
			int tileCountU = (int)ceilf( sizeU );
			int tileCountV = (int)ceilf( sizeV );
			if (tileCountU < 1) tileCountU = 1;
			if (tileCountV < 1) tileCountV = 1;

			for (int v = 0; v < tileCountV; v++)
			{
				for (int u = 0; u < tileCountU; u++)
				{
					float u0 = (float)u;
					float v0 = (float)v;
					float u1 = Clamp( (float)(u + 1), 0.f, sizeU );
					float v1 = Clamp( (float)(v + 1), 0.f, sizeV );

					Vec3 p0 = origin + axisU * u0 + axisV * v0;
					Vec3 p1 = origin + axisU * u1 + axisV * v0;
					Vec3 p2 = origin + axisU * u1 + axisV * v1;
					Vec3 p3 = origin + axisU * u0 + axisV * v1;

					float fracU = u1 - u0;
					float fracV = v1 - v0;
					Vec2 uvMins = spriteUVs.m_mins;
					Vec2 uvMaxs = spriteUVs.m_mins + spriteUVs.GetDimensions() * Vec2( fracU, fracV );
					AddVertsForQuad3D( verts, p0, p1, p2, p3, Rgba8::WHITE, AABB2( uvMins, uvMaxs ) );
				}
			}
		};

		float sizeI = bounds.m_halfDimensions.x * 2.f;
		float sizeJ = bounds.m_halfDimensions.y * 2.f;
		float sizeK = bounds.m_halfDimensions.z * 2.f;
		Vec3 i = bounds.m_iBasisNormal;
		Vec3 j = bounds.m_jBasisNormal;
		Vec3 k = bounds.m_kBasisNormal;
		Vec3 c = bounds.m_center;
		Vec3 hi = i * bounds.m_halfDimensions.x;
		Vec3 hj = j * bounds.m_halfDimensions.y;
		Vec3 hk = k * bounds.m_halfDimensions.z;

		if (faceMask & FACE_TOP)
		{
			// +K face: origin at -I -J +K corner, U along +I, V along +J
			appendFaceTiles( c - hi - hj + hk, i, j, sizeI, sizeJ );
		}
		if (faceMask & FACE_BOTTOM)
		{
			// -K face: origin at -I +J -K, U along +I, V along -J
			appendFaceTiles( c - hi + hj - hk, i, -j, sizeI, sizeJ );
		}
		if (faceMask & FACE_SIDES)
		{
			// +J
			appendFaceTiles( c + hi + hj - hk, -i, k, sizeI, sizeK );
			// -J
			appendFaceTiles( c - hi - hj - hk, i, k, sizeI, sizeK );
			// +I
			appendFaceTiles( c + hi - hj - hk, j, k, sizeJ, sizeK );
			// -I
			appendFaceTiles( c - hi + hj - hk, -j, k, sizeJ, sizeK );
		}
	}

	void AppendFaceGroupVerts_AABB( MapGeometryBuildResult& result, MapFaceMaterialDefinition const& faceDef, AABB3 const& bounds, int faceMask )
	{
		if (!faceDef.m_enabled)
		{
			return;
		}

		Texture* texture = ResolveMaterialTexture( faceDef.m_material );
		AABB2 spriteUVs = ResolveMaterialUVs( faceDef.m_material, faceDef.m_spriteCoords );
		MapRenderBatch& batch = FindOrCreateBatch( result, texture, SamplerMode::POINT_CLAMP );

		if (faceDef.m_wrapMode == MapTextureWrapMode::Stretch)
		{
			AppendStretchedAABBFace( batch.m_verts, bounds, spriteUVs, faceMask );
		}
		else
		{
			AppendTiledAABBFace( batch.m_verts, bounds, spriteUVs, faceMask );
		}
	}

	void AppendFaceGroupVerts_OBB( MapGeometryBuildResult& result, MapFaceMaterialDefinition const& faceDef, OBB3 const& bounds, int faceMask )
	{
		if (!faceDef.m_enabled)
		{
			return;
		}

		Texture* texture = ResolveMaterialTexture( faceDef.m_material );
		AABB2 spriteUVs = ResolveMaterialUVs( faceDef.m_material, faceDef.m_spriteCoords );
		MapRenderBatch& batch = FindOrCreateBatch( result, texture, SamplerMode::POINT_CLAMP );

		if (faceDef.m_wrapMode == MapTextureWrapMode::Stretch)
		{
			AppendStretchedOBBFace( batch.m_verts, bounds, spriteUVs, faceMask );
		}
		else
		{
			AppendTiledOBBFace( batch.m_verts, bounds, spriteUVs, faceMask );
		}
	}

	void AppendAABBGeometry( MapGeometryBuildResult& result, MapGeometryDefinition const& geometry )
	{
		Vec3 halfSize = geometry.m_size * 0.5f;
		AABB3 bounds( geometry.m_center - halfSize, geometry.m_center + halfSize );
		result.solidBounds.push_back( bounds );
		result.solidBoundsNavWalkable.push_back( geometry.m_navWalkable );

		AppendFaceGroupVerts_AABB( result, geometry.m_top, bounds, FACE_TOP );
		AppendFaceGroupVerts_AABB( result, geometry.m_bottom, bounds, FACE_BOTTOM );
		AppendFaceGroupVerts_AABB( result, geometry.m_side, bounds, FACE_SIDES );
	}

	void AppendOBBGeometry( MapGeometryBuildResult& result, MapGeometryDefinition const& geometry )
	{
		OBB3 bounds = MakeOBBFromDefinition( geometry );
		result.solidObbs.push_back( bounds );
		result.solidObbsNavWalkable.push_back( geometry.m_navWalkable );

		AppendFaceGroupVerts_OBB( result, geometry.m_top, bounds, FACE_TOP );
		AppendFaceGroupVerts_OBB( result, geometry.m_bottom, bounds, FACE_BOTTOM );
		AppendFaceGroupVerts_OBB( result, geometry.m_side, bounds, FACE_SIDES );
	}
}

MapGeometryBuildResult MapGeometryBaker::Bake( MapDefinition const& definition, std::vector<std::string> const& skipGeometryNames )
{
	MapGeometryBuildResult result;

	for (MapGeometryDefinition const& geometry : definition.m_geometries)
	{
		if (!geometry.m_name.empty() &&
			std::find( skipGeometryNames.begin(), skipGeometryNames.end(), geometry.m_name ) != skipGeometryNames.end())
		{
			continue;
		}

		if (geometry.m_type == MapGeometryType::OBB)
		{
			AppendOBBGeometry( result, geometry );
		}
		else
		{
			AppendAABBGeometry( result, geometry );
		}
	}

	return result;
}
