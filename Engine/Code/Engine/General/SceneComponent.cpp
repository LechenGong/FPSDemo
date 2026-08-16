#include "Engine/General/SceneComponent.hpp"

SceneComponent::SceneComponent( Actor* owner )
{
	m_owner = owner;
}

void SceneComponent::SetLocalTransform( Mat44 const& transform )
{
	m_position = transform.GetTranslation3D();
	m_orientation = Quat( transform );
	m_scale = transform.GetScale3D();
}

Mat44 SceneComponent::GetLocalTransform() const
{
	Mat44 transformMatrix = Mat44::CreateTranslation3D( m_position );
	transformMatrix.Append( m_orientation.ToRotationMatrix() );
	transformMatrix.Append( Mat44::CreateNonUniformScale3D( m_scale ) );

	return transformMatrix;
}

void SceneComponent::SetWorldTransform( Mat44 const& transform )
{
	if (!m_parent)
	{
		SetLocalTransform( transform );
	}
	else
	{
		Mat44 parentWorldTransform = m_parent->GetWorldTransform();
		Mat44 localTransform = parentWorldTransform.GetInverse() * transform;
		SetLocalTransform( localTransform );
	}
}

Mat44 SceneComponent::GetWorldTransform() const
{
	Mat44 selfTransformMatrix = GetLocalTransform();

	if (!m_parent)
		return selfTransformMatrix;

	return m_parent->GetWorldTransform() * selfTransformMatrix;
}

void SceneComponent::SetLocalPosition( Vec3 const& position )
{
	m_position = position;
}

Vec3 SceneComponent::GetLocalPosition() const
{
	return m_position;
}

void SceneComponent::SetWorldPosition( Vec3 const& position )
{
	if (!m_parent)
	{
		SetLocalPosition( position );
	}
	else
	{
		Vec3 parentWorldPosition = m_parent->GetWorldPosition();
		Vec3 localPosition = position - parentWorldPosition;
		SetLocalPosition( localPosition );
	}
}

Vec3 SceneComponent::GetWorldPosition() const
{
	Vec3 localPosition = GetLocalPosition();

	if (!m_parent)
		return localPosition;

	return m_parent->GetWorldPosition() + localPosition;
}

void SceneComponent::SetLocalOrientation( Quat const& orientation )
{
	m_orientation = orientation;
}

Quat SceneComponent::GetLocalOrientation() const
{
	return m_orientation;
}

void SceneComponent::SetWorldOrientation( Quat const& orientation )
{
	if (!m_parent)
	{
		SetLocalOrientation( orientation );
	}
	else
	{
		Quat parentWorldOrientation = m_parent->GetWorldOrientation();
		Quat localOrientation = orientation * parentWorldOrientation.GetInversed();
		SetLocalOrientation( localOrientation );
	}
}

Quat SceneComponent::GetWorldOrientation() const
{
	Quat localOrientation = GetLocalOrientation();

	if (!m_parent)
		return localOrientation;

	return m_parent->GetWorldOrientation() * localOrientation;
}

void SceneComponent::SetLocalScale( Vec3 const& scale )
{
	m_scale = scale;
}

Vec3 SceneComponent::GetLocalScale() const
{
	return m_scale;
}

void SceneComponent::SetWorldScale( Vec3 const& scale )
{
	if (!m_parent)
	{
		SetLocalScale( scale );
	}
	else
	{
		Vec3 parentWorldScale = m_parent->GetWorldScale();
		Vec3 localScale = scale * Vec3( 1.f / parentWorldScale.x, 1.f / parentWorldScale.y, 1.f / parentWorldScale.z );
		SetLocalScale( localScale );
	}
}

Vec3 SceneComponent::GetWorldScale() const
{
	Vec3 localScale = GetLocalScale();

	if (!m_parent)
		return localScale;

	return m_parent->GetWorldScale() * localScale;
}

void SceneComponent::AttachToComponent( SceneComponent* parentComponent )
{
	if (!parentComponent) return;
	if (m_parent) DetachFromParent();
	m_parent = parentComponent;
	parentComponent->AddChild( this );
}

void SceneComponent::DetachFromParent()
{
	if (!m_parent) return;
	m_parent->RemoveChild( this );
	m_parent = nullptr;
}

SceneComponent* SceneComponent::GetParent() const
{
	return m_parent;
}

std::vector<SceneComponent*> SceneComponent::GetChildren() const
{
	return m_children;
}

void SceneComponent::AddChild( SceneComponent* newChild )
{
	int index = -1;
	for (int i = 0; i < m_children.size(); i++)
	{
		if (!m_children[i] && index == -1)
		{
			index = i;
		}
		if (m_children[i] == newChild)
		{
			return;
		}
	}
	if (index != -1)
	{
		m_children[index] = newChild;
	}
	else
	{
		m_children.push_back( newChild );
	}
}

void SceneComponent::RemoveChild( SceneComponent* newChild )
{
	for (int i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == newChild)
		{
			m_children[i] = nullptr;
			return;
		}
	}
}

Actor* SceneComponent::GetOwner() const
{
	return m_owner;
}
