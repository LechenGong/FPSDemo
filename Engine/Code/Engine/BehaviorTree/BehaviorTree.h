#pragma once

#include <string>

#include "Engine/Core/XmlUtils.hpp"

class TreeNode;
class Clock;
class Character;

class BehaviorTree
{
	friend class TreeNode;
public:
	BehaviorTree( Clock* clock );

	void ExportToXML( std::string filePath );
	void ImportFromXML( std::string filePath );

	TreeNode* Find( int index );
	TreeNode* GetRootNode() { return m_rootNode; };
	int GetSize() const { return m_size; }
	//void AddChild( TreeNode* newNode, int parentIndex, int location );

	void GenerateCoordinates( TreeNode* node, int& x, int& y );

public:
	struct Context
	{
		Character* chara = nullptr;
	};

	std::string m_name;
	TreeNode* m_rootNode = nullptr;
	TreeNode* m_lastRunningNode = nullptr;
	TreeNode* m_breakingNode = nullptr;
	Clock* m_clock = nullptr;

public:
	void Startup( Context const* btContext );
	void Execute( Context const* btContext );
	void Abort( Context const* btContext );

	bool IsBreaking( Context const* btContext );
	//bool m_finished = true;

protected:
	void ExportNodeToXML( TreeNode* node, XmlDocument& doc, XmlElement* parentElement );
	void ImportNodeFromXML( XmlElement* nodeElement, TreeNode* parentNode );

	void ResetAllNodeStatus( Context const* btContext );

	int m_size = 0;
};
