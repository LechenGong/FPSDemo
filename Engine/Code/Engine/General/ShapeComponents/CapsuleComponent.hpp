#pragma once

#include "Engine/General/ShapeComponents/ShapeComponent.hpp"

class Character;

class CapsuleComponent : public ShapeComponent
{
public:
	CapsuleComponent( float radius = 0.42f, float height = 0.88f, bool collisionEnabled = true, CollisionChannel channel = WORLD_STATIC,
		Vec3 position = Vec3::ZERO, Quat orientation = Quat::IDENTITY, Vec3 scale = Vec3::ONE );

	virtual void SetOrientation( Quat const& orientation );

	float GetScaledRadius() const;
	float GetScaledHeight() const;
	float GetScaledCapsuleHeight() const;
	float GetUnscaledRadius() const;
	float GetUnscaledHeight() const;

	Vec3 GetWorldBoneStart() const;
	Vec3 GetWorldBoneEnd() const;

	Vec3 GetUpVector() const;

	virtual bool IsPointInside( Vec3 const& point ) const override;

	virtual void UpdateUpVector();

	virtual Vec3 GetFunctionalCenter() const override;

public:
	virtual CollisionShape GetCollisionShape() const override;

	static CapsuleComponent CreateCapsuleComponent( CollisionInfo info, Character* character, CollisionChannel channel = WORLD_STATIC );

public:
	virtual AABB3 CalculateBoundsAABB3D() const override;
	virtual OBB3 CalculateBoundsOBB3D() const override;

protected:
	float m_radius = 0.42f;
	float m_height = 0.88f; // Excluding hemispheres
	Vec3 m_upvectorNormal = Vec3( 0.f, 0.f, 1.f );
};