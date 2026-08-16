#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/CompositeNode.h"

CompositeNode::CompositeNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: TreeNode( bt, selfIndex, parentIndex, location )
{

}

