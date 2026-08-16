#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/General/Character.hpp"

SphereComponent::SphereComponent( float radius, bool collisionEnabled, CollisionChannel channel, Vec3 position, Quat orientation, Vec3 scale )
	: ShapeComponent( position, orientation, scale, collisionEnabled, channel )
	, m_radius( radius )
{
    UpdateBoundingBox();
}

void SphereComponent::SetOrientation( Quat const& orientation )
{
	ShapeComponent::SetLocalOrientation( orientation );
	UpdateBoundingBox();
}

void SphereComponent::SetLocalScale( Vec3 const& scale )
{
    float x = MIN( MIN( scale.x, scale.y ), scale.z );
    ShapeComponent::SetLocalScale( Vec3( x, x, x ) );
}

float SphereComponent::GetScaledRadius() const
{
    return m_radius * GetLocalScale().x;
}

float SphereComponent::GetUnscaledRadius() const
{
    return m_radius;
}

bool SphereComponent::IsPointInside( Vec3 const& point ) const
{
    UNUSED( point );
    return false;
}

Vec3 SphereComponent::GetFunctionalCenter() const
{
	return GetWorldPosition();
}

CollisionShape SphereComponent::GetCollisionShape() const
{
    return CollisionShape::SPHERE;
}

SphereComponent SphereComponent::CreateSphereComponent( CollisionInfo info, Character* character, CollisionChannel channel )
{
	float radius = info.data[3];
	Vec3 start = character->GetJointGlobalTransformByName( info.primaryJoint ).GetTranslation3D() * 0.01f;
	Vec3 end = character->GetJointGlobalTransformByName( info.secondaryJoint ).GetTranslation3D() * 0.01f;

	Vec3 pos = (start + end) * 0.5f;

	SphereComponent sphere( radius, true, channel, pos, Quat::IDENTITY );
	sphere.AttachToComponent( character->GetRootComponent() );
	return sphere;
}

AABB3 SphereComponent::CalculateBoundsAABB3D() const
{
    return AABB3();
}

OBB3 SphereComponent::CalculateBoundsOBB3D() const
{
    return OBB3();
}
