#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Mat44.hpp"

OBB3::OBB3( OBB3 const& copyForm )
{
	m_center = copyForm.m_center;
	m_iBasisNormal = copyForm.m_iBasisNormal;
	m_jBasisNormal = copyForm.m_jBasisNormal;
	m_kBasisNormal = copyForm.m_kBasisNormal;
	m_halfDimensions = copyForm.m_halfDimensions;
}

OBB3::OBB3( Vec3 const& center, Vec3 const& iBasisNormal, Vec3 const& jBasisNormal, Vec3 const& kBasisNormal, Vec3 const& halfDimensions )
{
	m_center = center;
	m_iBasisNormal = iBasisNormal;
	m_jBasisNormal = jBasisNormal;
	m_kBasisNormal = kBasisNormal;
	m_halfDimensions = halfDimensions;
}

void OBB3::SetCenter( Vec3 newCenter )
{
	m_center = newCenter;
}

Vec3 OBB3::WorldPosToLocalPos( Vec3 worldPos ) const
{
	Mat44 transform = Mat44( m_iBasisNormal, m_jBasisNormal, m_kBasisNormal, m_center );
	Mat44 inverseTransform = transform.GetOrthonormalInverse();
	return inverseTransform.TransformPosition3D( worldPos );
}

Vec3 OBB3::LocalPosToWorldPos( Vec3 localPos ) const
{
	Mat44 transform = Mat44( m_iBasisNormal, m_jBasisNormal, m_kBasisNormal, m_center );
	return transform.TransformPosition3D( localPos );
}

Vec3 OBB3::WorldVecToLocalVec( Vec3 worldVec ) const
{
	Mat44 transform = Mat44( m_iBasisNormal, m_jBasisNormal, m_kBasisNormal, Vec3::ZERO );
	Mat44 inverseTransform = transform.GetOrthonormalInverse();
	return inverseTransform.TransformVectorQuantity3D( worldVec );
}

Vec3 OBB3::LocalVecToWorldVec( Vec3 localVec ) const
{
	Mat44 transform = Mat44( m_iBasisNormal, m_jBasisNormal, m_kBasisNormal, Vec3::ZERO );
	return transform.TransformVectorQuantity3D( localVec );
}

AABB3 OBB3::GetLocalAABB3() const
{
	return AABB3( -m_halfDimensions.x, -m_halfDimensions.y, -m_halfDimensions.z, m_halfDimensions.x, m_halfDimensions.y, m_halfDimensions.z );
}

OBB3 const OBB3::GetMoved( Vec3 const& delta ) const
{
	return OBB3( m_center + delta, m_iBasisNormal, m_jBasisNormal, m_kBasisNormal, m_halfDimensions );
}
