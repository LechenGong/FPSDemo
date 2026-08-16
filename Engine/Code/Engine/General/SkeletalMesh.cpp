#include <utility>

#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Animation/AnimationController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quat.hpp"

namespace
{
constexpr float FOOT_IK_EPSILON = 0.0001f;

Vec3 GetSafeNormalized( Vec3 const& vector, Vec3 const& fallback )
{
	if (vector.GetLengthSquared() > FOOT_IK_EPSILON * FOOT_IK_EPSILON)
	{
		return vector.GetNormalized();
	}

	if (fallback.GetLengthSquared() > FOOT_IK_EPSILON * FOOT_IK_EPSILON)
	{
		return fallback.GetNormalized();
	}

	return Vec3::UP;
}

Vec3 RejectFromAxis( Vec3 const& vector, Vec3 const& axis )
{
	return vector - GetProjectedOnto3D( vector, axis );
}

Quat GetRotationOnlyQuat( Mat44 const& transform )
{
	Mat44 rotationOnly;
	rotationOnly.SetIJKT3D(
		GetSafeNormalized( transform.GetIBasis3D(), Vec3( 1.f, 0.f, 0.f ) ),
		GetSafeNormalized( transform.GetJBasis3D(), Vec3( 0.f, 1.f, 0.f ) ),
		GetSafeNormalized( transform.GetKBasis3D(), Vec3( 0.f, 0.f, 1.f ) ),
		Vec3::ZERO );
	return Quat( rotationOnly ).GetNormalized();
}

Quat GetFromToRotation( Vec3 const& fromDir, Vec3 const& toDir, Vec3 const& fallbackAxis )
{
	Vec3 from = GetSafeNormalized( fromDir, Vec3::UP );
	Vec3 to = GetSafeNormalized( toDir, from );

	float dot = Clamp( DotProduct3D( from, to ), -1.f, 1.f );
	if (dot > 1.f - FOOT_IK_EPSILON)
	{
		return Quat::IDENTITY;
	}

	Vec3 axis = CrossProduct3D( from, to );
	if (axis.GetLengthSquared() <= FOOT_IK_EPSILON * FOOT_IK_EPSILON)
	{
		axis = RejectFromAxis( fallbackAxis, from );
		if (axis.GetLengthSquared() <= FOOT_IK_EPSILON * FOOT_IK_EPSILON)
		{
			axis = RejectFromAxis( Vec3( 1.f, 0.f, 0.f ), from );
		}
		if (axis.GetLengthSquared() <= FOOT_IK_EPSILON * FOOT_IK_EPSILON)
		{
			axis = RejectFromAxis( Vec3( 0.f, 1.f, 0.f ), from );
		}
	}

	axis = GetSafeNormalized( axis, Vec3( 1.f, 0.f, 0.f ) );
	return Quat( axis, ACosDegrees( dot ) ).GetNormalized();
}
}

SkeletalMesh::SkeletalMesh()
{
}

SkeletalMesh::SkeletalMesh( std::vector<MeshT*>& meshes, Skeleton const& skeleton )
{
	m_meshes.reserve( meshes.size() );
	for (int i = 0; i < meshes.size(); i++)
	{
		if (meshes[i])
		{
			m_meshes.push_back( std::move( *meshes[i] ) );
			//delete meshes[i];
			meshes[i] = nullptr;
		}
	}
	m_skeleton = skeleton;
	for (int i = 0; i < m_skeleton.m_joints.size(); i++)
	{
		m_jointIndexCheckList[m_skeleton.m_joints[i].m_name] = i;
	}
}

SkeletalMesh::~SkeletalMesh()
{
}

void SkeletalMesh::BindMaterial( Material* material, int meshIndex )
{
	if (meshIndex == -1)
	{
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].material = material;
		}
	}
	if (meshIndex >= m_meshes.size())
		return;

	m_meshes[meshIndex].material = material;
}

void SkeletalMesh::Update()
{
}

void SkeletalMesh::Render()
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		if (m_meshes[i].isVisible)
		{
			m_meshes[i].Render();
		}
	}
}

void SkeletalMesh::SetVisibility( bool flag, int index )
{
	if (index == -1)
	{
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].isVisible = flag;
		}
		return;
	}
	if (index < 0 || index > m_meshes.size())
		return;

	m_meshes[index].isVisible = flag;
}

Skeleton const& SkeletalMesh::GetSkeleton() const
{
	return m_skeleton;
}

int SkeletalMesh::GetJointIndexByName( std::string name )
{
	return m_jointIndexCheckList[name];
}

void SkeletalMesh::InterpolateKeyFrames( KeyFrame const* keyframeA, KeyFrame const* keyframeB, float factor, Mat44& interpolatedTransform )
{
	if (!keyframeA)
	{
		interpolatedTransform = keyframeB->m_globalTransform;
		return;
	}
	if (!keyframeB)
	{
		interpolatedTransform = keyframeA->m_globalTransform;
		return;
	}
//  	interpolatedTransform = keyframeB->m_globalTransform;
//  	return;
	interpolatedTransform = Mat44::Interpolate( keyframeA->m_globalTransform, keyframeB->m_globalTransform, factor );
}

void SkeletalMesh::CrossfadeInterpolateTransform( KeyFrame* leaveKeyframeBegin, float leaveFrameRate, float animationLeaveTimer, KeyFrame* enterKeyframeBegin, float enterFrameRate, float animationEnterTimer, float factor, Mat44& interpolatedTransform )
{
	Mat44 animationEnterTransform;
	InterpolateTransform( enterKeyframeBegin, enterFrameRate, animationEnterTimer, animationEnterTransform );
	if (leaveKeyframeBegin)
	{
		Mat44 animationLeaveTransform;
		InterpolateTransform( leaveKeyframeBegin, leaveFrameRate, animationLeaveTimer, animationLeaveTransform );
		interpolatedTransform = Mat44::Interpolate( animationLeaveTransform, animationEnterTransform, factor );
	}
	else
	{
		interpolatedTransform = animationEnterTransform;
	}
}

void SkeletalMesh::InterpolateTransform( KeyFrame* keyFrameBegin, float frameRate, float currentTime, Mat44& interpolatedTransform )
{
	KeyFrame* prevKeyFrame = nullptr;
	KeyFrame* nextKeyFrame = nullptr;
	int currentFrame = int( std::floorf( currentTime * frameRate ) );
	for (auto keyFrame = keyFrameBegin; keyFrame; keyFrame = keyFrame->m_next)
	{
		if (keyFrame->m_frameNum <= currentFrame )
		{
			prevKeyFrame = keyFrame;
		}
		if (keyFrame->m_frameNum > currentFrame )
		{
			nextKeyFrame = keyFrame;
			break;
		}
	}

	if (prevKeyFrame && nextKeyFrame)
	{
		float factor = float( currentTime - prevKeyFrame->m_frameNum / frameRate ) / float( nextKeyFrame->m_frameNum - prevKeyFrame->m_frameNum ) * frameRate;
		InterpolateKeyFrames( prevKeyFrame, nextKeyFrame, factor, interpolatedTransform );
	}
	else if (!prevKeyFrame && nextKeyFrame)
	{
		interpolatedTransform = nextKeyFrame->m_globalTransform;
	}
	else if (prevKeyFrame && !nextKeyFrame)
	{
		interpolatedTransform = prevKeyFrame->m_globalTransform;
	}
	else
	{
		// This is an error
		//interpolatedTransform.SetIdentity();
	}
}

void SkeletalMesh::UpdateJoints( std::vector<Mat44>& globalTransforms, float currentTime, AnimationSequence* currentAnimation, float previousTime/* = 0.f*/, AnimationSequence* previousAnimation/* = nullptr*/, float alpha/* = 0.f*/ )
{
	std::vector<Joint> const& joints = m_skeleton.m_joints;
	globalTransforms.clear();
	globalTransforms.reserve( joints.size() );

	for (Joint const& joint : joints)
	{
		if (!currentAnimation || currentAnimation->m_keyFrames.find( joint.m_name ) == currentAnimation->m_keyFrames.end())
		{
			globalTransforms.push_back( Mat44() );
			continue;
		}

		Mat44 globalTransform = Mat44();

		if (previousAnimation)
		{
			if (previousAnimation->m_keyFrames.find( joint.m_name ) == previousAnimation->m_keyFrames.end())
			{
				globalTransforms.push_back( Mat44() );
				continue;
			}

			CrossfadeInterpolateTransform( previousAnimation->m_keyFrames[joint.m_name], previousAnimation->m_frameRate, previousTime,
				currentAnimation->m_keyFrames[joint.m_name], currentAnimation->m_frameRate, currentTime, alpha, globalTransform );
		}
		else
		{
			CrossfadeInterpolateTransform( nullptr, 0.f, 0.f,
				currentAnimation->m_keyFrames[joint.m_name], currentAnimation->m_frameRate, currentTime, alpha, globalTransform );
		}
		
		globalTransforms.push_back( globalTransform );
	}
}

void SkeletalMesh::UpdateJoints( std::vector<Mat44>& globalTransforms, AnimationStateMachine* animStateMachine )
{
	std::vector<Joint> const& joints = m_skeleton.m_joints;
	std::vector<std::vector<Mat44>> transformsSets;

	for (int i = 0; i < animStateMachine->GetOngoingAnimations().size(); i++)
	{
		AnimationStateMachine::StateSet& ongoingAnimation = animStateMachine->GetOngoingAnimation( i );

		if (i > 0 && ongoingAnimation.blendAlpha <= 0)
			continue;
		if (ongoingAnimation.currentState == nullptr)
			continue;

		float currentTimeSeconds = ongoingAnimation.GetCurrentAnimationPlaybackTime();
		AnimationSequence* currentAnimation = ongoingAnimation.GetCurrentState()->GetAnimation();
		float previousTimeSecond = ongoingAnimation.GetPreviousAnimationPlaybackTime();
		AnimationSequence* previousAnimaiton = (ongoingAnimation.GetPreviousState()) ? animStateMachine->GetOngoingAnimation( 0 ).GetPreviousState()->GetAnimation() : nullptr;
		float crossfadeAlpha = ongoingAnimation.GetCrossfadeAlpha();

		std::vector<Mat44> transforms;
		UpdateJoints( transforms, currentTimeSeconds, currentAnimation, previousTimeSecond, previousAnimaiton, crossfadeAlpha);

		if (i > 0) // Todo: can we optimize this?
		{
			int selfIndex = m_skeleton.GetJointIndexByName( ongoingAnimation.rootJoint );
			int parentIndex = m_skeleton.m_joints[selfIndex].m_parentIndex;

			Mat44 parentGlobalTransform = transforms[parentIndex];
			Mat44 selfGlobalTransform = transforms[selfIndex];
			
			Mat44 newParentGlobalTransform = transformsSets[0][parentIndex];

			for (int j = 0; j < transforms.size(); j++)
			{
				transforms[j] = newParentGlobalTransform * parentGlobalTransform.GetInverse() * transforms[j];
			}
		}

		transformsSets.push_back( transforms );
	}
	if (transformsSets.size() == 1)
	{
		globalTransforms = transformsSets[0];
	}
	else
	{
		for (int j = 0; j < joints.size(); j++)
		{
			std::unordered_map<int, bool> jointList = m_skeleton.GetChildrenfromJoint( animStateMachine->GetOngoingAnimation( 1 ).rootJoint );
			float alpha = animStateMachine->GetOngoingAnimation( 1 ).blendAlpha;

			globalTransforms[j] = !jointList[j] ? transformsSets[0][j] : 
				Mat44::Interpolate( transformsSets[0][j], transformsSets[1][j], alpha );
			for (int i = 2; i < animStateMachine->GetOngoingAnimations().size(); i++)
			{
				globalTransforms[j] = !jointList[j] ? globalTransforms[j] :
					Mat44::Interpolate( globalTransforms[j], transformsSets[i][j], animStateMachine->GetOngoingAnimation( i ).blendAlpha );
			}
		}
	}
}

void SkeletalMesh::ApplyFootIK( std::vector<Mat44>& globalTransforms, FootIKConfig const& config, Vec3 const& target, Vec3 const& groundNormal, float ikWeight )
{
	ikWeight = ClampZeroToOne( ikWeight );
	if (ikWeight <= FOOT_IK_EPSILON)
	{
		return;
	}

	int thighIdx = GetJointIndexByName( config.thighJointName );
	int kneeIdx = GetJointIndexByName( config.kneeJointName );
	int footIdx = GetJointIndexByName( config.footJointName );
	if (thighIdx < 0 || kneeIdx < 0 || footIdx < 0)
	{
		return;
	}
	if (thighIdx >= (int)globalTransforms.size() || kneeIdx >= (int)globalTransforms.size() || footIdx >= (int)globalTransforms.size())
	{
		return;
	}
	if (thighIdx >= (int)m_skeleton.m_joints.size() || kneeIdx >= (int)m_skeleton.m_joints.size() || footIdx >= (int)m_skeleton.m_joints.size())
	{
		return;
	}
	if (m_skeleton.m_joints[thighIdx].m_name != config.thighJointName ||
		m_skeleton.m_joints[kneeIdx].m_name != config.kneeJointName ||
		m_skeleton.m_joints[footIdx].m_name != config.footJointName)
	{
		return;
	}

	TwoBoneIKInput input;
	input.rootPos = globalTransforms[thighIdx].GetTranslation3D();
	input.midPos = globalTransforms[kneeIdx].GetTranslation3D();
	input.endPos = globalTransforms[footIdx].GetTranslation3D();
	input.targetPos = target;
	input.poleVector = input.midPos;
	input.rootTransform = globalTransforms[thighIdx];
	input.midTransform = globalTransforms[kneeIdx];
	input.endTransform = globalTransforms[footIdx];

	Mat44 originalFootTransform = input.endTransform;
	Vec3 originalFootPos = input.endPos;
	TwoBoneIKResult output;
	if (!IKSolver::SolveTwoBoneIK( input, output ))
	{
		return;
	}

	Quat originalFootRotation = GetRotationOnlyQuat( originalFootTransform );
	Vec3 currentFootForward = GetSafeNormalized( originalFootTransform.GetIBasis3D(), Vec3( 1.f, 0.f, 0.f ) );
	Vec3 desiredFootUp = GetSafeNormalized( groundNormal, Vec3::UP );
	Quat slopeRotation = GetFromToRotation( Vec3::UP, desiredFootUp, currentFootForward );
	Quat solvedFootRotation = (slopeRotation * originalFootRotation).GetNormalized();
	output.endWorldTransform = Mat44( output.solvedEndPos, solvedFootRotation, originalFootTransform.GetScale3D() );

	globalTransforms[thighIdx] = Mat44::Interpolate( input.rootTransform, output.rootWorldTransform, ikWeight );
	globalTransforms[kneeIdx] = Mat44::Interpolate( input.midTransform, output.midWorldTransform, ikWeight );
	globalTransforms[footIdx] = Mat44::Interpolate( input.endTransform, output.endWorldTransform, ikWeight );

	Vec3 newFootPos = globalTransforms[footIdx].GetTranslation3D();
	Quat blendedFootRotation = GetRotationOnlyQuat( globalTransforms[footIdx] );
	Quat footRotationDelta = (blendedFootRotation * originalFootRotation.GetInversed()).GetNormalized();
	bool hasRotationChange = fabsf( fabsf( footRotationDelta.DotProduct( Quat::IDENTITY ) ) - 1.f ) > FOOT_IK_EPSILON;
	if (newFootPos != originalFootPos || hasRotationChange)
	{
		std::vector<int> jointsToMove = m_skeleton.m_joints[footIdx].m_childrenIndexes;
		while (!jointsToMove.empty())
		{
			int jointIdx = jointsToMove.back();
			jointsToMove.pop_back();

			if (jointIdx < 0 || jointIdx >= (int)globalTransforms.size() || jointIdx >= (int)m_skeleton.m_joints.size())
			{
				continue;
			}

			Mat44 childTransform = globalTransforms[jointIdx];
			Vec3 oldChildPos = childTransform.GetTranslation3D();
			Vec3 childOffset = oldChildPos - originalFootPos;
			Vec3 rotatedOffset = footRotationDelta.Rotate( childOffset );
			Vec3 newChildPos = newFootPos + rotatedOffset;

			Quat childRotation = GetRotationOnlyQuat( childTransform );
			Quat newChildRotation = (footRotationDelta * childRotation).GetNormalized();
			globalTransforms[jointIdx] = Mat44( newChildPos, newChildRotation, childTransform.GetScale3D() );
			for (int childIdx : m_skeleton.m_joints[jointIdx].m_childrenIndexes)
			{
				jointsToMove.push_back( childIdx );
			}
		}
	}
}

void SkeletalMesh::ExportToXML( std::string const& filePath ) const
{
	using namespace tinyxml2;
	XmlDocument doc;

	// Root element
	XmlElement* root = doc.NewElement( "SkeletalMesh" );
	doc.InsertFirstChild( root );

	// Export meshes
	XmlElement* meshesElem = doc.NewElement( "Meshes" );
	for (const MeshT& mesh : m_meshes)
	{
		XmlElement* meshElem = doc.NewElement( "Mesh" );
		meshElem->SetAttribute( "name", mesh.name.c_str() );
		meshElem->SetAttribute( "isVisible", mesh.isVisible );

		// Export material
		XmlElement* materialElem = doc.NewElement( "Material" );
		materialElem->SetAttribute( "name", mesh.material->m_name.c_str() );
		if (mesh.material->m_vertexType == VertexType::VERTEX_PCU)
		{
			materialElem->SetAttribute( "vertexType", "Vertex_PCU" );
		}
		else if (mesh.material->m_vertexType == VertexType::VERTEX_PCUTBN)
		{
			materialElem->SetAttribute( "vertexType", "Vertex_PCUTBN" );
		}
		else
		{
			materialElem->SetAttribute( "vertexType", "Vertex_Anim" );
		}
		
		materialElem->SetAttribute( "color", mesh.material->m_color.ToString().c_str() );
		if (mesh.material->m_shader)
		{
			materialElem->SetAttribute( "shader", mesh.material->m_shader->GetName().c_str() );
		}
		if (mesh.material->m_diffuseMap)
		{
			materialElem->SetAttribute( "diffuseTexture", mesh.material->m_diffuseMap->GetImageFilePath().c_str() );
		}
		if (mesh.material->m_normalMap)
		{
			materialElem->SetAttribute( "normalTexture", mesh.material->m_normalMap->GetImageFilePath().c_str());
		}
		if (mesh.material->m_specGlossEmitMap)
		{
			materialElem->SetAttribute( "specGlossEmitTexture", mesh.material->m_specGlossEmitMap->GetImageFilePath().c_str() );
		}
		if (mesh.material->m_transparencyMap)
		{
			materialElem->SetAttribute( "transparencyTexture", mesh.material->m_transparencyMap->GetImageFilePath().c_str());
		}
		meshElem->InsertEndChild( materialElem );

		// Export vertexes
		XmlElement* vertexesElem = doc.NewElement( "Vertexes" );
		for (const Vertex_PCUTBN& vertex : mesh.vertexes)
		{
			XmlElement* vertexElem = doc.NewElement( "Vertex" );
			vertexElem->SetAttribute( "position", vertex.m_position.ToString().c_str() );
			vertexElem->SetAttribute( "color", vertex.m_color.ToString().c_str() );
			vertexElem->SetAttribute( "uv", vertex.m_uvTexCoords.ToString().c_str() );
			vertexElem->SetAttribute( "tangent", vertex.m_tangent.ToString().c_str() );
			vertexElem->SetAttribute( "bitangent", vertex.m_bitangent.ToString().c_str() );
			vertexElem->SetAttribute( "normal", vertex.m_normal.ToString().c_str() );
			vertexesElem->InsertEndChild( vertexElem );
		}
		meshElem->InsertEndChild( vertexesElem );

		// Export joint influences
		XmlElement* influencesElem = doc.NewElement( "JointInfluences" );
		for (const Vertex_Anim& influence : mesh.jointInfluences)
		{
			XmlElement* influenceElem = doc.NewElement( "Influence" );
			XmlElement* jointIndexesElem = doc.NewElement( "JointIndexes" );
			XmlElement* jointWeightsElem = doc.NewElement( "JointWeights" );

			// Export joint indexes and weights
			for (size_t i = 0; i < 4; ++i)
			{
				XmlElement* indexElem = doc.NewElement( "Index" );
				indexElem->SetText( influence.m_jointIndexes[i] );
				jointIndexesElem->InsertEndChild( indexElem );

				XmlElement* weightElem = doc.NewElement( "Weight" );
				weightElem->SetText( influence.m_jointWeights[i] );
				jointWeightsElem->InsertEndChild( weightElem );
			}
			influenceElem->InsertEndChild( jointIndexesElem );
			influenceElem->InsertEndChild( jointWeightsElem );
			influencesElem->InsertEndChild( influenceElem );
		}

		XmlElement* indexesElem = doc.NewElement( "Indexes" );
		for (const unsigned int index : mesh.indexes)
		{
			XmlElement* indexElem = doc.NewElement( "Index" );
			indexElem->SetText( index );
			indexesElem->InsertEndChild( indexElem );
		}
		meshElem->InsertEndChild( indexesElem );

		meshElem->InsertEndChild( influencesElem );
		meshesElem->InsertEndChild( meshElem );
	}
	root->InsertEndChild( meshesElem );

	// Export skeleton
	XmlElement* skeletonElem = doc.NewElement( "Skeleton" );
	for (const Joint& joint : m_skeleton.m_joints)
	{
		XmlElement* jointElem = doc.NewElement( "Joint" );
		jointElem->SetAttribute( "name", joint.m_name.c_str() );
		jointElem->SetAttribute( "parentIndex", joint.m_parentIndex );
		jointElem->SetAttribute( "globalBindposeInverse", joint.m_globalBindposeInverse.ToString().c_str() );

		// Export children indexes
		XmlElement* childrenElem = doc.NewElement( "ChildrenIndexes" );
		for (int childIndex : joint.m_childrenIndexes)
		{
			XmlElement* childElem = doc.NewElement( "ChildIndex" );
			childElem->SetText( childIndex );
			childrenElem->InsertEndChild( childElem );
		}
		jointElem->InsertEndChild( childrenElem );

		skeletonElem->InsertEndChild( jointElem );
	}
	root->InsertEndChild( skeletonElem );

	// Save the XML file
	doc.SaveFile( filePath.c_str() );
}

SkeletalMesh* SkeletalMesh::ImportFromXML( std::string const& filePath )
{
	using namespace tinyxml2;
	SkeletalMesh* skeletalMesh = new SkeletalMesh;
	XmlDocument doc;

	if (doc.LoadFile( filePath.c_str() ) == XML_SUCCESS)
	{
		XmlElement* root = doc.FirstChildElement( "SkeletalMesh" );

		// Import meshes
		XmlElement* meshesElem = root->FirstChildElement( "Meshes" );
		XmlElement* meshElem = meshesElem->FirstChildElement( "Mesh" );
		while (meshElem)
		{
			MeshT mesh;
			mesh.name = meshElem->Attribute( "name" );
			mesh.isVisible = meshElem->BoolAttribute( "isVisible" );

			// Import material
			mesh.material = new Material;
			XmlElement* materialElem = meshElem->FirstChildElement( "Material" );
			mesh.material->m_name = ParseXmlAttribute( *materialElem, "name", "" );
			std::string shaderPath = ParseXmlAttribute( *materialElem, "shader", "" );
			std::string vertexTypeName = ParseXmlAttribute( *materialElem, "vertexType", "Vertex_PCUTBN" );
			std::string diffuseTexturePath = ParseXmlAttribute( *materialElem, "diffuseTexture", "" );
			std::string normalTexturePath = ParseXmlAttribute( *materialElem, "normalTexture", "" );
			std::string specTexturePath = ParseXmlAttribute( *materialElem, "specGlossEmitTexture", "" );
			std::string transparencyPath = ParseXmlAttribute( *materialElem, "transparencyTexture", "" );
			Rgba8 color = ParseXmlAttribute( *materialElem, "color", Rgba8::WHITE );

			if (vertexTypeName == "Vertex_PCU")
			{
				mesh.material->m_vertexType = VertexType::VERTEX_PCU;
			}
			else if (vertexTypeName == "Vertex_PCUTBN")
			{
				mesh.material->m_vertexType = VertexType::VERTEX_PCUTBN;
			}
			else
			{
				mesh.material->m_vertexType = VertexType::VERTEX_ANIM;
			}
			if (shaderPath != "")
			{
				mesh.material->m_shader = g_theRenderer->CreateShader( shaderPath.c_str(), mesh.material->m_vertexType );
				//m_shader = g_theRenderer->CreateShader( "Data/Shaders/Diffuse.hlsl", m_vertexType);
			}
			if (diffuseTexturePath != "")
			{
				mesh.material->m_diffuseMap = g_theRenderer->CreateOrGetTextureFromFile( diffuseTexturePath.c_str() );
			}
			if (normalTexturePath != "")
			{
				mesh.material->m_normalMap = g_theRenderer->CreateOrGetTextureFromFile( normalTexturePath.c_str() );
			}
			if (specTexturePath != "")
			{
				mesh.material->m_specGlossEmitMap = g_theRenderer->CreateOrGetTextureFromFile( specTexturePath.c_str() );
			}
			if (transparencyPath != "")
			{
				mesh.material->m_transparencyMap = g_theRenderer->CreateOrGetTextureFromFile( transparencyPath.c_str() );
			}

			// Import vertexes
			XmlElement* vertexesElem = meshElem->FirstChildElement( "Vertexes" );
			XmlElement* vertexElem = vertexesElem->FirstChildElement( "Vertex" );
			while (vertexElem)
			{
				Vertex_PCUTBN vertex;
				vertex.m_position.SetFromText( vertexElem->Attribute( "position" ) );
				vertex.m_color.SetFromText( vertexElem->Attribute( "color" ) );
				vertex.m_uvTexCoords.SetFromText( vertexElem->Attribute( "uv" ) );
				vertex.m_tangent.SetFromText( vertexElem->Attribute( "tangent" ) );
				vertex.m_bitangent.SetFromText( vertexElem->Attribute( "bitangent" ) );
				vertex.m_normal.SetFromText( vertexElem->Attribute( "normal" ) );

				mesh.vertexes.push_back( vertex );
				vertexElem = vertexElem->NextSiblingElement( "Vertex" );
			}

			// Import joint influences
			XmlElement* influencesElem = meshElem->FirstChildElement( "JointInfluences" );
			XmlElement* influenceElem = influencesElem->FirstChildElement( "Influence" );
			while (influenceElem)
			{
				Vertex_Anim influence;
				XmlElement* jointIndexesElem = influenceElem->FirstChildElement( "JointIndexes" )->FirstChildElement( "Index" );
				XmlElement* jointWeightsElem = influenceElem->FirstChildElement( "JointWeights" )->FirstChildElement( "Weight" );

				int index = 0;
				while (jointIndexesElem && jointWeightsElem)
				{
					influence.m_jointIndexes[index] = std::stoi( jointIndexesElem->GetText() );
					influence.m_jointWeights[index] = std::stof( jointWeightsElem->GetText() );

					jointIndexesElem = jointIndexesElem->NextSiblingElement( "Index" );
					jointWeightsElem = jointWeightsElem->NextSiblingElement( "Weight" );

					index++;
				}

				mesh.jointInfluences.push_back( influence );
				influenceElem = influenceElem->NextSiblingElement( "Influence" );
			}
			XmlElement* indexesElem = meshElem->FirstChildElement( "Indexes" );
			XmlElement* indexElem = indexesElem->FirstChildElement( "Index" );
			while (indexElem)
			{
				mesh.indexes.push_back( std::stoul( indexElem->GetText() ) );
				indexElem = indexElem->NextSiblingElement( "Index" );
			}
			skeletalMesh->m_meshes.push_back( mesh );
			meshElem = meshElem->NextSiblingElement( "Mesh" );
		}

		// Import skeleton
		XmlElement* skeletonElem = root->FirstChildElement( "Skeleton" );
		XmlElement* jointElem = skeletonElem->FirstChildElement( "Joint" );
		while (jointElem)
		{
			Joint joint;
			joint.m_name = jointElem->Attribute( "name" );
			joint.m_parentIndex = jointElem->IntAttribute( "parentIndex" );
			joint.m_globalBindposeInverse.FromString( jointElem->Attribute( "globalBindposeInverse" ) );

			// Import children indexes
			XmlElement* childrenElem = jointElem->FirstChildElement( "ChildrenIndexes" )->FirstChildElement( "ChildIndex" );
			while (childrenElem)
			{
				joint.m_childrenIndexes.push_back( std::stoi( childrenElem->GetText() ) );
				childrenElem = childrenElem->NextSiblingElement( "ChildIndex" );
			}

			skeletalMesh->m_jointIndexCheckList[joint.m_name] = (int)skeletalMesh->m_skeleton.m_joints.size();
			skeletalMesh->m_skeleton.m_joints.push_back( joint );
			jointElem = jointElem->NextSiblingElement( "Joint" );
		}
	}
	return skeletalMesh;
}
