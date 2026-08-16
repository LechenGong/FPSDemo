#include "Engine/General/ShapeComponents/CubeComponent.hpp"
#include "Engine/Math/MathUtils.hpp"


CubeComponent::CubeComponent( float length, float width, float height, bool collisionEnabled, CollisionChannel channel,
    Vec3 position, Quat orientation, Vec3 scale )
    : ShapeComponent( position, orientation, scale, collisionEnabled, channel )
    , m_length( length )
    , m_width( width )
    , m_height( height )
{
	UpdateBoundingBox();
}

void CubeComponent::SetOrientation( Quat const& orientation )
{
	ShapeComponent::SetLocalOrientation( orientation );
	UpdateBoundingBox();
}

float CubeComponent::GetScaledLength() const
{
    return m_length * GetLocalScale().x;
}

float CubeComponent::GetScaledWidth() const
{
    return m_width * GetLocalScale().y;
}

float CubeComponent::GetScaledHeight() const
{
    return m_height * GetLocalScale().z;
}

float CubeComponent::GetUnscaledLength() const
{
    return m_length;
}

float CubeComponent::GetUnscaledWidth() const
{
    return m_width;
}

float CubeComponent::GetUnscaledHeight() const
{
    return m_height;
}

bool CubeComponent::IsPointInside( Vec3 const& point ) const
{
	return IsPointInsideOBB3D( point, CalculateBoundsOBB3D() );
}

Vec3 CubeComponent::GetFunctionalCenter() const
{
	return GetWorldPosition();
}

CollisionShape CubeComponent::GetCollisionShape() const
{
	return CollisionShape::CUBE;
}

// CubeComponent CubeComponent::CreateCubeComponent( CollisionInfo info, Character* character, CollisionChannel channel )
// {
// 	return CubeComponent();
// }

CubeComponent CubeComponent::CreateCubeComponent( Vec3 center, Quat orientation, Vec3 scale, float length, float width, float height, CollisionChannel channel )
{
	return CubeComponent( length, width, height, true, channel, center, orientation, scale );
}

float CubeComponent::GetBoundingSphereRadius() const
{
	return MAX( MAX( m_height, m_length ), m_width ) * 0.5f;
}

AABB3 CubeComponent::CalculateBoundsAABB3D() const
{
	Vec3 corners[8];
	OBB3 obb = CalculateBoundsOBB3D();
	Vec3 halfDims = obb.m_halfDimensions;

	corners[0] = obb.m_center + halfDims.x * obb.m_iBasisNormal + halfDims.y * obb.m_jBasisNormal + halfDims.z * obb.m_kBasisNormal;
	corners[1] = obb.m_center + halfDims.x * obb.m_iBasisNormal + halfDims.y * obb.m_jBasisNormal - halfDims.z * obb.m_kBasisNormal;
	corners[2] = obb.m_center + halfDims.x * obb.m_iBasisNormal - halfDims.y * obb.m_jBasisNormal + halfDims.z * obb.m_kBasisNormal;
	corners[3] = obb.m_center + halfDims.x * obb.m_iBasisNormal - halfDims.y * obb.m_jBasisNormal - halfDims.z * obb.m_kBasisNormal;
	corners[4] = obb.m_center - halfDims.x * obb.m_iBasisNormal + halfDims.y * obb.m_jBasisNormal + halfDims.z * obb.m_kBasisNormal;
	corners[5] = obb.m_center - halfDims.x * obb.m_iBasisNormal + halfDims.y * obb.m_jBasisNormal - halfDims.z * obb.m_kBasisNormal;
	corners[6] = obb.m_center - halfDims.x * obb.m_iBasisNormal - halfDims.y * obb.m_jBasisNormal + halfDims.z * obb.m_kBasisNormal;
	corners[7] = obb.m_center - halfDims.x * obb.m_iBasisNormal - halfDims.y * obb.m_jBasisNormal - halfDims.z * obb.m_kBasisNormal;

	Vec3 minp = corners[0];
	Vec3 maxp = corners[0];

	for (int i = 1; i < 8; ++i)
	{
		minp.x = std::min( minp.x, corners[i].x );
		minp.y = std::min( minp.y, corners[i].y );
		minp.z = std::min( minp.z, corners[i].z );

		maxp.x = std::max( maxp.x, corners[i].x );
		maxp.y = std::max( maxp.y, corners[i].y );
		maxp.z = std::max( maxp.z, corners[i].z );
	}

	AABB3 aabb;
	aabb.m_mins = minp;
	aabb.m_maxs = maxp;
	return aabb;
}

OBB3 CubeComponent::CalculateBoundsOBB3D() const
{
	Mat44 rotationMatrix = GetWorldOrientation().ToRotationMatrix();
	OBB3 obb;
	obb.m_center = GetWorldPosition();
	obb.m_halfDimensions = Vec3( GetScaledLength() * 0.5f, GetScaledWidth() * 0.5f, GetScaledHeight() * 0.5f );
	obb.m_iBasisNormal = rotationMatrix.GetIBasis3D().GetNormalized();
	obb.m_jBasisNormal = rotationMatrix.GetJBasis3D().GetNormalized();
	obb.m_kBasisNormal = rotationMatrix.GetKBasis3D().GetNormalized();
	return obb;
}
