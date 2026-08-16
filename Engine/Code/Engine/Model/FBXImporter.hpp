#pragma once

// #define FBXSDK_SHARED
// #include "fbxsdk.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>

#include "Engine/Model/FBXUtility.hpp"
#include "Engine/Model/Vertex_Anim.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Model/ModelUtility.hpp"

class Renderer;
class Texture;
class VertexBuffer;
class IndexBuffer;
class AnimationSequence;
class MeshT;

class Character;

class FBX
{
public:
	struct RootMotionConfig
	{
		bool removeForward = false;
		bool removeLeftward = false;
		bool removeUpward = false;

		bool removeRotation = false;

		RootMotionConfig( bool pRemoveForward = false, bool pRemoveLeftward = false, bool pRemoveUpward = false, bool pRemoveRotation = false )
		{
			removeForward = pRemoveForward;
			removeLeftward = pRemoveLeftward;
			removeUpward = pRemoveUpward;
			removeRotation = pRemoveRotation;
		}
	};

	FBX() {};
	FBX( std::string filePath, Renderer* renderer, bool ignoreMaterial = false, bool ignoreAnimation = false, RootMotionConfig pRootMotion = RootMotionConfig() );
	~FBX();

// 	struct MeshT
// 	{
// 		std::string name;
// 		bool isVisible = true;
// 		std::vector<Vertex_Anim> vertexes;
// 		std::vector<unsigned int> indexes;
// 		VertexBuffer* vertexBuffer = nullptr;
// 		IndexBuffer* indexBuffer = nullptr;
// 		int indexesCount = 0;
// 		Texture* video = nullptr;
// 		Texture* diffuseMap = nullptr;
// 		Texture* transparencyMap = nullptr;
// 
// 		~MeshT();
// 	};

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void PlayPrevAnimation();
	void PlayNextAnimation();
	void PlayAnimation( int index );

	void SyncMaterialFrom( FBX* fbx );

	std::string GetCurrentAnimationName();

	std::string GetName() {
		size_t pos = m_filePath.find_last_of( '/' ) + 1;
		return m_filePath.substr( pos, m_filePath.length() - pos );
	}
	int GetAnimationCount() { return int( m_animationSequences.size() ); }
	std::string GetAnimationNameByIndex( int index );

protected:
	FbxManager* m_fbxManager = nullptr;
	FbxScene* m_fbxScene = nullptr;
	
private:
	bool Initialize();
	void InitializeAllTexture();
	bool LoadScene( char const* pSceneName );

	void LoadSkeleton( FbxNode* pNode, int myIndex, int parentIndex );
	void LoadNode( FbxNode* pNode );
	void LoadMesh( FbxNode* pNode );
	void LoadControlPoints( FbxNode* pNode );
	void LoadJointsAndAnimations( FbxNode* pNode );

	void ReadVertex( FbxMesh* pMesh, int ctrlPointIndex, Vec3& position );
	void ReadColor( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Rgba8& color );
	void ReadUVs( FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int UVLayer, Vec2& UVs );
	void ReadTangent( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& tangent );
	void ReadBinormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& binormal );
	void ReadNormal( FbxMesh* pMesh, int ctrlPointIndex, int vertexCount, Vec3& normal );

	void LoadMaterial( MeshT* localMesh, FbxMesh* pMesh );
	void LoadMaterialAttribute( MeshT* localMesh, FbxSurfaceMaterial* pSurfaceMaterial );
	void LoadMaterialTexture( MeshT* localMesh, FbxSurfaceMaterial* pSurfaceMaterial, const char* textureType );

	FbxAMatrix GetGeometryTransformation( FbxNode* pNode );
	FbxAMatrix GetLocalTransformation( FbxNode* pNode );

	int FindJointIndexByName( std::string name );

public:
	Renderer* m_renderer = nullptr;
	std::string m_filePath;
	std::vector<MeshT*> m_meshes;
	std::map<std::string, Texture*> m_textures;
	RootMotionConfig m_rootMotionConfig;

	Skeleton m_skeleton;
	std::unordered_map<unsigned int, ControlPoint*> m_controlPoints; 

	std::vector<AnimationSequence*> m_animationSequences;
	AnimationSequence* m_currentAnimation = nullptr;
	int m_animationIndex = 0;
	//float m_scale = 1.f;

	bool m_isLooping = true;
	bool m_ignoreMaterial = false;
	bool m_ignoreAnimation = false;
};
