#pragma once

#include "Engine/General/ShapeComponents/ShapeComponent.hpp"

class Character;

class SphereComponent : public ShapeComponent
{
public:
	SphereComponent( float radius = 0.42f, bool collisionEnabled = true, CollisionChannel channel = WORLD_STATIC,
		Vec3 position = Vec3::ZERO, Quat orientation = Quat::IDENTITY, Vec3 scale = Vec3::ONE );

	virtual void SetOrientation( Quat const& orientation );

	virtual void SetLocalScale( Vec3 const& scale ) override;

	float GetScaledRadius() const;
	float GetUnscaledRadius() const;

	virtual bool IsPointInside( Vec3 const& point ) const override;

	virtual Vec3 GetFunctionalCenter() const override;

public:
	virtual CollisionShape GetCollisionShape() const override;
	static SphereComponent CreateSphereComponent( CollisionInfo info, Character* character, CollisionChannel channel = WORLD_STATIC );

public:
	virtual AABB3 CalculateBoundsAABB3D() const override;
	virtual OBB3 CalculateBoundsOBB3D() const override;

protected:
	float m_radius = 0.42f;
};