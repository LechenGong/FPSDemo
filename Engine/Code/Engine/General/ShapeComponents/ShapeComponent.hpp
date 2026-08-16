#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Quat.hpp"
#include "Engine/General/SceneComponent.hpp"

enum CollisionType
{
	IGNORING = 0b01,
	OVERLAPPING = 0b10,
	BLOCKING = 0b11,
};

enum CollisionChannel
{
	WORLD_STATIC		 = 0,
	WORLD_DYNAMIC		 = 1,
	PAWN				 = 2,
	CAMERA				 = 3
};

enum class CollisionShape
{
	CAPSULE,
	SPHERE,
	CUBE,
	COUNT
};

enum class CollisionUsage
{
	BODY,
	ATTACK,
	PHYSICS,
	INTERACTION,
	COUNT,
};

struct CollisionInfo
{
	unsigned short index;
	CollisionShape shape;
	std::string primaryJoint = "";
	std::string secondaryJoint = "";
	float data[8] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	CollisionUsage use = CollisionUsage::BODY;
};

class Character;

class ShapeComponent : public SceneComponent
{
public:

	ShapeComponent();
	ShapeComponent( Vec3 position, Quat orientation, Vec3 scale, bool collisionEnabled, CollisionChannel channel );
	virtual ~ShapeComponent() = default;

	virtual void SetLocalPosition( Vec3 const& position ) override;
	virtual void SetLocalOrientation( Quat const& orientation ) override;
	virtual void SetLocalScale( Vec3 const& scale ) override;

	// world
	AABB3 GetBoundsAABB3D() const;

	// world
	OBB3 GetBoundsOBB3D() const;

	// world
	virtual bool IsPointInside( const Vec3& point ) const = 0;

	static bool DoShapesOverlap( ShapeComponent* shapeA, ShapeComponent* shapeB, Vec3& out_mtv_XY );
	static bool DoShapesOverlap( CollisionInfo const& infoA, Character* charaA, CollisionInfo const& infoB, Character* charaB, Vec3& out_mtv_XY );

	virtual Vec3 GetFunctionalCenter() const = 0;

public:
	void SetCollisionEnabled( bool enabled );
	bool IsCollisionEnabled() const;

	void SetCollisionChannel( CollisionChannel channel );
	CollisionChannel GetCollisionChannel() const;

	CollisionType CollisionResultAgainst( ShapeComponent const& other ) const;
	CollisionType CollisionResultAgainst( CollisionChannel const& other ) const;

	virtual CollisionShape GetCollisionShape() const = 0;

public:
	// world
	virtual AABB3 CalculateBoundsAABB3D() const = 0;
	// world
	virtual OBB3 CalculateBoundsOBB3D() const = 0;

	void UpdateBoundingBox();

protected:
// 	AABB3 m_boundingBoxAABB;
// 	OBB3 m_boundingBoxOBB;

	bool m_collisionEnabled = true;

	CollisionShape m_collisionShape = CollisionShape::COUNT;
	CollisionChannel m_collisionChannel = WORLD_STATIC;
};
