#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

SkeletalMeshComponent::~SkeletalMeshComponent()
{
}

void SkeletalMeshComponent::Render()
{
    if (!m_skeletalMesh)
        return;

    m_skeletalMesh->Render();
}

void SkeletalMeshComponent::SetSkeletalMesh( SkeletalMesh* skeletalMesh )
{
    m_skeletalMesh = skeletalMesh;
}

SkeletalMesh* SkeletalMeshComponent::GetSkeletalMesh() const
{
    return m_skeletalMesh;
}

std::vector<Mat44>& SkeletalMeshComponent::GetSkeletonGlobalTransform()
{
    return m_jointsGlobalTransform;
}
