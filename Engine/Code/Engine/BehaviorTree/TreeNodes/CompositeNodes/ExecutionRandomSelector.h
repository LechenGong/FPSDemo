#pragma once

#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/CompositeNode.h"
#include "Engine/BehaviorTree/BehaviorTree.h"

class ExecutionRandomSelector : public CompositeNode
{
public:
	ExecutionRandomSelector( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );

	void InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	NodeStatus InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	void InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext ) override {};

	std::string GetTypeName() const override;
};