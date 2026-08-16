#pragma once

#include <vector>
#include <string>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include "Engine/BehaviorTree/BehaviorTree.h"

enum class NodeStatus
{
	SUCCESS,
	FAILURE,
	RUNNING,
	BREAKING,
	INVALID
};

class BitmapFont;

class TreeNode
{
	friend class BehaviorTree;
public:
	TreeNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );
	virtual ~TreeNode();

	virtual void ParseDataFromXml( XmlElement* xmlElement );
	virtual void ExportAttributeToXml( XmlElement* xmlElement );

	NodeStatus Execute( [[maybe_unused]] const BehaviorTree::Context* btContext );
	virtual void InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext ) = 0;
	virtual NodeStatus InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext ) = 0;
	virtual void InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext ) = 0;

	virtual std::string GetTypeName() const;

	void AddChild( TreeNode* child, int location = -1 );
	void RemoveChild( TreeNode* child );
	std::vector<TreeNode*> GetChildren() const;
	TreeNode* GetParent();

	int GetLeafCount() const;

	TreeNode* Find( int index );
	int GetIndex() const;

	void ResetStatus( [[maybe_unused]] const BehaviorTree::Context* btContext );
	void SetStatus( NodeStatus status ) { m_status = status; }
	NodeStatus GetStatus() const { return m_status; }

public:
	int x, y;
	float minX, maxX, maxY, minY;
	bool m_isMouseOver = false;

	bool m_hasBreakpoint = false;

	bool IsMouseOver( Vec2 mousePos ) const;

protected:
	int m_index;
	BehaviorTree* m_btRef = nullptr;
	TreeNode* m_parent = nullptr;
	std::vector<TreeNode*> m_children;
	NodeStatus m_status = NodeStatus::INVALID;

public:
	void AddVertsForTreeNode( std::vector<Vertex_PCU>& verts ) const;
	void AddTextForTreeNode( std::vector<Vertex_PCU>& verts, BitmapFont* font ) const;

	Vec2 GetInCoord() const;
	Vec2 GetOutCoord() const;
};
