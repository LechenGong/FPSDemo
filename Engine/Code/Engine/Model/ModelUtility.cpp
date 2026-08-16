#include <queue>

#include "Engine/Model/ModelUtility.hpp"

int Skeleton::GetJointIndexByName( std::string name )
{
	int index = -1;
	for (int i = 0; i < m_joints.size(); i++)
	{
		if (m_joints[i].m_name == name)
		{
			index = i;
			break;
		}
	}
	return index;
}

std::unordered_map<int, bool> Skeleton::GetChildrenfromJoint( std::string name )
{
    std::unordered_map<int, bool> list;

    int index = -1;
    for (int i = 0; i < m_joints.size(); i++)
    {
        if (m_joints[i].m_name == name)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
        return list;

    std::queue<int> queue;
    queue.push( index );

	while (!queue.empty())
	{
		int current = queue.front();
		queue.pop();

		for (int childIndex : m_joints[current].m_childrenIndexes)
		{
            list[childIndex] = true;
			queue.push( childIndex );
		}
	}

	return list;
}
