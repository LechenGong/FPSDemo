#include "Engine/Animation/AnimationState.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Core/EngineCommon.hpp"

AnimationState::AnimationState( AnimationSequence* animationSequence, std::string name )
	: m_animationSequence( animationSequence )
	, m_stateName( name )
{
}

AnimationState::~AnimationState()
{
}

void AnimationState::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

AnimationSequence* AnimationState::GetAnimation() const
{
	return m_animationSequence;
}

std::string AnimationState::GetStateName() const
{
	return m_stateName;
}
