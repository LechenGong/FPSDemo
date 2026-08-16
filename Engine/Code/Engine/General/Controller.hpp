#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/BehaviorTree/Blackboard.h"
#include "Engine/BehaviorTree/TreeNodes/TreeNode.h"

class Character;
class BehaviorTree;

class Controller
{
public:
	Controller() = default;
	Controller( Character* charaRef );
	~Controller();


	virtual void InputResponse( float deltaSeconds ) = 0;
	virtual void Update( float deltaSeconds );
	virtual void Possess( Character* charaRef );
	virtual void Unpossess();

	virtual void TickBehaviorTree( float deltaSeconds );

	Character* m_possessedChara = nullptr;

public:
	virtual bool IsSprinting() const = 0;
	virtual bool IsImpact() const = 0;
	virtual bool IsTurning() const = 0;
	virtual bool IsFalling() const = 0;
	virtual bool IsDying() const = 0;
	virtual bool IsAttemptingToMove() const = 0;
	virtual void ResetAllState() = 0;
	virtual bool IsMovementHindered() const = 0;
	virtual Vec3 GetMoveDirection() const = 0;

public:
	virtual void SetIsFalling( bool flag ) = 0;

public:
	Blackboard m_blackboard;

	BehaviorTree*& GetBehaviorTree() { return m_btTree; }
	TreeNode*& GetLastRunningNode() { return m_lastRunningNode; }
	TreeNode*& GetBreakingNode() { return m_breakingNode; }
	std::vector<NodeStatus>& GetNodeStates() { return m_nodeStates; }
	bool& IsBTFinished() { return m_isBTFinished; }
	float& GetTimeSpentOnNode() { return m_timeSpentOnNode; }

protected:
	BehaviorTree* m_btTree = nullptr;
	TreeNode* m_lastRunningNode = nullptr;
	TreeNode* m_breakingNode = nullptr;
	std::vector<NodeStatus> m_nodeStates;
	bool m_isBTFinished = false;
	float m_timeSpentOnNode = 0.f;

protected:
	bool m_isImpact = false;
	bool m_isSprinting = false;
	bool m_isTurning = false;
	bool m_isFalling = false;
	bool m_isDying = false;
	Vec3 m_moveDirection = Vec3::ZERO;
};