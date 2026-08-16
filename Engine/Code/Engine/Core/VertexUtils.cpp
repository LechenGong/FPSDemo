#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY )
{

	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
	{
		TransformPositionXY3D( verts[vertIndex].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY );
	}
}

void TransformVertexArrayXY3D( std::vector<Vertex_PCU> &verts, float uniformScaleXY,
	float rotationDegreesAboutZ, Vec2 const& translationXY )
{
	for (int vertIndex = 0; vertIndex < verts.size(); vertIndex++)
	{
		TransformPositionXY3D( verts[vertIndex].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY );
	}
}

void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform )
{
	for (Vertex_PCU& vert : verts)
	{
		vert.m_position = transform.TransformPosition3D( vert.m_position );
	}
}

void TransformVertexArray3D( std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform )
{
	for (Vertex_PCUTBN& vert : verts)
	{
		vert.m_position = transform.TransformPosition3D( vert.m_position );
	}
}

void CalculateTangantSpaceBasisVectors( std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, bool computeNormals, bool computeTangents )
{
	for (int i = 0; i < indexes.size() / 3; i++)
	{
		Vec3 E0 = vertexes[indexes[i * 3 + 1]].m_position - vertexes[indexes[i * 3 + 0]].m_position;
		Vec3 E1 = vertexes[indexes[i * 3 + 2]].m_position - vertexes[indexes[i * 3 + 0]].m_position;
		if (computeNormals)
		{
			vertexes[indexes[i * 3 + 0]].m_normal = CrossProduct3D( E0, E1 );
			vertexes[indexes[i * 3 + 1]].m_normal = vertexes[indexes[i * 3 + 0]].m_normal;
			vertexes[indexes[i * 3 + 2]].m_normal = vertexes[indexes[i * 3 + 0]].m_normal;
		}
		if (computeTangents)
		{
			float deltaU0 = vertexes[indexes[i * 3 + 1]].m_uvTexCoords.x - vertexes[indexes[i * 3 + 0]].m_uvTexCoords.x;
			float deltaU1 = vertexes[indexes[i * 3 + 2]].m_uvTexCoords.x - vertexes[indexes[i * 3 + 0]].m_uvTexCoords.x;
			float deltaV0 = vertexes[indexes[i * 3 + 1]].m_uvTexCoords.y - vertexes[indexes[i * 3 + 0]].m_uvTexCoords.y;
			float deltaV1 = vertexes[indexes[i * 3 + 2]].m_uvTexCoords.y - vertexes[indexes[i * 3 + 0]].m_uvTexCoords.y;
			float r = 1.f / (deltaU0 * deltaV1 - deltaU1 * deltaV0);
			Vec3 T = (r * (deltaV1 * E0 - deltaV0 * E1)).GetNormalized();
			Vec3 B = (r * (deltaU0 * E1 - deltaU1 * E0)).GetNormalized();
			vertexes[indexes[i * 3 + 0]].m_tangent = T;
			vertexes[indexes[i * 3 + 1]].m_tangent = T;
			vertexes[indexes[i * 3 + 2]].m_tangent = T;
			vertexes[indexes[i * 3 + 0]].m_bitangent = B;
			vertexes[indexes[i * 3 + 1]].m_bitangent = B;
			vertexes[indexes[i * 3 + 2]].m_bitangent = B;
		}
	}
	for (int i = 0; i < vertexes.size(); i++)
	{
		Mat44 mat;
		mat.SetIJK3D( vertexes[i].m_tangent, vertexes[i].m_bitangent, vertexes[i].m_normal );
		mat.Orthonormalize_IFwd_JLeft_KUp();
		vertexes[i].m_tangent = mat.GetIBasis3D();
		vertexes[i].m_bitangent = mat.GetJBasis3D();
		vertexes[i].m_normal = mat.GetKBasis3D();
	}
}


Vertex_PCU GetTransformedVertex3D( Vertex_PCU vert, Mat44 const& transform )
{
	Vertex_PCU newVert = vert;
	newVert.m_position = transform.TransformPosition3D( vert.m_position );
	return newVert;
}

Vertex_PCU RotateVertexAboutPivot( Vertex_PCU vert, Vec3 const& pivot, Mat44 const& transform )
{
	Vec3 pivotToVert = vert.m_position - pivot;
	Vec3 pivotToNewVert = transform.TransformVectorQuantity3D( pivotToVert );
	Vertex_PCU newVert = vert;
	newVert.m_position = pivot + pivotToNewVert;
	return newVert;
}


Vec2 TransformPolarToCarteDegreeNormal( float rotationDegree )
{
	return Vec2(
		CosDegrees( rotationDegree ),
		SinDegrees( rotationDegree )
	);
}

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{

	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
	{
		if (verts[vertIndex].m_position.x == 0 && verts[vertIndex].m_position.y == 0)
			continue;

		TransformPositionXY3D( verts[vertIndex].m_position, iBasis, jBasis, translationXY );
	}
}

void TransformVertexArrayXY3D( std::vector<Vertex_PCU>& verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{
	for (int vertIndex = 0; vertIndex < verts.size(); vertIndex++)
	{
		if (verts[vertIndex].m_position.x == 0 && verts[vertIndex].m_position.y == 0)
			continue;

		TransformPositionXY3D( verts[vertIndex].m_position, iBasis, jBasis, translationXY );
	}
}

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color )
{
	Vec2 bottomLeft = bounds.m_mins;
	Vec2 upperRight = bounds.m_maxs;
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2(0, 0) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, bottomLeft.y ) ), color, Vec2(1, 0) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2(1, 1) ) );

	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2(1, 1) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, upperRight.y ) ), color, Vec2(0, 1) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2(0, 0) ) );
}

void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 uvAtMins, Vec2 uvAtMaxs )
{
	Vec2 bottomLeft = bounds.m_mins;
	Vec2 upperRight = bounds.m_maxs;
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, bottomLeft.y ) ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );

	verts.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, upperRight.y ) ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	verts.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );
}

void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& upperRight, Rgba8 color )
{
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, bottomLeft.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color ) );

	vertArray.push_back( Vertex_PCU( Vec3( Vec2( upperRight.x, upperRight.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, upperRight.y ) ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( Vec2( bottomLeft.x, bottomLeft.y ) ), color ) );
}

void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color )
{
	vertArray.push_back( Vertex_PCU( Vec3( bottomLeft ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( bottomRight ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( topRight ), color ) );

	vertArray.push_back( Vertex_PCU( Vec3( topRight ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( topLeft ), color ) );
	vertArray.push_back( Vertex_PCU( Vec3( bottomLeft ), color ) );
}

void AddVertsForRect2D( std::vector<Vertex_PCU>& vertArray, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color, Vec2 uvAtMins, Vec2 uvAtMaxs )
{
	vertArray.push_back( Vertex_PCU( Vec3( bottomLeft ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );
	vertArray.push_back( Vertex_PCU( Vec3( bottomRight ), color, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertArray.push_back( Vertex_PCU( Vec3( topRight ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );

	vertArray.push_back( Vertex_PCU( Vec3( topRight ), color, Vec2( uvAtMaxs.x, uvAtMaxs.y ) ) );
	vertArray.push_back( Vertex_PCU( Vec3( topLeft ), color, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertArray.push_back( Vertex_PCU( Vec3( bottomLeft ), color, Vec2( uvAtMins.x, uvAtMins.y ) ) );
}

void AddVertsForHollowRect2D( std::vector<Vertex_PCU>& vertArray, float thickness, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color )
{
	AddVertsForRect2D( vertArray, bottomLeft + Vec2( 0.f, thickness ), bottomRight + Vec2( 0.f, thickness ), bottomRight, bottomLeft, color );
	AddVertsForRect2D( vertArray, bottomRight + Vec2( 0.f, thickness ), bottomRight + Vec2( thickness, thickness ), topRight + Vec2( thickness, -thickness ), topRight + Vec2( 0.f, -thickness ), color );
	AddVertsForRect2D( vertArray, topRight, topLeft, topLeft + Vec2( 0.f, -thickness ), topRight + Vec2( 0.f, -thickness ), color );
	AddVertsForRect2D( vertArray, bottomLeft + Vec2( -thickness, thickness ), bottomLeft + Vec2( 0.f, thickness ), topLeft + Vec2( 0.f, -thickness ), topLeft + Vec2( -thickness, -thickness ), color );
}

void AddVertsForRing2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, float const& thick, Rgba8 color )
{
	constexpr int numSlices = 24;
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);

	float innerRadius = radius - thick * 0.5f;
	float outerRadius = radius + thick * 0.5f;

	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vec2 bottomLeft = center + Vec2(
			outerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex) ),
			outerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex) )
		);
		Vec2 bottomRight = center + Vec2(
			outerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) ),
			outerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) )
		);
		Vec2 topRight = center + Vec2(
			innerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) ),
			innerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex + 1) )
		);
		Vec2 topLeft = center + Vec2(
			innerRadius * CosDegrees( SLICE_DEGREES * static_cast<float>(triIndex) ),
			innerRadius * SinDegrees( SLICE_DEGREES * static_cast<float>(triIndex) )
		);

		Vertex_PCU buildQuad[6] =
		{
			Vertex_PCU( Vec3( bottomLeft ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( bottomRight ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( topRight ), color, Vec2( 0.f, 0.f ) ),

			Vertex_PCU( Vec3( topRight ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( topLeft ), color, Vec2( 0.f, 0.f ) ),
			Vertex_PCU( Vec3( bottomLeft ), color, Vec2( 0.f, 0.f ) ),
		};
		AddVertsForRect2D( verts, bottomLeft, bottomRight, topRight, topLeft, color );
	}
}

void AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color )
{
	constexpr int numSlices = 24;
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);

	std::vector<Vertex_PCU> vertexRing;
	vertexRing.reserve( numSlices );

	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		vertexRing.push_back( Vertex_PCU(
			Vec3(
				radius * CosDegrees( vertIndex * SLICE_DEGREES ),
				radius * SinDegrees( vertIndex * SLICE_DEGREES ),
				0.f
			) + Vec3( center ),
			color,
			Vec2( 0.f, 0.f )
		) );
	}

	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		verts.push_back( vertexRing[triIndex] );
		verts.push_back( vertexRing[(triIndex + 1) % 24] );
		verts.push_back( Vertex_PCU( Vec3( center ), color, Vec2( 0.f, 0.f ) ) );
	}
}

void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color )
{
	constexpr int numSlices = 24;
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);
	
	Vec2 boneVector = boneEnd - boneStart;
	float boneDegree = Atan2Degrees( boneVector.y, boneVector.x );

	std::vector<Vertex_PCU> vertexRing;
	vertexRing.reserve( 72 );
	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		vertexRing.push_back( Vertex_PCU(
			Vec3(
				radius * CosDegrees( vertIndex * SLICE_DEGREES + boneDegree - 90.f ),
				radius * SinDegrees( vertIndex * SLICE_DEGREES + boneDegree - 90.f ),
				0.f
			) + ((vertIndex < (numSlices >> 1)) ? Vec3( boneEnd ) : Vec3( boneStart )),
			color,
			Vec2( 0.f, 0.f )
		) );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		verts.push_back( vertexRing[triIndex] );
		verts.push_back( vertexRing[(triIndex + 1) % 24] );
		verts.push_back( Vertex_PCU( Vec3( (triIndex < (numSlices >> 1)) ? Vec3( boneEnd ) : Vec3( boneStart ) ), color, Vec2( 0.f, 0.f ) ) );
	}
	Vec2 center = boneStart + boneVector * 0.5f;
	Vec2 iBasisNormal = Vec2( boneVector.y, -boneVector.x ).GetNormalized();
	Vec2 halfDimensions = Vec2( radius, boneVector.GetLength() * 0.5f );
	AddVertsForOBB2D( verts, OBB2( center, iBasisNormal, halfDimensions ), color );
}

void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color )
{
	Vec2 lineVector = end - start;
	Vec2 center = start + lineVector * 0.5f;
	Vec2 iBasisNormal = Vec2( lineVector.y, -lineVector.x ).GetNormalized();
	Vec2 halfDimensions = Vec2( thickness, thickness + lineVector.GetLength() * 0.5f );
	AddVertsForOBB2D( verts, OBB2( center, iBasisNormal, halfDimensions ), color );
}

void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );
	Vec2 bottomLeft = box.m_center + box.m_halfDimensions.x * iBasis + box.m_halfDimensions.y * jBasis;
	Vec2 bottomRight = box.m_center + box.m_halfDimensions.x * iBasis - box.m_halfDimensions.y * jBasis;
	Vec2 topRight = box.m_center - box.m_halfDimensions.x * iBasis - box.m_halfDimensions.y * jBasis;
	Vec2 topLeft = box.m_center - box.m_halfDimensions.x * iBasis + box.m_halfDimensions.y * jBasis;

	AddVertsForRect2D( verts, bottomLeft, bottomRight, topRight, topLeft, color );
}

void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, Vec2 uvAtMins, Vec2 uvAtMaxs )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );
	Vec2 bottomLeft = box.m_center + box.m_halfDimensions.x * iBasis + box.m_halfDimensions.y * jBasis;
	Vec2 bottomRight = box.m_center + box.m_halfDimensions.x * iBasis - box.m_halfDimensions.y * jBasis;
	Vec2 topRight = box.m_center - box.m_halfDimensions.x * iBasis - box.m_halfDimensions.y * jBasis;
	Vec2 topLeft = box.m_center - box.m_halfDimensions.x * iBasis + box.m_halfDimensions.y * jBasis;

	AddVertsForRect2D( verts, bottomLeft, bottomRight, topRight, topLeft, color, uvAtMins, uvAtMaxs );
}

void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color  )
{
	AddVertsForLineSegment2D( verts, tipPos, tailPos, lineThickness, color );
	//drawLine( tipPos, tailPos, lineThickness * 0.5, color );
	Vec2 arrowNormal = (tailPos - tipPos).GetNormalized();
	Vec2 leftArm = arrowNormal.GetRotatedDegrees( 45 );
	Vec2 rightArm = arrowNormal.GetRotatedDegrees( -45 );
	AddVertsForLineSegment2D( verts, tipPos, tipPos + leftArm * arrowSize, lineThickness, color );
	AddVertsForLineSegment2D( verts, tipPos, tipPos + rightArm * arrowSize, lineThickness, color );
}

void AddVertsForQuadOutline2D( std::vector<Vertex_PCU>& verts, AABB2 const& box, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/ )
{
	Vec2 adjustment( thickness * 0.5f, thickness * 0.5f );

	Vec2 bottomLeft( box.m_mins.x, box.m_mins.y );
	Vec2 bottomRight( box.m_maxs.x, box.m_mins.y );
	Vec2 topRight( box.m_maxs.x, box.m_maxs.y );
	Vec2 topLeft( box.m_mins.x, box.m_maxs.y );

	// Bottom edge
	AddVertsForAABB2D( verts,
		AABB2(
			Vec2( bottomLeft - adjustment ),
			Vec2( bottomRight + adjustment )
		),
		color );
	// Right edge
	AddVertsForAABB2D( verts,
		AABB2(
			Vec2( bottomRight - adjustment ),
			Vec2( topRight + adjustment )
		),
		color );
	// Top edge
	AddVertsForAABB2D( verts,
		AABB2(
			Vec2( topLeft - adjustment ),
			Vec2( topRight + adjustment )
		),
		color );
	// Left edge
	AddVertsForAABB2D( verts,
		AABB2(
			Vec2( bottomLeft - adjustment ),
			Vec2( topLeft + adjustment )
		),
		color );
}

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	verts.push_back( Vertex_PCU( bottomLeft, color, Vec2( UVs.m_mins ) ) );
	verts.push_back( Vertex_PCU( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ) ) );
	verts.push_back( Vertex_PCU( topRight, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( topRight, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ) ) );
	verts.push_back( Vertex_PCU( bottomLeft, color, Vec2( UVs.m_mins ) ) );
}

void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	int index = (int)verts.size();

	verts.push_back( Vertex_PCU( bottomLeft, color, Vec2( UVs.m_mins ) ) );
	verts.push_back( Vertex_PCU( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ) ) );
	verts.push_back( Vertex_PCU( topRight, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ) ) );

	indexes.push_back( index + 0 );
	indexes.push_back( index + 1 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 3 );
	indexes.push_back( index + 0 );
}

void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 normal = CrossProduct3D( bottomRight - bottomLeft, topLeft - bottomLeft ).GetNormalized();

	int index = (int)verts.size();

	verts.push_back( Vertex_PCUTBN( bottomLeft, color, Vec2( UVs.m_mins ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( topRight, color, Vec2( UVs.m_maxs ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, normal ) );

	indexes.push_back( index + 0 );
	indexes.push_back( index + 1 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 3 );
	indexes.push_back( index + 0 );
}

void AddVertsForQuadOutline3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE */ )
{
	Vec3 bottomDirection = (bottomRight - bottomLeft).GetNormalized();
	Vec3 rightDirection = (topRight - bottomRight).GetNormalized();
	Vec3 topDirection = (topLeft - topRight).GetNormalized();
	Vec3 leftDirection = (bottomLeft - topLeft).GetNormalized();

	Vec3 bottomPerp = Vec3( -bottomDirection.y, bottomDirection.x, bottomDirection.z ) * thickness * 0.5f;
	Vec3 rightPerp = Vec3( -rightDirection.y, rightDirection.x, rightDirection.z ) * thickness * 0.5f;
	Vec3 topPerp = Vec3( -topDirection.y, topDirection.x, topDirection.z ) * thickness * 0.5f;
	Vec3 leftPerp = Vec3( -leftDirection.y, leftDirection.x, leftDirection.z ) * thickness * 0.5f;

	// Bottom edge
	AddVertsForQuad3D( verts, indexes,
		bottomLeft - bottomPerp,
		bottomRight - bottomPerp,
		bottomRight + bottomPerp,
		bottomLeft + bottomPerp,
		color, UVs );
	// Right edge
	AddVertsForQuad3D( verts, indexes,
		bottomRight - rightPerp,
		topRight - rightPerp,
		topRight + rightPerp,
		bottomRight + rightPerp,
		color, UVs );
	// Top edge
	AddVertsForQuad3D( verts, indexes,
		topRight - topPerp,
		topLeft - topPerp,
		topLeft + topPerp,
		topRight + topPerp,
		color, UVs );
	// Left edge
	AddVertsForQuad3D( verts, indexes,
		topLeft - leftPerp,
		bottomLeft - leftPerp,
		bottomLeft + leftPerp,
		topLeft + leftPerp,
		color, UVs );
}

void AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 bottomLeftNormal;
	Vec3 bottomMidNormal;
	Vec3 bottomRightNormal;
	Vec3 topLeftNormal;
	Vec3 topMidNormal;
	Vec3 topRightNormal;

	Vec3 bottomMid = (bottomLeft + bottomRight) * 0.5f;
	Vec3 topMid = (topLeft + topRight) * 0.5f;

	bottomLeftNormal = (bottomLeft - bottomRight).GetNormalized();
	bottomRightNormal = -bottomLeftNormal;
	topLeftNormal = (topLeft - topRight).GetNormalized();
	topRightNormal = -topLeftNormal;
	topMidNormal = CrossProduct3D( topLeft - topMid, bottomMid - topMid ).GetNormalized();
	bottomMidNormal = CrossProduct3D( topMid - bottomMid, bottomLeft - bottomMid ).GetNormalized();

	int index = (int)verts.size();

	verts.push_back( Vertex_PCUTBN( bottomLeft, color, Vec2( UVs.m_mins ), Vec3::ZERO, Vec3::ZERO, bottomLeftNormal ) );
	verts.push_back( Vertex_PCUTBN( bottomMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, bottomMidNormal ) );
	verts.push_back( Vertex_PCUTBN( topMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, topMidNormal ) );
	verts.push_back( Vertex_PCUTBN( topLeft, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, topLeftNormal ) );

	indexes.push_back( index + 0 );
	indexes.push_back( index + 1 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 2 );
	indexes.push_back( index + 3 );
	indexes.push_back( index + 0 );

	verts.push_back( Vertex_PCUTBN( bottomMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, bottomMidNormal ) );
	verts.push_back( Vertex_PCUTBN( bottomRight, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ), Vec3::ZERO, Vec3::ZERO, bottomRightNormal ) );
	verts.push_back( Vertex_PCUTBN( topRight, color, Vec2( UVs.m_maxs ), Vec3::ZERO, Vec3::ZERO, topRightNormal ) );
	verts.push_back( Vertex_PCUTBN( topMid, color, Vec2( (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f, UVs.m_maxs.y ), Vec3::ZERO, Vec3::ZERO, topMidNormal ) );

	indexes.push_back( index + 4 );
	indexes.push_back( index + 5 );
	indexes.push_back( index + 6 );
	indexes.push_back( index + 6 );
	indexes.push_back( index + 7 );
	indexes.push_back( index + 4 );
}

void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color/* = Rgba8::WHITE*/, AABB2 const& UVs/* = AABB2::ZERO_TO_ONE*/ )
{
	Vec3 point1( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 point2( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 point3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 point4( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 point5( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 point6( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 point7( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 point8( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	AddVertsForQuad3D( verts, point3, point4, point1, point2, color, UVs );//z
	AddVertsForQuad3D( verts, point6, point5, point8, point7, color, UVs );//-z
	AddVertsForQuad3D( verts, point5, point6, point2, point1, color, UVs );//y
	AddVertsForQuad3D( verts, point7, point8, point4, point3, color, UVs );//-y
	AddVertsForQuad3D( verts, point8, point5, point1, point4, color, UVs );//x
	AddVertsForQuad3D( verts, point6, point7, point3, point2, color, UVs );//-x
}

void AddVertsForAABB3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color/* = Rgba8::WHITE*/, AABB2 const& UVs/* = AABB2::ZERO_TO_ONE*/ )
{
	Vec3 point1( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 point2( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );
	Vec3 point3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 point4( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );
	Vec3 point5( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 point6( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );
	Vec3 point7( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );
	Vec3 point8( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );
	AddVertsForQuad3D( verts, indexes, point3, point4, point1, point2, color, UVs );//z
	AddVertsForQuad3D( verts, indexes, point6, point5, point8, point7, color, UVs );//-z
	AddVertsForQuad3D( verts, indexes, point5, point6, point2, point1, color, UVs );//y
	AddVertsForQuad3D( verts, indexes, point7, point8, point4, point3, color, UVs );//-y
	AddVertsForQuad3D( verts, indexes, point8, point5, point1, point4, color, UVs );//x
	AddVertsForQuad3D( verts, indexes, point6, point7, point3, point2, color, UVs );//-x
}

void AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3 const& bounds, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 point1( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point2( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point3( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point4( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y + bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point5( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point6( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x + bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point7( bounds.m_center - bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	Vec3 point8( bounds.m_center + bounds.m_iBasisNormal * bounds.m_halfDimensions.x - bounds.m_jBasisNormal * bounds.m_halfDimensions.y - bounds.m_kBasisNormal * bounds.m_halfDimensions.z );
	AddVertsForQuad3D( verts, point3, point4, point1, point2, color, UVs );
	AddVertsForQuad3D( verts, point6, point5, point8, point7, color, UVs );
	AddVertsForQuad3D( verts, point5, point6, point2, point1, color, UVs );
	AddVertsForQuad3D( verts, point7, point8, point4, point3, color, UVs );
	AddVertsForQuad3D( verts, point8, point5, point1, point4, color, UVs );
	AddVertsForQuad3D( verts, point6, point7, point3, point2, color, UVs );
}

void AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color/* = Rgba8::WHITE*/, AABB2 const& UVs/* = AABB2::ZERO_TO_ONE*/, int numLongtitude/* = 32*/, int numLatitude/* = 16*/ )
{
	float latitudeDegrees = 180.f / numLatitude;
	float longtitudeDegrees = 360.f / numLongtitude;
	float uvGridHeight = 1.f / (float)numLatitude;
	float uvGridWidth = 1.f / (float)numLongtitude;
	for (int i = 0; i < numLatitude; i++)
	{
		for (int j = 0; j < numLongtitude; j++)
		{
			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * i, longtitudeDegrees * j, radius );
			Vec3 buttomRight = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * i, longtitudeDegrees * (j + 1), radius );
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius );
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * (i + 1), longtitudeDegrees * j, radius );
			Vec2 uvAtMin = Vec2( uvGridWidth * j + UVs.m_mins.x, UVs.m_maxs.y - uvGridHeight * (i + 1) );
			Vec2 uvAtMax = Vec2( uvGridWidth * (j + 1.f) + UVs.m_mins.x, UVs.m_maxs.y - uvGridHeight * i );
			AddVertsForQuad3D( verts, bottomLeft, buttomRight, topRight, topLeft, color, AABB2( uvAtMin, uvAtMax ) );
		}
	}
}

void AddVertsForSphere3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& color/* = Rgba8::WHITE*/, AABB2 const& UVs/* = AABB2::ZERO_TO_ONE*/, int numLongtitude/* = 32*/, int numLatitude/* = 16*/ )
{
	float latitudeDegrees = 180.f / numLatitude;
	float longtitudeDegrees = 360.f / numLongtitude;
	float uvGridHeight = 1.f / (float)numLatitude;
	float uvGridWidth = 1.f / (float)numLongtitude;
	for (int i = 0; i < numLatitude; i++)
	{
		for (int j = 0; j < numLongtitude; j++)
		{
			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * i, longtitudeDegrees * j, radius );
			Vec3 buttomRight = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * i, longtitudeDegrees * (j + 1), radius );
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius );
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees( 90.f - latitudeDegrees * (i + 1), longtitudeDegrees * j, radius );
			Vec2 uvAtMin = Vec2( uvGridWidth * j + UVs.m_mins.x, UVs.m_maxs.y - uvGridHeight * (i + 1) );
			Vec2 uvAtMax = Vec2( uvGridWidth * (j + 1.f) + UVs.m_mins.x, UVs.m_maxs.y - uvGridHeight * i );
			AddVertsForQuad3D( verts, indexes, bottomLeft, buttomRight, topRight, topLeft, color, AABB2( uvAtMin, uvAtMax ) );
		}
	}
}

void AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices )
{
	// Calculate the cylinder's basis vectors
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis, jBasis;

	// Create an orthonormal basis
	if (kBasis == Vec3( 0.f, 0.f, 1.f ) || kBasis == Vec3( 0.f, 0.f, -1.f ))
	{
		iBasis = Vec3( 1.f, 0.f, 0.f );
		jBasis = Vec3( 0.f, 1.f, 0.f );
	}
	else
	{
		jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), kBasis ).GetNormalized();
		iBasis = CrossProduct3D( jBasis, kBasis );
	}

	// Transformation matrix to align local cylinder vertices with world space
	Mat44 transform( iBasis, jBasis, kBasis, start );

	// Precompute UV and geometry properties
	float sliceDegrees = 360.f / static_cast<float>(numSlices);
	float height = (end - start).GetLength();
	Vec2 UVSize = UVs.GetDimensions();
	Vec2 UVCenter = UVs.GetCenter();
	float pieceLength = 1.f / static_cast<float>(numSlices);

	// Generate vertex rings for the cylinder
	std::vector<Vec3> bottomRing, topRing;
	bottomRing.reserve( numSlices );
	topRing.reserve( numSlices );

	for (int i = 0; i < numSlices; ++i)
	{
		float angle = i * sliceDegrees;
		float x = radius * CosDegrees( angle );
		float y = radius * SinDegrees( angle );
		Vec3 localPos( x, y, 0.f );
		Vec3 bottomVertex = transform.TransformPosition3D( localPos );
		Vec3 topVertex = transform.TransformPosition3D( localPos + Vec3( 0.f, 0.f, height ) );

		bottomRing.push_back( bottomVertex );
		topRing.push_back( topVertex );
	}

	// Add top and bottom faces
	Vec3 centerBottom = transform.TransformPosition3D( Vec3( 0.f, 0.f, 0.f ) );
	Vec3 centerTop = transform.TransformPosition3D( Vec3( 0.f, 0.f, height ) );
	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % numSlices;

		// Bottom face (triangle fan)
		verts.emplace_back( bottomRing[next], color, UVs.m_mins );
		verts.emplace_back( bottomRing[i], color, UVs.m_mins );
		verts.emplace_back( centerBottom, color, UVCenter );

		// Top face (triangle fan)
		verts.emplace_back( topRing[i], color, UVs.m_mins );
		verts.emplace_back( topRing[next], color, UVs.m_mins );
		verts.emplace_back( centerTop, color, UVCenter );
	}

	// Add side faces (quads)
	for (int i = 0; i < numSlices; ++i)
	{
		int next = (i + 1) % numSlices;

		Vec3 bottomLeft = bottomRing[i];
		Vec3 bottomRight = bottomRing[next];
		Vec3 topRight = topRing[next];
		Vec3 topLeft = topRing[i];

		Vec2 UVBottomLeft( UVs.m_mins.x + i * pieceLength * UVSize.x, UVs.m_mins.y );
		Vec2 UVBottomRight( UVs.m_mins.x + next * pieceLength * UVSize.x, UVs.m_mins.y );
		Vec2 UVTopLeft( UVBottomLeft.x, UVs.m_maxs.y );
		Vec2 UVTopRight( UVBottomRight.x, UVs.m_maxs.y );

		// First triangle of quad
		verts.emplace_back( bottomLeft, color, UVBottomLeft );
		verts.emplace_back( bottomRight, color, UVBottomRight );
		verts.emplace_back( topRight, color, UVTopRight );

		// Second triangle of quad
		verts.emplace_back( bottomLeft, color, UVBottomLeft );
		verts.emplace_back( topRight, color, UVTopRight );
		verts.emplace_back( topLeft, color, UVTopLeft );
	}
}

void AddVertsForCapsule3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	Vec3 capsuleDirection = (end - start).GetNormalized();

	AddVertsForCylinder3D( verts, start, end, radius, color, UVs, numSlices );

	AddVertsForSphere3D( verts, start, radius, color, UVs, numSlices, numSlices / 2 );

	AddVertsForSphere3D( verts, end, radius, color, UVs, numSlices, numSlices / 2 );
}

void AddVertsForCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	Vec3 kBasis = ((end - start) * 100.f).GetNormalized();
	Vec3 iBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 jBasis = Vec3( 0.f, 1.f, 0.f );
	if (kBasis != Vec3( 0.f, 0.f, 1.f ) && kBasis != Vec3( 0.f, 0.f, -1.f ))
	{
		jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), kBasis );
		iBasis = CrossProduct3D( jBasis, kBasis );
	}
	float SLICE_DEGREES = 360.f / static_cast<float>(numSlices);
	Mat44 transform( iBasis, jBasis, kBasis, Vec3::ZERO );
	std::vector<Vertex_PCU> vertexRing;
	vertexRing.reserve( numSlices );
	float height = (end - start).GetLength();
	Vec2 UVsSize = UVs.GetDimensions();
	for (int vertIndex = 0; vertIndex < numSlices; vertIndex++)
	{
		float CD = CosDegrees( vertIndex * SLICE_DEGREES );
		float SD = SinDegrees( vertIndex * SLICE_DEGREES );
		vertexRing.push_back( Vertex_PCU(
			Vec3(
				radius * CD,
				radius * SD,
				0.f
			) + start,
			color,
			UVs.m_mins + Vec2( (CD * 0.5f + 0.5f) * UVsSize.x, (SD * 0.5f + 0.5f) * UVsSize.y )
		) );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vertex_PCU pointA = RotateVertexAboutPivot( vertexRing[(triIndex + 1) % numSlices], start, transform );
		Vertex_PCU pointB = RotateVertexAboutPivot( vertexRing[triIndex], start, transform );
		Vertex_PCU pointC = RotateVertexAboutPivot( Vertex_PCU( start, color, UVs.m_mins + Vec2( 0.5f * UVsSize.x, 0.5f * UVsSize.y ) ), start, transform );
		verts.push_back( pointA );
		verts.push_back( pointB );
		verts.push_back( pointC );
	}
	for (int triIndex = 0; triIndex < numSlices; triIndex++)
	{
		Vertex_PCU bottomLeft = RotateVertexAboutPivot( vertexRing[triIndex], start, transform );
		Vertex_PCU buttomRight = RotateVertexAboutPivot( vertexRing[(triIndex + 1) % numSlices], start, transform );
		Vertex_PCU topVert = RotateVertexAboutPivot( Vertex_PCU( start + Vec3( 0.f, 0.f, height ), color, Vec2( 0.5f * UVsSize.x, 0.5f * UVsSize.y ) ), start, transform );
		verts.push_back( bottomLeft );
		verts.push_back( buttomRight );
		verts.push_back( topVert );
	}
}

void AddVertsForHexigon3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& color, AABB2 const& UVs )
{
	UNUSED( UVs );
	float const angleDegree = 60.f;
	unsigned int const vertSize = (unsigned int)verts.size();
	verts.push_back( Vertex_PCU{ center, color } );
	for (int i = 0; i < 6; i++)
	{
		float angle = i * angleDegree;
		Vec3 pos( center.x + radius * CosDegrees( angle ), center.y + radius * SinDegrees( angle ), center.z );
		verts.push_back( Vertex_PCU{ pos, color } );
	}
	for (int i = 0; i < 6; i++)
	{
		unsigned int next = vertSize + 1 + (i + 1) % 6;
		indexes.push_back( vertSize );
		indexes.push_back( vertSize + 1 + i );
		indexes.push_back( next );
	}
}

void AddVertsForHexigonOutline3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float thickness, Rgba8 const& color, AABB2 const& UVs )
{
	UNUSED( UVs );
	float const angleDegree = 60.f;
	unsigned int const vertSize = (unsigned int)verts.size();
	float const halfThickness = 0.5f * thickness;
	float quartThickness = 0.25f * thickness;

	for (int i = 0; i < 6; i++)
	{
		float angle1 = i * angleDegree;
		float angle2 = angle1 + angleDegree;
		Vec3 pos1( center.x + radius * CosDegrees( angle1 ), center.y + radius * SinDegrees( angle1 ), center.z );
		Vec3 pos2( center.x + radius * CosDegrees( angle2 ), center.y + radius * SinDegrees( angle2 ), center.z );

		Vec3 direction = (pos2 - pos1).GetNormalized();
		Vec3 perpendicular = Vec3(-direction.y, direction.x, direction.z);
		Vec3 extension1 = pos1 - direction * quartThickness;
		Vec3 extension2 = pos2 + direction * quartThickness;

		Vec3 vert1 = extension1 + perpendicular * halfThickness;
		Vec3 vert2 = extension1 - perpendicular * halfThickness;
		Vec3 vert3 = extension2 + perpendicular * halfThickness;
		Vec3 vert4 = extension2 - perpendicular * halfThickness;

		verts.push_back( Vertex_PCU{ vert1, color } );
		verts.push_back( Vertex_PCU{ vert2, color } );
		verts.push_back( Vertex_PCU{ vert3, color } );
		verts.push_back( Vertex_PCU{ vert4, color } );

		unsigned int idx = vertSize + i * 4;
		indexes.push_back( idx );
		indexes.push_back( idx + 1 );
		indexes.push_back( idx + 2 );
		indexes.push_back( idx + 1 );
		indexes.push_back( idx + 3 );
		indexes.push_back( idx + 2 );
	}
}

AABB2 GetVertexBounds2D( std::vector<Vertex_PCU>& verts )
{
	if (verts.size() == 0)
		return AABB2();

	float minX = verts[0].m_position.x;
	float maxX = verts[0].m_position.x;
	float minY = verts[0].m_position.y;
	float maxY = verts[0].m_position.y;
	for (Vertex_PCU vert : verts)
	{
		minX = (minX > vert.m_position.x) ? vert.m_position.x : minX;
		maxX = (maxX < vert.m_position.x) ? vert.m_position.x : maxX;
		minY = (minY > vert.m_position.y) ? vert.m_position.y : minY;
		maxY = (maxY < vert.m_position.y) ? vert.m_position.y : maxY;
	}
	return AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) );
}
