#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Engine/Model/Vertex_Anim.hpp"
#include "Engine/Model/ModelUtility.hpp"
#include "Engine/General/Actor.hpp"
#include "Engine/Animation/IKSolver.hpp"

class VertexBuffer;
class IndexBuffer;
class Texture;
class AnimationSequence;
class MeshT;
class Renderer;
class Character;
class AnimationStateMachine;
class Material;

class SkeletalMesh
{
	friend class Character;
public:
	SkeletalMesh();
	SkeletalMesh( std::vector<MeshT*>&meshes, Skeleton const& skeleton );
	~SkeletalMesh();

	void BindMaterial( Material* material, int meshIndex = -1 );

	virtual void Update();
	virtual void Render();

	void SetVisibility( bool flag, int index = -1 );

	Skeleton const& GetSkeleton() const;

	int GetJointIndexByName( std::string name );

protected:
	void InterpolateKeyFrames( KeyFrame const* keyframeA, KeyFrame const* keyframeB, float factor, Mat44& interpolatedTransform );
	void CrossfadeInterpolateTransform( KeyFrame* leaveKeyframeBegin, float leaveFrameRate, float animationLeaveTimer, KeyFrame* enterKeyframeBegin, float enterFrameRate, float animationEnterTimer, float factor, Mat44& interpolatedTransform );
	void InterpolateTransform( KeyFrame* keyFrameBegin, float frameRate, float currentTime, Mat44& interpolatedTransform );

	void UpdateJoints( std::vector<Mat44>& globalTransforms, float currentTime, AnimationSequence* currentAnimation, float previousTime = 0.f, AnimationSequence* previousAnimation = nullptr, float alpha = 0.f );
	void UpdateJoints( std::vector<Mat44>& globalTransforms, AnimationStateMachine* animStateMachine );

	void ApplyFootIK( std::vector<Mat44>& globalTransforms, FootIKConfig const& config, Vec3 const& target, Vec3 const& groundNormal, float ikWeight );

public:
	void ExportToXML( std::string const& filePath ) const;
	static SkeletalMesh* ImportFromXML( std::string const& filePath );

protected:
	std::vector<MeshT> m_meshes;
	Skeleton m_skeleton;
	std::unordered_map<std::string, int> m_jointIndexCheckList;
};
