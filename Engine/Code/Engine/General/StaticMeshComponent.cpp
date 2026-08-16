#include "Engine/General/StaticMeshComponent.hpp"
#include "Engine/General/StaticMesh.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

StaticMeshComponent::StaticMeshComponent( StaticMesh* staticMesh )
{
    m_staticMesh = staticMesh;
}

void StaticMeshComponent::Render()
{
    if (!m_staticMesh)
        return;

    m_staticMesh->Render();
}

void StaticMeshComponent::SetStaticMesh( StaticMesh* staticMesh )
{
    m_staticMesh = staticMesh;
}

StaticMesh* StaticMeshComponent::GetStaticMesh() const
{
    return m_staticMesh;
}
