#include "Engine/General/Controller.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"

Controller::Controller( Character* charaRef )
{
	Possess( charaRef );
}

Controller::~Controller()
{
	Unpossess();
}

void Controller::Update( float deltaSeconds )
{
	TickBehaviorTree( deltaSeconds );
}

void Controller::Possess( Character* charaRef )
{
	if (charaRef)
	{
		Unpossess();
		charaRef->OnPossessed( this );
	}
}

void Controller::Unpossess()
{
	if (m_possessedChara)
	{
		m_possessedChara->OnUnpossessed();
	}
}

void Controller::TickBehaviorTree( [[maybe_unused]] float deltaSeconds )
{
	if (!m_btTree || !m_possessedChara)
		return;

	if (m_nodeStates.size() < m_btTree->GetSize())
	{
		m_nodeStates.resize( m_btTree->GetSize(), NodeStatus::INVALID );
	}

	BehaviorTree::Context btContext;
	btContext.chara = m_possessedChara;
	m_btTree->Execute( &btContext );
}
