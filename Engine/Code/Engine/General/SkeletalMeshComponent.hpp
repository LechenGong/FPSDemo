#pragma once

#include <vector>

#include "Engine/General/SceneComponent.hpp"
#include "Engine/Math/Mat44.hpp"

class SkeletalMesh;
class Renderer;

class SkeletalMeshComponent : public SceneComponent
{
public:
	~SkeletalMeshComponent();

	void Render();

	void SetSkeletalMesh( SkeletalMesh* skeletalMesh );
	SkeletalMesh* GetSkeletalMesh() const;

	std::vector<Mat44>& GetSkeletonGlobalTransform();

protected:

private:
	SkeletalMesh* m_skeletalMesh;
	std::vector<Mat44> m_jointsGlobalTransform;
};
