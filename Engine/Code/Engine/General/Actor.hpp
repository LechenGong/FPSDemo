#pragma once
#include <string>
#include <vector>

#include "Engine/General/SceneComponent.hpp"
#include "Engine/General/Object.hpp"
#include "Engine/General/ActorUID.hpp"

class Renderer;
class GameRun;

class Actor : public Object
{
public:
	Actor();
	Actor( Actor* parentActor );
	virtual ~Actor();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	ActorUID GetUID() const;
	void SetUID( ActorUID const& actorUid );;

public:
	Mat44 GetActorLocalTransform() const;
	Vec3 GetActorLocalPosition() const;
	Quat GetActorLocalOrientation() const;
	Vec3 GetActorLocalScale() const;

	Mat44 GetActorWorldTransform() const;
	Vec3 GetActorWorldPosition() const;
	Quat GetActorWorldOrientation() const;
	Vec3 GetActorWorldScale() const;

	void SetActorLocalTransform( Mat44 transform );
	void SetActorLocalPosition( Vec3 position );
	void SetActorLocalOrientation( Quat orientation );
	void SetActorLocalScale( Vec3 scale );

	void SetActorWorldTransform( Mat44 transform );
	void SetActorWorldPosition( Vec3 position );
	void SetActorWorldOrientation( Quat orientation );
	void SetActorWorldScale( Vec3 scale );

public:
	SceneComponent* GetRootComponent() const;
	void SetRootComponent( SceneComponent* newRootComponent );

	Actor* GetAttachParentActor() const;

protected:
	SceneComponent* m_rootComponent = nullptr;
	ActorUID m_uid;
	bool m_isEventRecipient = false;
};
