#include "Engine/BehaviorTree/TreeNodes/LeafNodes/LeafNode.h"

LeafNode::LeafNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: TreeNode( bt, selfIndex, parentIndex, location )
{

}

void LeafNode::ParseDataFromXml( [[maybe_unused]] XmlElement* xmlElement )
{
}

void LeafNode::ExportAttributeToXml( [[maybe_unused]] XmlElement* xmlElement )
{
}

void LeafNode::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
}

NodeStatus LeafNode::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	return NodeStatus();
}

void LeafNode::InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
}

std::string LeafNode::GetTypeName() const
{
	return m_typeName;
}

