#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/Math/MathUtils.hpp"

CapsuleComponent::CapsuleComponent( float radius, float height, bool collisionEnabled, CollisionChannel channel, Vec3 position, Quat orientation, Vec3 scale )
	: ShapeComponent( position, orientation, scale, collisionEnabled, channel )
	, m_radius( radius )
	, m_height( height )
{
	UpdateUpVector();
	UpdateBoundingBox();
}

// local orientation
void CapsuleComponent::SetOrientation( Quat const& orientation )
{
	ShapeComponent::SetLocalOrientation( orientation );
	UpdateUpVector();
	UpdateBoundingBox();
}

// local scale
float CapsuleComponent::GetScaledRadius() const
{
	return m_radius * MAX( GetLocalScale().x, GetLocalScale().y);
}

// local scale
float CapsuleComponent::GetScaledHeight() const
{
	return m_height * GetLocalScale().z;
}

// local scale
float CapsuleComponent::GetScaledCapsuleHeight() const
{
	return GetScaledHeight() + GetScaledRadius() * 2.f;
}

// local scale
float CapsuleComponent::GetUnscaledRadius() const
{
	return m_radius;
}

// local scale
float CapsuleComponent::GetUnscaledHeight() const
{
	return m_height;
}

Vec3 CapsuleComponent::GetWorldBoneStart() const
{
	Mat44 worldTransform = GetWorldTransform();
	return worldTransform.GetTranslation3D() - worldTransform.GetKBasis3D() * (GetScaledCapsuleHeight() * 0.5f - GetScaledRadius());
}

Vec3 CapsuleComponent::GetWorldBoneEnd() const
{
	Mat44 worldTransform = GetWorldTransform();
	return worldTransform.GetTranslation3D() + worldTransform.GetKBasis3D() * (GetScaledCapsuleHeight() * 0.5f - GetScaledRadius());
}

// local up
Vec3 CapsuleComponent::GetUpVector() const
{
	return m_upvectorNormal;
}

// world position
bool CapsuleComponent::IsPointInside( Vec3 const& point ) const
{
	Vec3 lineSegStart = GetWorldPosition() - m_upvectorNormal * GetScaledHeight();
	Vec3 lineSegEnd = GetWorldPosition() + m_upvectorNormal * GetScaledHeight();
	return IsPointInsideCapsule3D( point, lineSegStart, lineSegEnd, GetScaledRadius() );
}

void CapsuleComponent::UpdateUpVector()
{
	Quat rotation = GetLocalOrientation();

	Quat upVector( 0, 0, 1, 0 );

	Quat rotationConjugate = rotation.GetConjugated();

	Quat rotatedUp = rotation * upVector * rotationConjugate;

	m_upvectorNormal = Vec3( rotatedUp.x, rotatedUp.y, rotatedUp.z );
}

Vec3 CapsuleComponent::GetFunctionalCenter() const
{
	return (GetWorldBoneStart() + GetWorldBoneEnd()) * 0.5f;
}

CollisionShape CapsuleComponent::GetCollisionShape() const
{
	return CollisionShape::CAPSULE;
}

CapsuleComponent CapsuleComponent::CreateCapsuleComponent( CollisionInfo info, Character* character, CollisionChannel channel )
{
	float radius = info.data[3];
	if (info.secondaryJoint != "")
	{
		Vec3 start = character->GetJointGlobalTransformByName( info.primaryJoint ).GetTranslation3D() * 0.01f;
		Vec3 end = character->GetJointGlobalTransformByName( info.secondaryJoint ).GetTranslation3D() * 0.01f;

		Vec3 startToEnd = end - start;
		float distance = (end - start).GetLength();
		Vec3 direction = startToEnd / distance;

		Vec3 crossP = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), direction );
		float dotP = DotProduct3D( Vec3( 0.f, 0.f, 1.f ), direction );
		float angle = ACosDegrees( dotP );
		Quat orientation( crossP, angle );

		Vec3 pos = (start + end) * 0.5f;

		CapsuleComponent capsule( radius, distance, true, channel, pos, orientation );
		capsule.AttachToComponent( character->GetRootComponent() );
		return capsule;
	}
	else
	{
		float height = info.data[7];
		Mat44 transform = character->GetJointGlobalTransformByName( info.primaryJoint );
		Vec3 start = transform.GetTranslation3D() * 0.01f;
		Quat rotationOffset = Quat::FromEulerAngles( EulerAngles( info.data[4], info.data[5], info.data[6] ) );
		transform = transform * rotationOffset.ToRotationMatrix();
		Vec3 pos = start + transform.GetIBasis3D() * info.data[0] + transform.GetJBasis3D() * info.data[1] + transform.GetKBasis3D() * info.data[2];
		CapsuleComponent capsule( radius, height, true, channel, pos, Quat( transform ) );
		capsule.AttachToComponent( character->GetRootComponent() );
		return capsule;
	}
}

// world
AABB3 CapsuleComponent::CalculateBoundsAABB3D() const
{
	Vec3 posMin = GetWorldPosition() - Vec3( GetScaledRadius(), GetScaledRadius(), GetScaledHeight() * 0.5f + GetScaledRadius() );
	Vec3 posMax = GetWorldPosition() + Vec3( GetScaledRadius(), GetScaledRadius(), GetScaledHeight() * 0.5f + GetScaledRadius() );

	return AABB3(
		MIN( posMin.x, posMax.x ), MIN( posMin.y, posMax.y ), MIN( posMin.z, posMax.z ),
		MAX( posMin.x, posMax.x ), MAX( posMin.y, posMax.y ), MAX( posMin.z, posMax.z )
	);
}

//world
OBB3 CapsuleComponent::CalculateBoundsOBB3D() const
{
	OBB3 obb;
	obb.m_center = GetWorldPosition();
	Mat44 rotationMatrix = GetWorldOrientation().ToRotationMatrix();
	obb.m_iBasisNormal = rotationMatrix.GetIBasis3D();
	obb.m_jBasisNormal = rotationMatrix.GetJBasis3D();
	obb.m_kBasisNormal = rotationMatrix.GetKBasis3D();
	obb.m_halfDimensions = Vec3( GetScaledRadius(), GetScaledRadius(), GetScaledHeight() * 0.5f + GetScaledRadius() );
	return obb;
}