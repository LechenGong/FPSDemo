#include "Engine/Animation/AnimationController.hpp"
#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/General/Controller.hpp"

AnimationController::AnimationController()
{
}

AnimationController::AnimationController( Character* character )
	:m_charaRef( character )
{
	m_stateMachine = new AnimationStateMachine( this );
}

AnimationController::~AnimationController()
{
}

void AnimationController::Update( float deltaSeconds )
{
	if (!m_stateMachine)
		return;

	m_stateMachine->Update( deltaSeconds );

	//std::string currentStateName = m_stateMachine->GetCurrentState()->GetStateName();
}

AnimationStateMachine* AnimationController::GetStateMachine()
{
	return m_stateMachine;
}

Character* AnimationController::GetCharaRef()
{
	return m_charaRef;
}
