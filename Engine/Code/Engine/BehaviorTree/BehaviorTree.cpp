#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/BehaviorTree/Blackboard.h"
#include "Engine/BehaviorTree/TreeNodes/TreeNode.h"
#include "Engine/BehaviorTree/TreeNodes/TreeNodeFactory.h"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/General/Controller.hpp"

BehaviorTree::BehaviorTree( Clock* clock )
{
	m_clock = new Clock( *clock );
}

void BehaviorTree::ExportToXML( std::string filePath )
{
	XmlDocument doc;
	XmlElement* rootElement = doc.NewElement( "BehaviorTree" );
	doc.InsertFirstChild( rootElement );

	rootElement->SetAttribute( "Name", m_name.c_str() );

	ExportNodeToXML( m_rootNode, doc, rootElement );

	if (doc.SaveFile( filePath.c_str() ) != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE( Stringf( "Can not export %s to XML", m_name.c_str() ).c_str() );
	}
}

void BehaviorTree::ExportNodeToXML( TreeNode* node, XmlDocument& doc, XmlElement* parentElement )
{
	if (!node)
		return;

	XmlElement* nodeElement = doc.NewElement( "TreeNode" );

	nodeElement->SetAttribute( "Index", node->GetIndex() );
	nodeElement->SetAttribute( "Parent", node->GetParent() ? node->GetParent()->GetIndex() : -1 );
	nodeElement->SetAttribute( "Type", node->GetTypeName().c_str() );
	node->ExportAttributeToXml( nodeElement );

	parentElement->InsertEndChild( nodeElement );

	for (TreeNode* treeNode : node->GetChildren())
	{
		ExportNodeToXML( treeNode, doc, nodeElement );
	}
}

void BehaviorTree::ImportFromXML( std::string filePath )
{
	XmlDocument doc;
	if (doc.LoadFile( filePath.c_str() ) != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE( Stringf( "Can not load %s", filePath.c_str() ).c_str() );
	}

	XmlElement* rootElement = doc.FirstChildElement();

	if (!rootElement)
	{
		ERROR_AND_DIE( Stringf( "Invalid XML: Missing Behavior Tree Element" ).c_str() );
	}

	m_name = ParseXmlAttribute( *rootElement, "Name", "" );

	ImportNodeFromXML( rootElement->FirstChildElement( "TreeNode" ), nullptr );
}

void BehaviorTree::ImportNodeFromXML( XmlElement* nodeElement, TreeNode* parentNode )
{
	UNUSED( parentNode );

	if (!nodeElement)
		return;

	int index = ParseXmlAttribute( *nodeElement, "Index", -1 );
	int parentIndex = ParseXmlAttribute( *nodeElement, "Parent", -1 );
	std::string typeName = ParseXmlAttribute( *nodeElement, "Type", "Selector" );

	TreeNode* newNode = TreeNodeFactory::CreateNode( typeName, this, index, parentIndex );
	m_size++;
	newNode->ParseDataFromXml( nodeElement );

	if (parentIndex == -1)
	{
		newNode->x = 0;
		newNode->y = 0;
	}
	else
	{
		newNode->x = parentNode->x + 1;
		int totalLeafCount = parentNode->y;
		for (TreeNode* child : parentNode->GetChildren())
		{
			if (child == newNode)
				break;
			totalLeafCount += child->GetLeafCount();
		}
	}

	if (newNode)
	{
		XmlElement* childElement = nodeElement->FirstChildElement( "TreeNode" );
		while (childElement)
		{
			ImportNodeFromXML( childElement, newNode );
			childElement = childElement->NextSiblingElement( "TreeNode" );
		}
	}
}

void BehaviorTree::ResetAllNodeStatus( [[maybe_unused]] Context const* btContext )
{
	m_rootNode->ResetStatus( btContext );
	btContext->chara->m_controller->GetLastRunningNode() = nullptr;
	btContext->chara->m_controller->GetBreakingNode() = nullptr;
}

TreeNode* BehaviorTree::Find( int index )
{
	if (!m_rootNode)
		ERROR_AND_DIE( "Find conducted on empty behavior tree!" );

	return m_rootNode->Find( index );
}

void BehaviorTree::GenerateCoordinates( TreeNode* node, int& x, int& y )
{
	node->x = x;
	node->y = y;

	node->minX = 50.f + 300.f * (float)node->x;
	node->maxX = node->minX + 200.f;
	node->maxY = 800.f - 50.f - 225.f * (float)node->y;
	node->minY = node->maxY - 150.f;

	if (!node->GetChildren().empty())
	{
		x += 1;
		GenerateCoordinates( node->GetChildren()[0], x, y );
		x -= 1;
	}
	for (int i = 1; i < node->GetChildren().size(); i++)
	{
		x += 1;
		y += 1;
		GenerateCoordinates( node->GetChildren()[i], x, y );
		x -= 1;
	}
}

bool BehaviorTree::IsBreaking( [[maybe_unused]] Context const* btContext )
{
	return btContext->chara->m_controller->GetBreakingNode();
}

void BehaviorTree::Startup( [[maybe_unused]] Context const* btContext )
{
	if (!btContext->chara->m_controller->IsBTFinished())
		return;

	btContext->chara->m_controller->IsBTFinished() = false;
	ResetAllNodeStatus( btContext );
}

void BehaviorTree::Execute( [[maybe_unused]] Context const* btContext  )
{
	if (btContext->chara->m_controller->IsBTFinished())
	{
		btContext->chara->m_controller->IsBTFinished() = false;
	}

	NodeStatus status = m_rootNode->Execute( btContext );
	if (status != NodeStatus::RUNNING && status != NodeStatus::INVALID && status != NodeStatus::BREAKING)
	{
		Abort( btContext );
	}
}

void BehaviorTree::Abort( [[maybe_unused]] Context const* btContext ) 
{
	btContext->chara->m_controller->IsBTFinished() = true;
	ResetAllNodeStatus( btContext );
}

