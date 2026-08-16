#pragma once

#include <vector>

#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Quat.hpp"

#include "Engine/General/Object.hpp"

class Actor;

class SceneComponent : public Object
{
public:
	SceneComponent() {};
	SceneComponent( Actor* owner );
	~SceneComponent() {};

public:
	virtual void SetLocalTransform( Mat44 const& transform );
	virtual Mat44 GetLocalTransform() const;
	virtual void SetWorldTransform( Mat44 const& transform );
	virtual Mat44 GetWorldTransform() const;

	virtual void SetLocalPosition( Vec3 const& position );
	virtual Vec3 GetLocalPosition() const;
	virtual void SetWorldPosition( Vec3 const& position );
	virtual Vec3 GetWorldPosition() const;

	virtual void SetLocalOrientation( Quat const& orientation );
	virtual Quat GetLocalOrientation() const;
	virtual void SetWorldOrientation( Quat const& orientation );
	virtual Quat GetWorldOrientation() const;

	virtual void SetLocalScale( Vec3 const& scale );
	virtual Vec3 GetLocalScale() const;
	virtual void SetWorldScale( Vec3 const& scale );
	virtual Vec3 GetWorldScale() const;

public:
	virtual void AttachToComponent( SceneComponent* parentComponent );
	virtual void DetachFromParent();

	virtual SceneComponent* GetParent() const;
	virtual std::vector<SceneComponent*> GetChildren() const;
	virtual void AddChild( SceneComponent* newChild );
	virtual void RemoveChild( SceneComponent* newChild );

	virtual Actor* GetOwner() const;

protected:

protected:
	SceneComponent* m_parent = nullptr;
	std::vector<SceneComponent*> m_children;

	Actor* m_owner = nullptr;

protected:
	Vec3 m_position = Vec3::ZERO;
	Quat m_orientation = Quat::IDENTITY;
	Vec3 m_scale = Vec3::ONE;
};
