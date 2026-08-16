#pragma once

#include "Engine/BehaviorTree/TreeNodes/TreeNode.h"

class LeafNode : public TreeNode
{
public:
	LeafNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );

	void ParseDataFromXml( XmlElement* xmlElement ) override;
	void ExportAttributeToXml( XmlElement* xmlElement ) override;

	void InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	NodeStatus InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	void InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;

	std::string GetTypeName() const override;

public:
	std::string m_typeName;
};
