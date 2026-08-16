#pragma comment(lib,"libfbxsdk.lib")

#include <filesystem>

#include "Engine/Model/FBXImporter.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Vec4.hpp"


FBX::FBX( std::string filePath, Renderer* renderer, bool ignoreMaterial, bool ignoreAnimation, RootMotionConfig pRootMotionConfig )
{
	double k = GetCurrentTimeSeconds();

	m_ignoreMaterial = ignoreMaterial;
	m_ignoreAnimation = ignoreAnimation;
	m_rootMotionConfig = pRootMotionConfig;
	//m_scale = scale;
	m_renderer = renderer;
	m_filePath = filePath;
	Initialize();
	InitializeAllTexture();
	DebuggerPrintf( Stringf( "FBX Initialize %f\n", GetCurrentTimeSeconds() - k ).c_str() );
	LoadScene( "bruh" );
	DebuggerPrintf( Stringf( "FBX Scene %f\n", GetCurrentTimeSeconds() - k ).c_str() );
	LoadSkeleton( m_fbxScene->GetRootNode(), 0, -1 );
	DebuggerPrintf( Stringf( "FBX Skeleton %f\n", GetCurrentTimeSeconds() - k ).c_str() );
	LoadNode( m_fbxScene->GetRootNode() );
	DebuggerPrintf( Stringf( "FBX Nodes %f\n", GetCurrentTimeSeconds() - k ).c_str() );
	m_fbxScene->Clear();
	m_fbxScene->Destroy();
	m_fbxManager->Destroy();

	m_animationIndex = 0;
	if (m_animationSequences.size() > 0)
	{
		m_currentAnimation = m_animationSequences[m_animationIndex];
	}

	DebuggerPrintf( Stringf( "Total %f\n", GetCurrentTimeSeconds() - k ).c_str() );
}

FBX::~FBX()
{
}

void FBX::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
// 	FbxTime currentFbxTime;
// 	FbxTime durationFbxTime;
// 	currentFbxTime.SetSecondDouble( double( m_currentAnimation->m_currentTime ) );
// 	durationFbxTime.SetSecondDouble( double( m_currentAnimation->m_duration ) );
// 	UpdateJoints( m_skeleton.m_joints, currentFbxTime, durationFbxTime, m_isLooping );
// 	int vertCount = 0;
// 	for (int i = 0; i < m_meshes.size(); i++)
// 	{
// 		if (m_meshes[i]->isVisible)
// 		{
// 			UpdateVertexes( m_meshes[i]->vertexes, m_meshes[i]->jointInfluences, m_skeleton.m_joints);
// 			delete m_meshes[i]->vertexBuffer;
// 			delete m_meshes[i]->indexBuffer;
// 			m_meshes[i]->vertexBuffer = m_renderer->CreateVertexBuffer( m_meshes[i]->vertexes.size() * sizeof( Vertex_PCUTBN ) );
// 			m_meshes[i]->indexBuffer = m_renderer->CreateIndexBuffer( (int)m_meshes[i]->indexes.size() * sizeof( unsigned int ) );
// 			m_renderer->CopyCPUToGPU( m_meshes[i]->vertexes.data(), m_meshes[i]->vertexes.size(), m_meshes[i]->vertexBuffer, m_meshes[i]->indexes.data(), (int)m_meshes[i]->indexes.size(), m_meshes[i]->indexBuffer );
// 			vertCount += (int)m_meshes[i]->indexes.size();
// 		}
// 	}
// 	size_t pos = m_filePath.find_last_of( '/' );
// 	DebugAddMessage( Stringf( "VertCount for %s: %d", m_filePath.substr( pos + 1, m_filePath.size() - pos + 1 ).c_str(), vertCount ), 0.f );
// 	m_currentAnimation->m_currentTime += deltaSeconds;
// 	if (m_currentAnimation->m_currentTime >= m_currentAnimation->m_duration)
// 	{
// 		m_currentAnimation->m_currentTime -= m_currentAnimation->m_duration;
// 	}
}

void FBX::Render() const
{
	//m_renderer->BindTexture( nullptr );
	//m_renderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	//m_renderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_NONE );
	//m_renderer->SetBlendMode( BlendMode::ALPHA );
	//m_renderer->SetSamplerMode( SamplerMode::BILINEAR_WARP );
	for (int i = 0; i < m_meshes.size(); i++)
	{
		if (m_meshes[i]->isVisible)
		{
			m_meshes[i]->Render();
		}
	}
// 	std::vector<Vertex_PCU> jointVerts;
// 	for (int i = 0; i < m_skeleton.m_joints.size(); i++)
// 	{
// 		if (!m_skeleton.m_joints[i].m_keyFrameBegin)
// 			continue;
// 		FbxVector4 k = m_skeleton.m_joints[i].m_currentGlobalTransform.GetT();
// 		Vec3 g;
// 		g.x = float( k.mData[0] );
// 		g.y = float( k.mData[1] );
// 		g.z = float( k.mData[2] );
// 		g = Mat44::CreateXRotationDegrees( 90.f ).TransformPosition3D( g );
// 		g = Mat44::CreateZRotationDegrees( 90.f ).TransformPosition3D( g );
// 		g *= m_scale;
// 		AddVertsForSphere3D( jointVerts, g, 0.5f * m_scale, Rgba8::RED, AABB2::ZERO_TO_ONE, 8, 4 );
// 	}
// 	m_renderer->BindTexture( nullptr );
// 	m_renderer->DrawVertexArray( jointVerts );
}

void FBX::PlayPrevAnimation()
{
// 	m_animationIndex -= 1;
// 	if (m_animationIndex < 0)
// 	{
// 		m_animationIndex += int( m_animationSequences.size() );
// 	}
// 	m_currentAnimation = m_animationSequences[m_animationIndex];
// 	m_currentAnimation->m_currentTime = 0;
}

void FBX::PlayNextAnimation()
{
// 	m_animationIndex += 1;
// 	if (m_animationIndex >= int( m_animationSequences.size() ))
// 	{
// 		m_animationIndex -= int( m_animationSequences.size() );
// 	}
// 	m_currentAnimation = m_animationSequences[m_animationIndex];
// 	m_currentAnimation->m_currentTime = 0;
}

void FBX::PlayAnimation( int index )
{
	UNUSED( index );
// 	if (index < 0 || index > int( m_animationSequences.size() ))
// 		return;
// 
// 	if (m_currentAnimation != m_animationSequences[index])
// 	{
// 		m_currentAnimation = m_animationSequences[index];
// 		m_currentAnimation->m_currentTime = 0;
// 	}
}

void FBX::SyncMaterialFrom( FBX* fbx )
{
	if (m_meshes.size() != fbx->m_meshes.size())
		ERROR_AND_DIE( "Sync Material Failed" );

	for (int i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i]->material = fbx->m_meshes[i]->material;
	}
}

std::string FBX::GetCurrentAnimationName()
{
	return m_currentAnimation->m_name;
}

std::string FBX::GetAnimationNameByIndex( int index )
{
	return m_animationSequences[index]->m_name;
}

bool FBX::Initialize()
{
	if (m_fbxManager)
	{
		m_fbxManager->Destroy();
	}
	m_fbxManager = FbxManager::Create();
	if (m_fbxManager == NULL)
	{
		return false;
	}

	FbxIOSettings* ios = FbxIOSettings::Create( m_fbxManager, IOSROOT );
	m_fbxManager->SetIOSettings( ios );

	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_MATERIAL, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_TEXTURE, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_LINK, false );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_SHAPE, false );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_GOBO, false );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_ANIMATION, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( IMP_FBX_GLOBAL_SETTINGS, true );

	bool lEmbedMedia = true;
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_MATERIAL, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_TEXTURE, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_EMBEDDED, lEmbedMedia );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_SHAPE, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_GOBO, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_ANIMATION, true );
	(*(m_fbxManager->GetIOSettings())).SetBoolProp( EXP_FBX_GLOBAL_SETTINGS, true );

	FbxString lExtension = "dll";
	FbxString lPath = FbxGetApplicationDirectory();
	m_fbxManager->LoadPluginsDirectory( lPath.Buffer(), lExtension.Buffer() );

	m_fbxScene = FbxScene::Create( m_fbxManager, "" );
	return true;
}

void FBX::InitializeAllTexture()
{
	std::string texturePath = m_filePath.substr( 0, m_filePath.find_last_of( '/' ) ) + "/textures/";

// 	std::string myPath = std::filesystem::current_path().string();
// 	std::filesystem::create_directory( myPath + "\\Saves" );

	if (!std::filesystem::exists( texturePath ))
		return;

	for (const std::filesystem::directory_entry& dirEntry : std::filesystem::recursive_directory_iterator( texturePath ))
	{
		std::string filePath = dirEntry.path().string();
		m_textures[filePath] = m_renderer->CreateOrGetTextureFromFile( filePath.c_str() );
	}
}

bool FBX::LoadScene( char const* pSceneName )
{
	UNUSED( pSceneName );
	if (m_fbxManager == NULL)
	{
		return false;
	}

// 	int major, minor, revision;
// 	FbxManager::GetFileFormatVersion( major, minor, revision );

	FbxImporter* fbxImporter = FbxImporter::Create( m_fbxManager, "" );

	bool importStatus = fbxImporter->Initialize( m_filePath.c_str(), -1, m_fbxManager->GetIOSettings() );

	if (!importStatus)
		return false;

	m_fbxScene->Clear();
	importStatus = fbxImporter->Import( m_fbxScene );

	fbxImporter->Destroy();

	return importStatus;
}

void FBX::LoadSkeleton( FbxNode* pNode, int myIndex, int parentIndex )
{
	if (pNode->GetNodeAttribute())
	{
		std::string k;
		switch (pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eSkeleton:
		{
			Joint newJoint;
			newJoint.m_parentIndex = parentIndex;
			newJoint.m_name = pNode->GetName();
			m_skeleton.m_joints.push_back( newJoint );
			if (parentIndex >= 0)
			{
				m_skeleton.m_joints[parentIndex].m_childrenIndexes.push_back( int(m_skeleton.m_joints.size() - 1) );
			}
			break;
		}
		default:
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		LoadSkeleton( pNode->GetChild( i ), int( m_skeleton.m_joints.size() ), (m_skeleton.m_joints.size() == 0) ? -1 : myIndex );
	}
}

void FBX::LoadNode( FbxNode* pNode )
{
	if (pNode->GetNodeAttribute())
	{
		std::string k;
		switch (pNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			LoadControlPoints( pNode );
			if (!m_skeleton.m_joints.empty())
			{
				LoadJointsAndAnimations( pNode );
			}
			LoadMesh( pNode );
			break;
		case FbxNodeAttribute::eLight:
			break;
		case FbxNodeAttribute::eCamera:
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		LoadNode( pNode->GetChild( i ) );
	}
}

void FBX::LoadMesh( FbxNode* pNode )
{
	FbxMesh* mesh = pNode->GetMesh();
	if (mesh == NULL)
		return;

	bool isTriangleMesh = mesh->IsTriangleMesh();
	int polygonCount = mesh->GetPolygonCount();
	int vertexCounter = 0;

	std::vector<Vertex_PCUTBN> verts;
	std::vector<Vertex_Anim> jointInfluences;
	std::vector<unsigned int> indexes;
	verts.reserve( isTriangleMesh ? polygonCount * 3 : polygonCount * 4 );
	jointInfluences.reserve( isTriangleMesh ? polygonCount * 3 : polygonCount * 4 );
	indexes.reserve( isTriangleMesh ? polygonCount * 3 : polygonCount * 6 );
	
	for (int i = 0; i < polygonCount; i++)
	{
		if (isTriangleMesh)
		{
			for (int j = 0; j < 3; j++)
			{
				int ctrlPointVertex = mesh->GetPolygonVertex( i, j );
				ControlPoint* currControlPoint = m_controlPoints[ctrlPointVertex];

				Vec3 position;
				ReadVertex( mesh, ctrlPointVertex, position );
// 
// 				static const Mat44 XRotation90 = Mat44::CreateXRotationDegrees( 90.f );
// 				static const Mat44 ZRotation90 = Mat44::CreateZRotationDegrees( 90.f );
// 				position = XRotation90.TransformPosition3D( position );
// 				position = ZRotation90.TransformPosition3D( position );
// 				position = position * Vec3( 0.01, 0.01, 0.01 );
// 
				// Delete This: Scaling
				//position *= 10;

				Rgba8 color = Rgba8::WHITE;
				ReadColor( mesh, ctrlPointVertex, vertexCounter, color );

				Vec2 uv;
				ReadUVs( mesh, ctrlPointVertex, mesh->GetTextureUVIndex( i, j ), 0, uv );

				// 			Vec2 uvLightMap;
				// 			ReadUVs( mesh, ctrlPointVertex, mesh->GetTextureUVIndex( i, j ), 1, uvLightMap );

				Vec3 tangent;
				ReadTangent( mesh, ctrlPointVertex, vertexCounter, tangent );

				Vec3 binormal;
				ReadBinormal( mesh, ctrlPointVertex, vertexCounter, binormal );

				Vec3 normal;
				ReadNormal( mesh, ctrlPointVertex, vertexCounter, normal );

				Vertex_PCUTBN vertex( position, color, uv, tangent, binormal, normal );
				Vertex_Anim vertexAnim;

				int index = 0;
				for (int k = 0; k < currControlPoint->m_blendingInfo.size(); k++)
				{
					if (currControlPoint->m_blendingInfo[k].m_blendingWeight == 0.0)
					{
						index++;
						continue;
					}

					vertexAnim.m_jointIndexes[index] = unsigned int( currControlPoint->m_blendingInfo[k].m_blendingIndex );
					vertexAnim.m_jointWeights[index] = float( currControlPoint->m_blendingInfo[k].m_blendingWeight );
					index++;
				}

				verts.push_back( vertex );
				jointInfluences.push_back( vertexAnim );
				indexes.push_back( (unsigned int)vertexCounter );

				vertexCounter++;
			}
		}
 		else
 		{
 			int indexNext = (int)jointInfluences.size();
			int counter = 0;
 			for (int j = 0; j < 4; j++)
 			{
 				int ctrlPointVertex = mesh->GetPolygonVertex( i, j );
				if (ctrlPointVertex == -1)
					continue;
				ControlPoint* currControlPoint = m_controlPoints[ctrlPointVertex];

				if (!currControlPoint)
					continue;
				counter++;

 				Vec3 position;
 				ReadVertex( mesh, ctrlPointVertex, position );
 
 				// Delete This: Scaling
 				//position *= 10;
 
 				Rgba8 color = Rgba8::WHITE;
 				ReadColor( mesh, ctrlPointVertex, vertexCounter, color );
 
 				Vec2 uv;
 				ReadUVs( mesh, ctrlPointVertex, mesh->GetTextureUVIndex( i, j ), 0, uv );

 				// 			Vec2 uvLightMap;
 				// 			ReadUVs( mesh, ctrlPointVertex, mesh->GetTextureUVIndex( i, j ), 1, uvLightMap );
 
 				Vec3 tangent;
 				ReadTangent( mesh, ctrlPointVertex, vertexCounter, tangent );
 
 				Vec3 binormal;
 				ReadBinormal( mesh, ctrlPointVertex, vertexCounter, binormal );
 
 				Vec3 normal;
 				ReadNormal( mesh, ctrlPointVertex, vertexCounter, normal );
 
 				Vertex_PCUTBN vertex( position, color, uv, tangent, binormal, normal );
				Vertex_Anim vertexAnim;

				int index = 0;
				for (int k = 0; k < currControlPoint->m_blendingInfo.size(); k++)
				{
					if (currControlPoint->m_blendingInfo[k].m_blendingWeight == 0.0)
					{
						index++;
						continue;
					}

					vertexAnim.m_jointIndexes[index] = unsigned int( currControlPoint->m_blendingInfo[k].m_blendingIndex );
					vertexAnim.m_jointWeights[index] = float( currControlPoint->m_blendingInfo[k].m_blendingWeight );
					index++;
				}

				verts.push_back( vertex );
				jointInfluences.push_back( vertexAnim );

				vertexCounter++;
 			}
			if (counter < 3)
			{
				ERROR_AND_DIE( "polygon not even has 3 vertexes" );
			}
 			indexes.push_back( indexNext + 0 );
 			indexes.push_back( indexNext + 1 );
 			indexes.push_back( indexNext + 2 );
			if (counter == 4)
			{
				indexes.push_back( indexNext + 2 );
				indexes.push_back( indexNext + 3 );
				indexes.push_back( indexNext + 0 );
			}
			if (counter > 4)
			{
				ERROR_AND_DIE( "encountered n-gon: case not supported" );
			}
 		}
	}
	
	MeshT* newMesh = new MeshT();
	newMesh->name = pNode->GetName();
	newMesh->isVisible = true;
	newMesh->vertexes = verts;
	newMesh->jointInfluences = jointInfluences;
	newMesh->indexes = indexes;
// 	newMesh->vertexBuffer = m_renderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCUTBN ) );
// 	newMesh->indexBuffer = m_renderer->CreateIndexBuffer( indexes.size() * sizeof( unsigned int ) );
// 	m_renderer->CopyCPUToGPU( verts.data(), verts.size(), newMesh->vertexBuffer, indexes.data(), indexes.size(), newMesh->indexBuffer );

	LoadMaterial( newMesh, mesh );
	CalculateTangantSpaceBasisVectors( newMesh->vertexes, newMesh->indexes, true, true );
	m_meshes.push_back( newMesh );
}

void FBX::LoadControlPoints( FbxNode* pNode )
{
	FbxMesh* mesh = pNode->GetMesh();
	int controlPointCount = mesh->GetControlPointsCount();
	for (int i = 0; i < controlPointCount; i++)
	{
		ControlPoint* controlPoint = new ControlPoint();
		Vec3 position(
			float( mesh->GetControlPointAt( i ).mData[0] ),
			float( mesh->GetControlPointAt( i ).mData[1] ),
			float( mesh->GetControlPointAt( i ).mData[2] )
		);
		controlPoint->m_position = position;
		m_controlPoints[i] = controlPoint;
	}
}

void FBX::LoadJointsAndAnimations( FbxNode* pNode )
{
	FbxMesh* mesh = pNode->GetMesh();
	int deformerCount = mesh->GetDeformerCount();

	FbxAMatrix geometryTransformFBX = GetGeometryTransformation( pNode );

	Mat44 geometryTransform = ConvertFbxAMatrixToMat44( geometryTransformFBX );
	Mat44 localTransform = ConvertFbxAMatrixToMat44( GetLocalTransformation( pNode ) );

	FbxSkin* skin = nullptr;
	for (int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++)
	{
		skin = FbxCast<FbxSkin>( mesh->GetDeformer( deformerIndex, FbxDeformer::eSkin ) );
		if (!skin) continue;

		int clusterCount = skin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
		{
			FbxCluster* cluster = skin->GetCluster( clusterIndex );
			std::string jointName = cluster->GetLink()->GetName();
			int jointIndex = FindJointIndexByName( jointName );

			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;

			cluster->GetTransformMatrix( transformMatrix );
			cluster->GetTransformLinkMatrix( transformLinkMatrix );

			Mat44 globalBindposeInverseMatrix = ConvertFbxAMatrixToMat44( transformLinkMatrix.Inverse() ) * ConvertFbxAMatrixToMat44( transformMatrix ) * geometryTransform;

			m_skeleton.m_joints[jointIndex].m_globalBindposeInverse = globalBindposeInverseMatrix;
			//m_skeleton.m_joints[jointIndex].m_node = cluster->GetLink();

			int indexesCount = cluster->GetControlPointIndicesCount();
			for (int i = 0; i < indexesCount; i++)
			{
				BlendingIndexWeightPair blendingIndexWeightPair;
				blendingIndexWeightPair.m_blendingIndex = jointIndex;
				blendingIndexWeightPair.m_blendingWeight = cluster->GetControlPointWeights()[i];
				m_controlPoints[cluster->GetControlPointIndices()[i]]->m_blendingInfo.push_back( blendingIndexWeightPair );
			}

			if (m_ignoreAnimation)
				continue;

			int animationCount = m_fbxScene->GetSrcObjectCount<FbxAnimStack>();
			m_animationSequences.resize( animationCount );

			for (int animationIndex = 0; animationIndex < animationCount; animationIndex++)
			{
				FbxAnimStack* animStack = m_fbxScene->GetSrcObject<FbxAnimStack>( animationIndex );
				FbxTakeInfo* takeInfo = m_fbxScene->GetTakeInfo( animStack->GetName() );
				std::string animStackName = animStack->GetName();
				FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
				FbxTime::EMode timeMode = m_fbxScene->GetGlobalSettings().GetTimeMode();
				float frameRate = ConvertEModeToFrameRate( timeMode );
				FbxTime duration = end - start;
				float durationSeconds = float( duration.GetSecondDouble() );
				if (!m_animationSequences[animationIndex])
				{
					m_animationSequences[animationIndex] = new AnimationSequence( animStackName, frameRate, durationSeconds );
					m_animationSequences[animationIndex]->m_rootTranslation.resize( end.GetFrameCount( timeMode ) - start.GetFieldCount( timeMode ) + 1 );
					m_animationSequences[animationIndex]->m_rootRotation.resize( end.GetFrameCount( timeMode ) - start.GetFieldCount( timeMode ) + 1 );
				}
				m_animationSequences[animationIndex]->m_keyFrames[jointName] = new KeyFrame[end.GetFrameCount( timeMode ) - start.GetFieldCount( timeMode ) + 1];
			}
		}
	}

	if (m_ignoreAnimation)
		return;

	int animationCount = m_fbxScene->GetSrcObjectCount<FbxAnimStack>();
	m_animationSequences.resize( animationCount );

	FbxVector4 currentTotalReverseDisplacement;
	FbxQuaternion currentTotalRotation;
	FbxQuaternion previousRotation;
	for (int animationIndex = 0; animationIndex < animationCount; animationIndex++)
	{
		FbxAnimStack* animStack = m_fbxScene->GetSrcObject<FbxAnimStack>( animationIndex );
		m_fbxScene->SetCurrentAnimationStack( animStack );
		std::string animStackName = animStack->GetName();
		FbxTakeInfo* takeInfo = m_fbxScene->GetTakeInfo( animStack->GetName() );
		FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
		FbxTime::EMode timeMode = m_fbxScene->GetGlobalSettings().GetTimeMode();
		FbxTime duration = end - start;

		FbxVector4 currentPosition;
		FbxVector4 previousPosition;
		FbxVector4 deltaPosition;

		FbxQuaternion currentRotation;
		FbxQuaternion deltaRotation;

		FbxAMatrix previousGlobalTransform;
		previousGlobalTransform.SetIdentity();

		for (FbxLongLong i = start.GetFrameCount( timeMode ); i <= end.GetFrameCount( timeMode ); i++)
		{
			if (!skin) break;

			int clusterCount = skin->GetClusterCount();
			FbxAMatrix rootGlobalTransform;
			FbxAMatrix rootLocalTransform;
			FbxAMatrix defaultRootTransform;
			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
			{
				FbxCluster* cluster = skin->GetCluster( clusterIndex );
				std::string jointName = cluster->GetLink()->GetName();
				FbxNode* parentNode = cluster->GetLink()->GetParent();

				FbxTime currTime;
				currTime.SetFrame( i, timeMode );

				bool isRoot = false;
				if (parentNode != nullptr)
				{
					FbxSkeleton* parentSkeleton = parentNode->GetSkeleton();
					isRoot = (parentSkeleton == nullptr); // No parent skeleton => root
				}

				if (isRoot)
				{
					if (i != 0)
					{
						rootGlobalTransform = cluster->GetLink()->EvaluateGlobalTransform( currTime );

// 						currentRotation = rootGlobalTransform.GetQ();
// 						deltaRotation = currentRotation / previousRotation;
// 						previousRotation = currentRotation;
// 
// 						FbxAMatrix rootGlobalTransformNoRotation = rootGlobalTransform;
// 						rootGlobalTransformNoRotation.SetQ( FbxQuaternion( 1, 0, 0, 0 ) );
// 						currentPosition = rootGlobalTransformNoRotation.GetT();
// 						deltaPosition = currentPosition - previousPosition;
// 						previousPosition = currentPosition;

						FbxAMatrix deltaGlobalTransform = rootGlobalTransform * previousGlobalTransform.Inverse();
						deltaPosition = deltaGlobalTransform.GetT();
						deltaRotation = deltaGlobalTransform.GetQ();
						previousGlobalTransform = rootGlobalTransform;
						break;
					}
					else
					{
						rootGlobalTransform = cluster->GetLink()->EvaluateGlobalTransform( currTime );
						rootLocalTransform = cluster->GetLink()->EvaluateLocalTransform( currTime );
						previousGlobalTransform = rootGlobalTransform;
// 						previousRotation = rootGlobalTransform.GetQ();
// 						FbxAMatrix rootGlobalTransformNoRotation = rootGlobalTransform;
// 						rootGlobalTransformNoRotation.SetQ( FbxQuaternion( 1, 0, 0, 0 ) );
// 						previousPosition = rootGlobalTransformNoRotation.GetT();
						break;
					}
				}
			}

			if (!m_rootMotionConfig.removeForward)
				deltaPosition[2] = 0.0;
			if (!m_rootMotionConfig.removeLeftward)
				deltaPosition[0] = 0.0;
			if (!m_rootMotionConfig.removeUpward)
				deltaPosition[1] = 0.0;

			currentTotalReverseDisplacement -= deltaPosition;
			m_animationSequences[animationIndex]->m_rootTranslation[i] = Vec3( (float)deltaPosition[2], (float)deltaPosition[0], (float)deltaPosition[1] );

			FbxVector4 eulerAngle = deltaRotation.DecomposeSphericalXYZ();

			eulerAngle[0] = 0.0;
			eulerAngle[2] = 0.0;

			if (!m_rootMotionConfig.removeRotation)
			{
				eulerAngle[0] = 0.0;
				eulerAngle[1] = 0.0;
				eulerAngle[2] = 0.0;
			}

			float yaw, pitch, roll;
			yaw = (float)eulerAngle[0];
			pitch = (float)eulerAngle[1];
			roll = (float)eulerAngle[2];

			float nyaw, npitch, nroll;

			xyzToZyx( yaw, pitch, roll, nyaw, npitch, nroll );

			m_animationSequences[animationIndex]->m_rootRotation[i] = Vec3( nyaw, npitch, nroll );

			FbxQuaternion a;
			a.ComposeSphericalXYZ( eulerAngle );
			currentTotalRotation = a * currentTotalRotation;

			// Incomplete
// 			FbxVector4 rootGlobalYaw = rootGlobalTransform.GetQ().DecomposeSphericalXYZ();
// 			rootGlobalYaw[1] = 0.f;
// 			rootGlobalYaw[2] = 0.f;
// 			rootGlobalYaw[3] = 0.f;
// 			FbxQuaternion yawOnlyQuat;
// 			yawOnlyQuat.ComposeSphericalXYZ( rootGlobalYaw );
// 			yawOnlyQuat.Inverse();
// 			FbxAMatrix CTRRMat;
// 			CTRRMat.SetQ( yawOnlyQuat );
// 			FbxVector4 newDeltaTranslation = CTRRMat.MultT( deltaPosition );
// 			m_animationSequences[animationIndex]->m_rootTranslation[i] = Vec3( (float)newDeltaTranslation[2], (float)newDeltaTranslation[0], (float)newDeltaTranslation[1] );

			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
			{
				FbxCluster* cluster = skin->GetCluster( clusterIndex );
				std::string jointName = cluster->GetLink()->GetName();
				int jointIndex = FindJointIndexByName( jointName );
				
				FbxTime currTime;
				currTime.SetFrame( i, timeMode );
				KeyFrame *anim = &m_animationSequences[animationIndex]->m_keyFrames[jointName][i - start.GetFrameCount( timeMode )];
				(anim)->m_frameNum = i - start.GetFrameCount( timeMode );

				FbxAMatrix reverseRootMotion;
 				FbxQuaternion currentTotalReverseRotation = currentTotalRotation;
 				currentTotalReverseRotation.Inverse();
				reverseRootMotion.SetQ( currentTotalReverseRotation );
				reverseRootMotion.SetT( currentTotalReverseDisplacement );

				FbxAMatrix transformOffset = pNode->EvaluateGlobalTransform( currTime ) * geometryTransformFBX;
				FbxAMatrix clusterGlobalTransform = reverseRootMotion * cluster->GetLink()->EvaluateGlobalTransform( currTime );
				(anim)->m_globalTransform = localTransform * ConvertFbxAMatrixToMat44( transformOffset.Inverse() ) * ConvertFbxAMatrixToMat44( clusterGlobalTransform );

				if (i - start.GetFrameCount( timeMode ) > 0)
				{
					m_animationSequences[animationIndex]->m_keyFrames[jointName][i - start.GetFrameCount( timeMode ) - 1].m_next = anim;
				}
				else
				{
					m_skeleton.m_joints[jointIndex].m_keyFrameBegin = anim;
				}
			}
		}

		BlendingIndexWeightPair blendingIndexWeightPair;
		blendingIndexWeightPair.m_blendingIndex = 0;
		blendingIndexWeightPair.m_blendingWeight = 0;
		for (auto iter = m_controlPoints.begin(); iter != m_controlPoints.end(); iter++)
		{
			for (int i = int( iter->second->m_blendingInfo.size() ); i <= 4; i++)
			{
				iter->second->m_blendingInfo.push_back( blendingIndexWeightPair );
			}
		}
	}
}

void FBX::ReadVertex( FbxMesh* pMesh, int ctrlPointIndex, Vec3& position )
{
	FbxVector4* vertexArray = pMesh->GetControlPoints();
	position.x = (float)vertexArray[ctrlPointIndex][0];
	position.y = (float)vertexArray[ctrlPointIndex][1];
	position.z = (float)vertexArray[ctrlPointIndex][2];
}

void FBX::ReadColor( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Rgba8& color )
{
	if (pMesh->GetElementVertexColorCount() < 1)
		return;

	FbxGeometryElementVertexColor* vertexColor = pMesh->GetElementVertexColor( 0 );

	switch (vertexColor->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexColor->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( ctrlPointIndex ).mAlpha);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexColor->GetIndexArray().GetAt( ctrlPointIndex );
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mAlpha);
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexColor->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( vertexCount ).mAlpha);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexColor->GetIndexArray().GetAt( vertexCount );
			color.r = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mRed);
			color.g = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mGreen);
			color.b = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mBlue);
			color.a = (unsigned char)(255.f * vertexColor->GetDirectArray().GetAt( index ).mAlpha);
			break;
		}
		default:
			break;
		}
		break;
	}
	}
}

void FBX::ReadUVs( FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int UVLayer, Vec2& UVs )
{
	if (UVLayer >= 2 || pMesh->GetElementUVCount() <= UVLayer)
		return;

	FbxGeometryElementUV* uv = pMesh->GetElementUV( UVLayer );

	switch (uv->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (uv->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			UVs.x = (float)uv->GetDirectArray().GetAt( ctrlPointIndex )[0];
			UVs.y = (float)uv->GetDirectArray().GetAt( ctrlPointIndex )[1];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			
			int index = uv->GetIndexArray().GetAt( ctrlPointIndex );
			UVs.x = (float)uv->GetDirectArray().GetAt( index )[0];
			UVs.y = (float)uv->GetDirectArray().GetAt( index )[1];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (uv->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			break;

		case FbxGeometryElement::eIndexToDirect:
		{
			UVs.x = (float)uv->GetDirectArray().GetAt( textureUVIndex )[0];
			UVs.y = (float)uv->GetDirectArray().GetAt( textureUVIndex )[1];
			break;
		}
		default:
			break;
		}
		break;
	}
	}
}

void FBX::ReadTangent( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& tangent )
{
	if (pMesh->GetElementTangentCount() < 1)
	{
		return;
	}
	FbxGeometryElementTangent* vertexTangent = pMesh->GetElementTangent( 0 );
	
	switch (vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( ctrlPointIndex )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( ctrlPointIndex )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( ctrlPointIndex )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt( ctrlPointIndex );
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( index )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( index )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( vertexCount )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( vertexCount )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( vertexCount )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt( vertexCount );
			tangent.x = (float)vertexTangent->GetDirectArray().GetAt( index )[0];
			tangent.y = (float)vertexTangent->GetDirectArray().GetAt( index )[1];
			tangent.z = (float)vertexTangent->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void FBX::ReadBinormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& binormal )
{
	if (pMesh->GetElementBinormalCount() < 1)
	{
		return;
	}
	FbxGeometryElementBinormal* vertexBinormal = pMesh->GetElementBinormal( 0 );

	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( ctrlPointIndex )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( ctrlPointIndex )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( ctrlPointIndex )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt( ctrlPointIndex );
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( index )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( index )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( vertexCount )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( vertexCount )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( vertexCount )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt( vertexCount );
			binormal.x = (float)vertexBinormal->GetDirectArray().GetAt( index )[0];
			binormal.y = (float)vertexBinormal->GetDirectArray().GetAt( index )[1];
			binormal.z = (float)vertexBinormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}


void FBX::ReadNormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& normal )
{
	if (pMesh->GetElementNormalCount() < 1)
	{
		return;
	}
	FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal( 0 );

	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( ctrlPointIndex )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( ctrlPointIndex )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( ctrlPointIndex )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( ctrlPointIndex );
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( index )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( index )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( vertexCount )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( vertexCount )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( vertexCount )[2];
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( vertexCount );
			normal.x = (float)vertexNormal->GetDirectArray().GetAt( index )[0];
			normal.y = (float)vertexNormal->GetDirectArray().GetAt( index )[1];
			normal.z = (float)vertexNormal->GetDirectArray().GetAt( index )[2];
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void FBX::LoadMaterial( MeshT* localMesh, FbxMesh* pMesh )
{
	int materialCount = 0;
	//int polygonCount = pMesh->GetPolygonCount();
	FbxNode* node = nullptr;
	
	if (pMesh && pMesh->GetNode())
	{
		node = pMesh->GetNode();
		materialCount = node->GetMaterialCount();
	}
	if (materialCount <= 0)
		return;

	bool isAllSame = true;

	for (int i = 0; i < pMesh->GetElementMaterialCount(); i++)
	{
		FbxGeometryElementMaterial* materialElement = pMesh->GetElementMaterial( i );
		if (materialElement->GetMappingMode() == FbxGeometryElement::eByPolygon)
		{
			isAllSame = false;
			break;
		}
	}

	if (/*isAllSame || a < 2*/true)
	{
		for (int i = 0; i < pMesh->GetElementMaterialCount(); i++)
		{

			FbxGeometryElementMaterial* materialElement = pMesh->GetElementMaterial( i );
			FbxSurfaceMaterial* material = pMesh->GetNode()->GetMaterial( materialElement->GetIndexArray().GetAt( 0 ) );
			if (!material) continue;

			LoadMaterialAttribute( localMesh, material );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sAmbient );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sAmbientFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sBump );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sBumpFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sDiffuse );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sDiffuseFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sDisplacementColor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sDisplacementFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sEmissive );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sEmissiveFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sReflection );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sMultiLayer );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sNormalMap );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sReflection );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sReflectionFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sShadingModel );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sShadingModelDefault );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sShininess );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sSpecular );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sSpecularFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sTransparentColor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sTransparencyFactor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sVectorDisplacementColor );
			LoadMaterialTexture( localMesh, material, FbxSurfaceMaterial::sVectorDisplacementFactor );
		}
	}
	else
	{
//  		int materialId = 0;
//  		int polygonId = 0;
 		//polygonCount = 0;

	}

// 	for (int i = 0; i < materialCount; i++)
// 	{
// 		FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial( i );
// 		std::string name = surfaceMaterial->GetName();
// 		LoadMaterialAttribute( surfaceMaterial );
// 		LoadMaterialTexture( surfaceMaterial );
// 	}
}

void FBX::LoadMaterialAttribute( MeshT* localMesh, FbxSurfaceMaterial* pSurfaceMaterial )
{
	UNUSED( localMesh );

	pSurfaceMaterial->GetName();

 	if (pSurfaceMaterial->GetClassId().Is( FbxSurfacePhong::ClassId ))
 	{
 		FbxDouble3 fbxAmbient = ((FbxSurfacePhong*)pSurfaceMaterial)->Ambient;
 		Vec3 ambient( (float)fbxAmbient[0], (float)fbxAmbient[1], (float)fbxAmbient[2] );
 
 		FbxDouble3 fbxDiffuse = ((FbxSurfacePhong*)pSurfaceMaterial)->Diffuse;
 		Vec3 diffuse( (float)fbxDiffuse[0], (float)fbxDiffuse[1], (float)fbxDiffuse[2] );
 
 		FbxDouble3 fbxSpecular = ((FbxSurfacePhong*)pSurfaceMaterial)->Specular;
 		Vec3 specular( (float)fbxSpecular[0], (float)fbxSpecular[1], (float)fbxSpecular[2] );
 	}
}

void FBX::LoadMaterialTexture( MeshT* localMesh, FbxSurfaceMaterial* pSurfaceMaterial, const char* textureType )
{
	FbxProperty property;
	property = pSurfaceMaterial->FindProperty( textureType );

	if (!property.IsValid())
		return;

	int textureCount = property.GetSrcObjectCount<FbxTexture>();
	for (int i = 0; i < textureCount; i++)
	{
		FbxTexture* diffuseMap = FbxCast<FbxTexture>( property.GetSrcObject( i ) );
		if (!diffuseMap)
			continue;
		std::string str = diffuseMap->GetName();
		FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>( diffuseMap );
		std::string textureFileName = fileTexture->GetFileName();
		if (textureFileName.empty())
			continue;
		textureFileName = "Data/Models/Assets/" + textureFileName;

		if (textureType == FbxSurfaceMaterial::sDiffuse)
		{
			localMesh->material->m_diffuseMap = m_renderer->CreateOrGetTextureFromFile( textureFileName.c_str() );
		}
		else if (textureType == FbxSurfaceMaterial::sNormalMap)
		{
			localMesh->material->m_normalMap = m_renderer->CreateOrGetTextureFromFile( textureFileName.c_str() );
		}
		else if (textureType == FbxSurfaceMaterial::sSpecular)
		{
			localMesh->material->m_specGlossEmitMap = m_renderer->CreateOrGetTextureFromFile( textureFileName.c_str() );
		}
		else if (textureType == FbxSurfaceMaterial::sTransparencyFactor)
		{
			localMesh->material->m_transparencyMap = m_renderer->CreateOrGetTextureFromFile( textureFileName.c_str() );
		}
	}
}

FbxAMatrix FBX::GetGeometryTransformation( FbxNode* pNode )
{
	if (!pNode)
	{
		throw std::exception( "Null for mesh geometry" );
	}

	const FbxVector4 lT = pNode->GetGeometricTranslation( FbxNode::eSourcePivot );
	const FbxVector4 lR = pNode->GetGeometricRotation( FbxNode::eSourcePivot );
	const FbxVector4 lS = pNode->GetGeometricScaling( FbxNode::eSourcePivot );

	return FbxAMatrix( lT, lR, lS );
}

FbxAMatrix FBX::GetLocalTransformation( FbxNode* pNode )
{
	if (!pNode)
	{
		throw std::exception( "Null for mesh geometry" );
	}
	const FbxVector4 lT = pNode->LclTranslation.Get();
	const FbxVector4 lR = pNode->LclRotation.Get();
	const FbxVector4 lS = pNode->LclScaling.Get();
	//const FbxVector4 lS(1.0, 1.0, 1.0, 1.0);
	return FbxAMatrix( lT, lR, lS );
}

int FBX::FindJointIndexByName( std::string name )
{
	for (int i = 0; i < m_skeleton.m_joints.size(); i++)
	{
		if (m_skeleton.m_joints[i].m_name == name)
		{
			return i;
		}
	}
	return -1;
}
