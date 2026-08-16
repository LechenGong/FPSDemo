#include "Engine/BehaviorTree/TreeNodes/TreeNodeFactory.h"

#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionSelector.h"
#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionSequence.h"
#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionRandomSelector.h"
#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionRandomSequence.h"

#include "Engine/BehaviorTree/TreeNodes/LeafNodes/ExecutionWait.h"

std::unordered_map<std::string, TreeNodeFactory::NodeCreator> TreeNodeFactory::NodeCreators;

void TreeNodeFactory::RegisterNodeTypes()
{
	NodeCreators["Selector"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionSelector( bt, selfIndex, parentIndex, location ); };

	NodeCreators["Sequence"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionSequence( bt, selfIndex, parentIndex, location ); };

	NodeCreators["RandomSelector"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionRandomSelector( bt, selfIndex, parentIndex, location ); };

	NodeCreators["RandomSequence"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionRandomSequence( bt, selfIndex, parentIndex, location ); };

	NodeCreators["Wait"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionWait( bt, selfIndex, parentIndex, location ); };
}

void TreeNodeFactory::Register( std::string const& typeName, NodeCreator nodeCreator )
{
	NodeCreators[typeName] = nodeCreator;
}

TreeNode* TreeNodeFactory::CreateNode( std::string typeName, BehaviorTree* bt, int selfIndex, int parentIndex, int location )
{
	auto iter = NodeCreators.find( typeName );
	if (iter != NodeCreators.end())
	{
		return iter->second( bt, selfIndex, parentIndex, location );
	}
	LeafNode* newLeafNode = new LeafNode( bt, selfIndex, parentIndex, location );
	newLeafNode->m_typeName = typeName;
	return newLeafNode;
}



