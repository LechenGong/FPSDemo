#pragma once

#include "Engine/General/ShapeComponents/ShapeComponent.hpp"

class CubeComponent : public ShapeComponent
{
public:
	CubeComponent( float length, float width, float height, bool collisionEnabled = true, CollisionChannel channel = WORLD_STATIC,
		Vec3 position = Vec3::ZERO, Quat orientation = Quat::IDENTITY, Vec3 scale = Vec3::ONE );

	virtual void SetOrientation( Quat const& orientation );

	float GetScaledLength() const;
	float GetScaledWidth() const;
	float GetScaledHeight() const;
	float GetUnscaledLength() const;
	float GetUnscaledWidth() const;
	float GetUnscaledHeight() const;

	virtual bool IsPointInside( Vec3 const& point ) const override;

	virtual Vec3 GetFunctionalCenter() const override;

public:
	virtual CollisionShape GetCollisionShape() const override;
	//static CubeComponent CreateCubeComponent( CollisionInfo info, Character* character, CollisionChannel channel = WORLD_STATIC );
	static CubeComponent CreateCubeComponent( Vec3 center, Quat orientation, Vec3 scale, float length, float width, float height, CollisionChannel channel = WORLD_STATIC );

	float GetBoundingSphereRadius() const;

public:
	virtual AABB3 CalculateBoundsAABB3D() const override;
	virtual OBB3 CalculateBoundsOBB3D() const override;

protected:
	float m_length = 0.f;
	float m_width = 0.f;
	float m_height = 0.f;
};