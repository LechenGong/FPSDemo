#pragma once

#include "Engine/BehaviorTree/TreeNodes/TreeNode.h"

class CompositeNode : public TreeNode
{
public:
	CompositeNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );
};
