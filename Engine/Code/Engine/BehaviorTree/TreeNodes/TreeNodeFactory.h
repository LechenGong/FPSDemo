#pragma once

#include <unordered_map>
#include <functional>
#include <string>

class TreeNode;
class BehaviorTree;

class TreeNodeFactory
{
public:
	using NodeCreator = std::function<TreeNode* ( BehaviorTree*, int, int, int)>;

	static std::unordered_map<std::string, NodeCreator> NodeCreators;

	static void RegisterNodeTypes();

	static void Register( std::string const& typeName, NodeCreator nodeCreator );

	static TreeNode* CreateNode( std::string typeName, BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );
};
