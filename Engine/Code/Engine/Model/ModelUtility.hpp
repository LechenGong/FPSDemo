#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"

struct BlendingIndexWeightPair
{
	unsigned int m_blendingIndex;
	double m_blendingWeight;

	BlendingIndexWeightPair()
		: m_blendingIndex( 0 )
		, m_blendingWeight( 0 )
	{}
};

struct ControlPoint
{
	ControlPoint()
	{
		m_blendingInfo.reserve( 4 );
	}

	Vec3 m_position;
	std::vector<BlendingIndexWeightPair> m_blendingInfo;
};

struct KeyFrame
{
	~KeyFrame()
	{
		delete m_next;
	}
	int64_t m_frameNum;
	Mat44 m_globalTransform;
	KeyFrame* m_next = nullptr;
};

struct Joint
{
	Joint()
		: m_parentIndex( -1 )
		, m_keyFrameBegin( nullptr )
		//, m_node( nullptr )
	{
		//m_globalBindposeInverse.SetIdentity();
	}

	~Joint()
	{
	}

	std::string m_name;
	Mat44 m_globalBindposeInverse;
	KeyFrame* m_keyFrameBegin;
	int m_parentIndex;
	std::vector<int> m_childrenIndexes;
};

struct Skeleton
{
	std::vector<Joint> m_joints;

	int GetJointIndexByName( std::string name );
	std::unordered_map<int, bool> GetChildrenfromJoint( std::string name );
};

struct Triangle
{
	std::vector<unsigned int> m_indexes;
	std::string m_materialName;
	unsigned int m_materialIndex;

	bool operator < ( Triangle const& compareTo )
	{
		return m_materialIndex < compareTo.m_materialIndex;
	}
};
