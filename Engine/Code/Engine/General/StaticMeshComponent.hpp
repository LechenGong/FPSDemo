#pragma once

#include <vector>

#include "Engine/General/SceneComponent.hpp"
#include "Engine/Math/Mat44.hpp"

class StaticMesh;

class StaticMeshComponent : public SceneComponent
{
public:
	StaticMeshComponent() {};
	StaticMeshComponent( StaticMesh* staticMesh );

	void Render();

	void SetStaticMesh( StaticMesh* staticMesh );
	StaticMesh* GetStaticMesh() const;

protected:

private:
	StaticMesh* m_staticMesh = nullptr;
};
