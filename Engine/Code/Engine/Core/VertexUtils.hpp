#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Mat44.hpp"

#include <vector>

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY );

void TransformVertexArrayXY3D( std::vector<Vertex_PCU>& verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY );

void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform );
void TransformVertexArray3D( std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform );

void CalculateTangantSpaceBasisVectors( std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, bool computeNormals, bool computeTangents );

Vertex_PCU GetTransformedVertex3D( Vertex_PCU vert, Mat44 const& transform );

Vec2 TransformPolarToCarteDegreeNormal( float rotationDegree );

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY );

void TransformVertexArrayXY3D( std::vector<Vertex_PCU>& verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY );

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color );
void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 uvAtMins, Vec2 uvAtMaxs );
void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& upperRight, Rgba8 color );
void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color );
void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color, Vec2 uvAtMins, Vec2 uvAtMaxs );
void AddVertsForHollowRect2D( std::vector<Vertex_PCU>& vertArray, float thickness, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color ); void AddVertsForRing2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, float const& thick, Rgba8 color );
void AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color );
void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color );
void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color );
void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color );
void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, Vec2 uvAtMins, Vec2 uvAtMaxs );
void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color  );
void AddVertsForQuadOutline2D( std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, Rgba8 const& color );

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForQuadOutline3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, float thickness, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForAABB3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16 );
void AddVertsForSphere3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16 );
void AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8 );
void AddVertsForCapsule3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8 );
void AddVertsForCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8 );
void AddVertsForHexigon3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void AddVertsForHexigonOutline3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float thickness, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );

AABB2 GetVertexBounds2D( std::vector<Vertex_PCU>& verts );