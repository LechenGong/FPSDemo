#include "Engine/BehaviorTree/TreeNodes/TreeNode.h"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/General/Controller.hpp"

TreeNode::TreeNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
{
	m_index = selfIndex;
	m_btRef = bt;

	if (parentIndex == -1)
	{
		m_btRef->m_rootNode = this;
	}
	else
	{
		m_parent = bt->Find( parentIndex );

		if (m_parent)
		{
			m_parent->AddChild( this, location );
		}
	}
}

TreeNode::~TreeNode()
{
	for (int i = 0; i < m_children.size(); i++)
	{
		delete m_children[i];
		m_children[i] = nullptr;
	}
}

void TreeNode::ParseDataFromXml( XmlElement* xmlElement )
{
	UNUSED( xmlElement );
}

void TreeNode::ExportAttributeToXml( XmlElement* xmlElement )
{
	UNUSED( xmlElement );
}

NodeStatus TreeNode::Execute( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	Controller*& charaController = btContext->chara->m_controller;

	if (!m_btRef->IsBreaking( btContext ))
	{
		if (m_hasBreakpoint)
		{
			charaController->GetBreakingNode() = this;
			return NodeStatus::BREAKING;
		}

		if (charaController->GetNodeStates()[m_index] != NodeStatus::RUNNING)
		{
			InternalSpawn( btContext );
		}
		charaController->GetNodeStates()[m_index] = this->InternalTick( btContext );

		std::string message = "SetStatus " + std::to_string(m_index) + " " + std::to_string((int)charaController->GetNodeStates()[m_index]);
		g_netSystem->AddToSendQueue( message.c_str() );

		if (charaController->GetNodeStates()[m_index] != NodeStatus::RUNNING)
		{
			InternalTerminate( btContext );
		}

		return charaController->GetNodeStates()[m_index];
	}
	else
	{
		if (charaController->GetBreakingNode() == this)
		{
			charaController->GetBreakingNode() = nullptr;

			if (charaController->GetNodeStates()[m_index] != NodeStatus::RUNNING)
			{
				InternalSpawn( btContext );
			}
			charaController->GetNodeStates()[m_index] = InternalTick( btContext );

			std::string message = "SetStatus " + std::to_string(m_index) + " " + std::to_string((int)charaController->GetNodeStates()[m_index]);
			g_netSystem->AddToSendQueue( message.c_str() );

			if (charaController->GetNodeStates()[m_index] != NodeStatus::RUNNING)
			{
				InternalTerminate( btContext );
			}

			return charaController->GetNodeStates()[m_index];
		}
		else
		{
			charaController->GetNodeStates()[m_index] = InternalTick( btContext );

			std::string message = "SetStatus " + std::to_string(m_index) + " " + std::to_string((int)charaController->GetNodeStates()[m_index]);
			g_netSystem->AddToSendQueue( message.c_str() );

			return charaController->GetNodeStates()[m_index];
		}
	}
}

std::string TreeNode::GetTypeName() const
{
	return "";
}

void TreeNode::AddChild( TreeNode* child, int location )
{
	if (location == -1)
		location = (int)m_children.size();
	m_children.insert( m_children.begin() + location, child );
}

void TreeNode::RemoveChild( TreeNode* child )
{
	int location = -1;
	for (int i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == child)
		{
			location = i;
			break;
		}
	}

	if (location != -1)
	{
		m_children.erase( m_children.begin() + location );
	}
}

std::vector<TreeNode*> TreeNode::GetChildren() const
{
	return m_children;
}

TreeNode* TreeNode::GetParent()
{
	return m_parent;
}

int TreeNode::GetLeafCount() const
{
	if (m_children.size() == 0)
	{
		return 1;
	}
	int count = 0;
	for (int i = 0; i < m_children.size(); i++)
	{
		count += m_children[i]->GetLeafCount();
	}
	return count;
}

TreeNode* TreeNode::Find( int index )
{
	if (m_index == index)
		return this;

	for (auto child : m_children)
	{
		TreeNode* result = child->Find( index );
		if (result != nullptr)
			return result;
	}
	return nullptr;
}

int TreeNode::GetIndex() const
{
	return m_index;
}

void TreeNode::ResetStatus( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	Controller*& charaController = btContext->chara->m_controller;

	charaController->GetNodeStates()[m_index] = NodeStatus::INVALID;

	std::string message = "SetStatus " + std::to_string( m_index ) + " " + std::to_string( (int)NodeStatus::INVALID );
	g_netSystem->AddToSendQueue( message.c_str() );

	for (auto child : m_children)
	{
		child->ResetStatus( btContext );
	}
}

bool TreeNode::IsMouseOver( Vec2 mousePos ) const
{
	return mousePos.x >= minX && mousePos.x <= maxX && mousePos.y >= minY && mousePos.y <= maxY;
}

void TreeNode::AddVertsForTreeNode( std::vector<Vertex_PCU>& verts ) const
{
// 	float minX = 50.f + 300.f * (float)x;
// 	float maxX = minX + 200.f;
// 	float maxY = dimension.y - 50.f - 225.f * (float)y;
// 	float minY = maxY - 150.f;
	Rgba8 color = Rgba8::GRAY;
	switch (m_status)
	{
	case NodeStatus::SUCCESS:
		color = Rgba8::GREEN;
		break;
	case NodeStatus::FAILURE:
		color = Rgba8::RED;
		break;
	case NodeStatus::RUNNING:
		color = Rgba8::YELLOW;
		break;
	case NodeStatus::BREAKING:
		color = Rgba8::WHITE;
		break;
	case NodeStatus::INVALID:
		color = Rgba8::GRAY;
		break;
	default:
		break;
	}
	if (!m_isMouseOver)
		AddVertsForAABB2D( verts, AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) ), Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
	else
		AddVertsForAABB2D( verts, AABB2( Vec2( minX - 3.f, minY - 3.f ), Vec2( maxX + 3.f, maxY + 3.f ) ), Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
	AddVertsForAABB2D( verts, AABB2( Vec2( minX + 2.f, minY + 2.f ), Vec2( maxX - 2.f, maxY - 2.f ) ), Rgba8( 100, 100, 100, 255 ), Vec2::ZERO, Vec2::ONE );
	AddVertsForAABB2D( verts, AABB2( Vec2( minX + 2.f + 25.f, maxY - 2.f - 2.f - 30.f - 30.f ), Vec2( maxX - 2.f - 25.f, maxY - 2.f - 2.f - 30.f ) ), color, Vec2::ZERO, Vec2::ONE );
	
	if (m_parent)
		AddVertsForArrow2D( verts, m_parent->GetOutCoord(), GetInCoord(), 10.f, 2.f, Rgba8::WHITE );
// 	if (m_hasBreakpoint)
// 	{
// 		AddVertsForDisc2D( verts, Vec2( maxX, maxY ), 20.f, Rgba8::RED );
// 		if (btContext->chara->m_controller->m_breakingNode == this)
// 		{
// 			AddVertsForArrow2D( verts, Vec2( maxX, maxY ) - Vec2( 10.f, 0.f ), Vec2( maxX, maxY ) + Vec2( 10.f, 0.f ), 8.f, 2.f, Rgba8::YELLOW );
// 		}
// 	}
}

void TreeNode::AddTextForTreeNode( std::vector<Vertex_PCU>& verts, BitmapFont* font ) const
{
// 	float minX = 50.f + 300.f * (float)x;
// 	float maxX = minX + 200.f;
// 	float maxY = dimension.y - 50.f - 225.f * (float)y;
	font->AddVertsForTextInBox2D( verts, AABB2( Vec2( minX + 2.f + 5.f, maxY - 2.f - 2.f - 30.f ), Vec2( maxX - 2.f - 5.f, maxY - 2.f - 2.f ) ), 20.f, std::to_string( GetIndex() ) + "." + GetTypeName(),
		Rgba8::BLACK );
}

Vec2 TreeNode::GetInCoord() const
{
// 	float minX = 50.f + 300.f * (float)x;
// 	float maxY = dimension.y - 50.f - 225.f * (float)y;
// 	float minY = maxY - 150.f;
	return Vec2( minX, (minY + maxY) * 0.5f );
}

Vec2 TreeNode::GetOutCoord() const
{
// 	float minX = 50.f + 300.f * (float)x;
// 	float maxX = minX + 200.f;
// 	float maxY = dimension.y - 50.f - 225.f * (float)y;
// 	float minY = maxY - 150.f;
	return Vec2( maxX, (minY + maxY) * 0.5f );
}
