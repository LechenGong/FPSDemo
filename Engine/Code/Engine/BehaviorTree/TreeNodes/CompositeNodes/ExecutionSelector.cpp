#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionSelector.h"
#include "Engine/General/Character.hpp"
#include "Engine/General/Controller.hpp"

ExecutionSelector::ExecutionSelector( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: CompositeNode( bt, selfIndex, parentIndex, location )
{

}

NodeStatus ExecutionSelector::InternalTick(const BehaviorTree::Context* btContext)
{
	int runningChildIndex = 0;
	for (int i = 0; i < m_children.size(); i++)
	{
		if (btContext->chara->m_controller->GetNodeStates()[m_children[i]->GetIndex()] == NodeStatus::RUNNING)
		{
			runningChildIndex = i;
			break;
		}
	}

	for (int i = runningChildIndex; i < m_children.size(); i++)
	{
		TreeNode* child = m_children[i];
		NodeStatus status = child->Execute( btContext );

		if (status == NodeStatus::RUNNING)
		{
			return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = NodeStatus::RUNNING;
		}
		else if (status == NodeStatus::SUCCESS)
		{
			return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = NodeStatus::SUCCESS;
		}
		else if (status == NodeStatus::BREAKING)
		{
			return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = (btContext->chara->m_controller->GetNodeStates()[GetIndex()] == NodeStatus::INVALID) ? NodeStatus::BREAKING : btContext->chara->m_controller->GetNodeStates()[GetIndex()];
		}
	}
	return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = NodeStatus::FAILURE;
}

std::string ExecutionSelector::GetTypeName() const
{
	return "Selector";
}

