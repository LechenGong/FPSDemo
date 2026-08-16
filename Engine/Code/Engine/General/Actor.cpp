#include "Engine/General/Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"

Actor::Actor()
{
	m_rootComponent = new SceneComponent( this );
}

Actor::Actor( Actor* parentActor )
{
	m_rootComponent = new SceneComponent( this );
	m_rootComponent->AttachToComponent( parentActor->GetRootComponent() );
}

Actor::~Actor()
{
	if (m_isEventRecipient && g_eventSystem)
	{
		g_eventSystem->UnsubscribeAllMethodsForObject( this );
	}
	delete m_rootComponent;
}

void Actor::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void Actor::Render() const
{
}

ActorUID Actor::GetUID() const
{
	return m_uid;
}

void Actor::SetUID( ActorUID const& actorUid )
{
	m_uid = actorUid;
}

Mat44 Actor::GetActorLocalTransform() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetLocalTransform() : Mat44();
}

Vec3 Actor::GetActorLocalPosition() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetLocalPosition() : Vec3::ZERO;
}

Quat Actor::GetActorLocalOrientation() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetLocalOrientation() : Quat::IDENTITY;
}

Vec3 Actor::GetActorLocalScale() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetLocalScale() : Vec3::ONE;
}

Mat44 Actor::GetActorWorldTransform() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetWorldTransform() : Mat44();
}

Vec3 Actor::GetActorWorldPosition() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetWorldPosition() : Vec3::ZERO;
}

Quat Actor::GetActorWorldOrientation() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetWorldOrientation() : Quat::IDENTITY;
}

Vec3 Actor::GetActorWorldScale() const
{
    return (m_rootComponent != nullptr) ? m_rootComponent->GetWorldScale() : Vec3::ONE;
}

void Actor::SetActorLocalTransform( Mat44 transform )
{
    if (!m_rootComponent) return;
    m_rootComponent->SetLocalTransform( transform );
}

void Actor::SetActorLocalPosition( Vec3 position )
{
	if (!m_rootComponent) return;
	m_rootComponent->SetLocalPosition( position );
}

void Actor::SetActorLocalOrientation( Quat orientation )
{
	if (!m_rootComponent) return;
	m_rootComponent->SetLocalOrientation( orientation );
}

void Actor::SetActorLocalScale( Vec3 scale )
{
	if (!m_rootComponent) return;
	m_rootComponent->SetLocalScale( scale );
}

void Actor::SetActorWorldTransform( Mat44 transform )
{
    if (!m_rootComponent) return;
	m_rootComponent->SetWorldTransform( transform );
}

void Actor::SetActorWorldPosition( Vec3 position )
{
	if (!m_rootComponent) return;
    m_rootComponent->SetWorldPosition( position );
}

void Actor::SetActorWorldOrientation( Quat orientation )
{
	if (!m_rootComponent) return;
	m_rootComponent->SetWorldOrientation( orientation );
}

void Actor::SetActorWorldScale( Vec3 scale )
{
	if (!m_rootComponent) return;
	m_rootComponent->SetWorldScale( scale );
}

SceneComponent* Actor::GetRootComponent() const
{
	return m_rootComponent;
}

void Actor::SetRootComponent( SceneComponent* newRootComponent )
{
	if (!newRootComponent) return;
	m_rootComponent = newRootComponent;
}

Actor* Actor::GetAttachParentActor() const
{
	return m_rootComponent->GetParent()->GetOwner();
}
