#include <unordered_map>

#include "Engine/General/ShapeComponents/ShapeComponent.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/General/Character.hpp"

struct PairHash {
	template <typename T1, typename T2>
	std::size_t operator()( const std::pair<T1, T2>& p ) const {
		return std::hash<T1>{}(p.first) ^ (std::hash<T2>{}(p.second) << 1);
	}
};

std::unordered_map<std::pair<CollisionChannel, CollisionChannel>, CollisionType, PairHash> collisionMap = 
{
	{{CollisionChannel::WORLD_STATIC, CollisionChannel::WORLD_STATIC}, CollisionType::BLOCKING},
	{{CollisionChannel::WORLD_STATIC, CollisionChannel::WORLD_DYNAMIC}, CollisionType::BLOCKING},
	{{CollisionChannel::WORLD_STATIC, CollisionChannel::PAWN}, CollisionType::BLOCKING},
	{{CollisionChannel::WORLD_STATIC, CollisionChannel::CAMERA}, CollisionType::BLOCKING},

	{{CollisionChannel::WORLD_DYNAMIC, CollisionChannel::WORLD_STATIC}, CollisionType::BLOCKING},
	{{CollisionChannel::WORLD_DYNAMIC, CollisionChannel::WORLD_DYNAMIC}, CollisionType::BLOCKING},
	{{CollisionChannel::WORLD_DYNAMIC, CollisionChannel::PAWN}, CollisionType::BLOCKING},
	{{CollisionChannel::WORLD_DYNAMIC, CollisionChannel::CAMERA}, CollisionType::BLOCKING},

	{{CollisionChannel::PAWN, CollisionChannel::WORLD_STATIC}, CollisionType::BLOCKING},
	{{CollisionChannel::PAWN, CollisionChannel::WORLD_DYNAMIC}, CollisionType::BLOCKING},
	{{CollisionChannel::PAWN, CollisionChannel::PAWN}, CollisionType::OVERLAPPING},
	{{CollisionChannel::PAWN, CollisionChannel::CAMERA}, CollisionType::BLOCKING},

	{{CollisionChannel::CAMERA, CollisionChannel::WORLD_STATIC}, CollisionType::BLOCKING},
	{{CollisionChannel::CAMERA, CollisionChannel::WORLD_DYNAMIC}, CollisionType::BLOCKING},
	{{CollisionChannel::CAMERA, CollisionChannel::PAWN}, CollisionType::BLOCKING},
	{{CollisionChannel::CAMERA, CollisionChannel::CAMERA}, CollisionType::IGNORING},
};

ShapeComponent::ShapeComponent()
    : m_collisionEnabled( true )
    , m_collisionChannel( WORLD_STATIC )
{
    SetLocalPosition( Vec3::ZERO );
    SetLocalOrientation( Quat::IDENTITY );
    SetLocalScale( Vec3::ONE );
}

ShapeComponent::ShapeComponent( Vec3 position, Quat orientation, Vec3 scale, bool collisionEnabled, CollisionChannel channel )
    : m_collisionEnabled( collisionEnabled )
    , m_collisionChannel( channel )
{
	SetLocalPosition( position );
	SetLocalOrientation( orientation );
	SetLocalScale( scale );
}

void ShapeComponent::SetLocalPosition( Vec3 const& position )
{
    SceneComponent::SetLocalPosition( position );
    UpdateBoundingBox();
}

void ShapeComponent::SetLocalOrientation( Quat const& orientation )
{
    SceneComponent::SetLocalOrientation( orientation );
    UpdateBoundingBox();
}

void ShapeComponent::SetLocalScale( Vec3 const& scale )
{
    SceneComponent::SetLocalScale( scale );
    UpdateBoundingBox();
}


AABB3 ShapeComponent::GetBoundsAABB3D() const
{
    //return m_boundingBoxAABB;
    return CalculateBoundsAABB3D();
}

OBB3 ShapeComponent::GetBoundsOBB3D() const
{
    //return m_boundingBoxOBB;
    return CalculateBoundsOBB3D();
}

bool ShapeComponent::DoShapesOverlap( ShapeComponent* shapeA, ShapeComponent* shapeB, Vec3& out_mtv_XY )
{
    if (shapeA->GetCollisionShape() == CollisionShape::CAPSULE &&
        shapeB->GetCollisionShape() == CollisionShape::CAPSULE)
    {
        CapsuleComponent* capsuleA = static_cast<CapsuleComponent*>(shapeA);
        CapsuleComponent* capsuleB = static_cast<CapsuleComponent*>(shapeB);
        return DoCapsulesOverlap3D(
            capsuleA->GetWorldBoneStart(),
            capsuleA->GetWorldBoneEnd(),
            capsuleA->GetScaledRadius(),
            capsuleB->GetWorldBoneStart(),
            capsuleB->GetWorldBoneEnd(),
            capsuleB->GetScaledRadius(),
            out_mtv_XY );
    }
    else if (shapeA->GetCollisionShape() == CollisionShape::CAPSULE &&
        shapeB->GetCollisionShape() == CollisionShape::SPHERE)
    {
		CapsuleComponent* capsuleA = static_cast<CapsuleComponent*>(shapeA);
		SphereComponent* sphereA = static_cast<SphereComponent*>(shapeB);
        return DoCapsuleOverlapSphere3D(
            capsuleA->GetWorldBoneStart(),
            capsuleA->GetWorldBoneEnd(),
            capsuleA->GetScaledRadius(),
            sphereA->GetWorldPosition(),
            sphereA->GetScaledRadius(),
            out_mtv_XY );
    }
	else if (shapeA->GetCollisionShape() == CollisionShape::SPHERE &&
		shapeB->GetCollisionShape() == CollisionShape::CAPSULE)
	{
		CapsuleComponent* capsuleA = static_cast<CapsuleComponent*>(shapeB);
		SphereComponent* sphereA = static_cast<SphereComponent*>(shapeA);
		return DoCapsuleOverlapSphere3D(
			capsuleA->GetWorldBoneStart(),
			capsuleA->GetWorldBoneEnd(),
			capsuleA->GetScaledRadius(),
			sphereA->GetWorldPosition(),
			sphereA->GetScaledRadius(),
			out_mtv_XY ) * -1.f;
	}
	if (shapeA->GetCollisionShape() == CollisionShape::SPHERE &&
		shapeB->GetCollisionShape() == CollisionShape::SPHERE)
	{
		SphereComponent* sphereA = static_cast<SphereComponent*>(shapeA);
		SphereComponent* sphereB = static_cast<SphereComponent*>(shapeB);
        return DoSpheresOverlap3D(
            sphereA->GetWorldPosition(),
            sphereA->GetScaledRadius(),
            sphereB->GetWorldPosition(),
            sphereB->GetScaledRadius(),
            out_mtv_XY );
	}
	return false;
}

bool ShapeComponent::DoShapesOverlap( CollisionInfo const& infoA, Character* charaA, CollisionInfo const& infoB, Character* charaB, Vec3& out_mtv_XY )
{
	if (infoA.shape == CollisionShape::CAPSULE &&
		infoB.shape == CollisionShape::CAPSULE)
	{
		CapsuleComponent capsuleA = CapsuleComponent::CreateCapsuleComponent( infoA, charaA );
		Mat44 worldTransformA = charaA->GetActorWorldTransform() * capsuleA.GetLocalTransform();
		CapsuleComponent capsuleB = CapsuleComponent::CreateCapsuleComponent( infoB, charaB );
		Mat44 worldTransformB = charaB->GetActorWorldTransform() * capsuleB.GetLocalTransform();
		bool result = DoCapsulesOverlap3D(
			worldTransformA.GetTranslation3D() - worldTransformA.GetKBasis3D() * (capsuleA.GetScaledCapsuleHeight() * 0.5f - capsuleA.GetScaledRadius()),
			worldTransformA.GetTranslation3D() + worldTransformA.GetKBasis3D() * (capsuleA.GetScaledCapsuleHeight() * 0.5f - capsuleA.GetScaledRadius()),
			capsuleA.GetScaledRadius(),
			worldTransformB.GetTranslation3D() - worldTransformB.GetKBasis3D() * (capsuleB.GetScaledCapsuleHeight() * 0.5f - capsuleB.GetScaledRadius()),
			worldTransformB.GetTranslation3D() + worldTransformB.GetKBasis3D() * (capsuleB.GetScaledCapsuleHeight() * 0.5f - capsuleB.GetScaledRadius()),
			capsuleB.GetScaledRadius(),
			out_mtv_XY );
		capsuleA.DetachFromParent();
		capsuleB.DetachFromParent();
		return result;
	}
	else if (infoA.shape == CollisionShape::CAPSULE &&
		infoB.shape == CollisionShape::SPHERE)
	{
		CapsuleComponent capsuleA = CapsuleComponent::CreateCapsuleComponent( infoA, charaA, CollisionChannel::PAWN );
		Mat44 worldTransformA = charaA->GetActorWorldTransform() * capsuleA.GetLocalTransform();
		SphereComponent sphereA = SphereComponent::CreateSphereComponent( infoB, charaB, CollisionChannel::PAWN );
		Mat44 worldTransformB = charaB->GetActorWorldTransform() * sphereA.GetLocalTransform();
		bool result = DoCapsuleOverlapSphere3D(
			worldTransformA.GetTranslation3D() - worldTransformA.GetKBasis3D() * (capsuleA.GetScaledCapsuleHeight() * 0.5f - capsuleA.GetScaledRadius()),
			worldTransformA.GetTranslation3D() + worldTransformA.GetKBasis3D() * (capsuleA.GetScaledCapsuleHeight() * 0.5f - capsuleA.GetScaledRadius()),
			capsuleA.GetScaledRadius(),
			worldTransformB.GetTranslation3D(),
			sphereA.GetScaledRadius(),
			out_mtv_XY );
		capsuleA.DetachFromParent();
		sphereA.DetachFromParent();
		return result;
	}
	else if (infoA.shape == CollisionShape::SPHERE &&
		infoB.shape == CollisionShape::CAPSULE)
	{
		CapsuleComponent capsuleA = CapsuleComponent::CreateCapsuleComponent( infoB, charaB, CollisionChannel::PAWN );
		Mat44 worldTransformA = charaA->GetActorWorldTransform() * capsuleA.GetLocalTransform();
		SphereComponent sphereA = SphereComponent::CreateSphereComponent( infoA, charaA, CollisionChannel::PAWN );
		Mat44 worldTransformB = charaB->GetActorWorldTransform() * sphereA.GetLocalTransform();
		bool result = DoCapsuleOverlapSphere3D(
			worldTransformA.GetTranslation3D() - worldTransformA.GetKBasis3D() * (capsuleA.GetScaledCapsuleHeight() * 0.5f - capsuleA.GetScaledRadius()),
			worldTransformA.GetTranslation3D() + worldTransformA.GetKBasis3D() * (capsuleA.GetScaledCapsuleHeight() * 0.5f - capsuleA.GetScaledRadius()),
			capsuleA.GetScaledRadius(),
			worldTransformB.GetTranslation3D(),
			sphereA.GetScaledRadius(),
			out_mtv_XY ) * -1.f;
		capsuleA.DetachFromParent();
		sphereA.DetachFromParent();
		return result;
	}
	if (infoA.shape == CollisionShape::SPHERE &&
		infoB.shape == CollisionShape::SPHERE)
	{
		SphereComponent sphereA = SphereComponent::CreateSphereComponent( infoA, charaA );
		Mat44 worldTransformA = charaA->GetActorWorldTransform() * sphereA.GetLocalTransform();
		SphereComponent sphereB = SphereComponent::CreateSphereComponent( infoB, charaB );
		Mat44 worldTransformB = charaB->GetActorWorldTransform() * sphereB.GetLocalTransform();
		bool result = DoSpheresOverlap3D(
			worldTransformA.GetTranslation3D(),
			sphereA.GetScaledRadius(),
			worldTransformB.GetTranslation3D(),
			sphereB.GetScaledRadius(),
			out_mtv_XY );
		sphereA.DetachFromParent();
		sphereB.DetachFromParent();
		return result;
	}
	return false;
}

void ShapeComponent::SetCollisionEnabled( bool enabled )
{
    m_collisionEnabled = enabled;
}

bool ShapeComponent::IsCollisionEnabled() const
{
    return m_collisionEnabled;
}

void ShapeComponent::SetCollisionChannel( CollisionChannel channel )
{
    m_collisionChannel = channel;
}

CollisionChannel ShapeComponent::GetCollisionChannel() const
{
    return m_collisionChannel;
}

CollisionType ShapeComponent::CollisionResultAgainst( ShapeComponent const& other ) const
{
	auto it = collisionMap.find( { GetCollisionChannel(), other.GetCollisionChannel() } );
	return (it != collisionMap.end()) ? it->second : CollisionType::IGNORING;
}

CollisionType ShapeComponent::CollisionResultAgainst( CollisionChannel const& other ) const
{
	auto it = collisionMap.find( { GetCollisionChannel(), other } );
	return (it != collisionMap.end()) ? it->second : CollisionType::IGNORING;
}

void ShapeComponent::UpdateBoundingBox()
{
//     m_boundingBoxAABB = CalculateBoundsAABB3D();
//     m_boundingBoxOBB = CalculateBoundsOBB3D();
}
