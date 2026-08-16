#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Animation/AnimationController.hpp"
#include "Engine/Core/EngineCommon.hpp"

AnimationStateMachine::AnimationStateMachine( AnimationController* animController )
{
	m_parent = animController;
	m_ongoingAnimations.resize( 2 );
	for (StateSet &animation : m_ongoingAnimations)
	{
		animation.stateMachineRef = this;
	}
}

void AnimationStateMachine::InitializeState( std::string name, std::unordered_map<std::string, std::unordered_map<std::string, AnimationState*>> const& animationMap )
{
	if (animationMap.find( name ) == animationMap.end())
		return;

	std::unordered_map<std::string, AnimationState*> const& animationStateList = animationMap.at( name );
	
	for (auto const& state : animationStateList)
	{
		AddState( state.second );
	}
}

void AnimationStateMachine::AddState( AnimationState* newState )
{
	std::string s = newState->GetStateName();
	auto a = m_states.find( s );
	if (m_states.find( newState->GetStateName() ) != m_states.end())
	{
		ERROR_AND_DIE( "Attempting to add state with same name" );
	}

	m_states[newState->GetStateName()] = newState;
}

void AnimationStateMachine::SetInitialState( std::string const& stateName, int index )
{
	auto iter = m_states.find( stateName );
	if (iter == m_states.end())
	{
		DebuggerPrintf( Stringf( "State with name %s doesn't exist", stateName.c_str() ).c_str() );
	}
	if (iter->second == nullptr)
	{
		ERROR_AND_DIE( "State is nullptr" );
	}

	m_ongoingAnimations[index].currentState = iter->second;

	m_ongoingAnimations[index].currentAnimationTimer = 0.f;
}

void AnimationStateMachine::Update( float deltaSeconds )
{
	for (StateSet &thisState : m_ongoingAnimations)
	{
		thisState.Update( deltaSeconds );
	}
}

std::vector<AnimationStateMachine::StateSet> &AnimationStateMachine::GetOngoingAnimations()
{
	return m_ongoingAnimations;
}

AnimationStateMachine::StateSet& AnimationStateMachine::GetOngoingAnimation( int index )
{
	return m_ongoingAnimations[index];
}

void AnimationStateMachine::StateSet::Update( float deltaSeconds )
{
	if (HasPreviousAnimationEnded())
	{
		previousState = nullptr;
	}

	if (currentState)
	{
		AnimationSequence* animation = currentState->GetAnimation();

		float playbackSpeed = animation->m_playbackSpeed;

		if (currentAnimationTimer + deltaSeconds * playbackSpeed >= animation->m_duration)
		{
			if (animation->m_looping)
			{
				currentAnimationTimer = currentAnimationTimer + deltaSeconds * playbackSpeed - animation->m_duration;
				CheckForEvents( animation->m_duration );
				CheckForEvents( currentAnimationTimer );
				currentLoopCount++;
			}
			else if (currentAnimationTimer < animation->m_duration)
			{
				currentAnimationTimer = animation->m_duration;
				CheckForEvents( animation->m_duration );
				ResetEvents();
			}
			else
			{
				if (this != &stateMachineRef->GetOngoingAnimation( 0 ))
				{
					Nullify( deltaSeconds, true );
				}
			}
		}
		else
		{
			if (this != &stateMachineRef->GetOngoingAnimation( 0 ))
			{
				if (currentAnimationTimer + blendFullDuration >= animation->m_duration)
				{
					Nullify( deltaSeconds, false );
				}
				else if (currentAnimationTimer <= blendFullDuration)
				{
					Validate( deltaSeconds, false );
				}
			}

			currentAnimationTimer += deltaSeconds * playbackSpeed;
			CheckForEvents( currentAnimationTimer );
		}
	}

	if (previousState && crossfadeRemainTimer > 0)
	{
		AnimationSequence* animation = previousState->GetAnimation();

		float playbackSpeed = animation->m_playbackSpeed;

		if (!previousInterrupted)
		{
			if (previousAnimationTimer + deltaSeconds * playbackSpeed >= animation->m_duration)
			{
				if (animation->m_looping)
				{
					previousAnimationTimer = previousAnimationTimer + deltaSeconds * playbackSpeed - animation->m_duration;
				}
				else
				{
					previousAnimationTimer = animation->m_duration;
				}
			}
			else
			{
				previousAnimationTimer += deltaSeconds * playbackSpeed;
			}
		}

		crossfadeRemainTimer -= deltaSeconds;
		crossfadeRemainTimer = MAX( crossfadeRemainTimer, 0 );
	}
}

void AnimationStateMachine::StateSet::TransitTo( std::string const& stateName, float duration, bool interrupting, float startTime )
{
 	auto iter = stateMachineRef->m_states.find( stateName );
	AnimationState* animationState;
	if (iter == stateMachineRef->m_states.end())
	{
		DebuggerPrintf( Stringf( "State with name %s doesn't exist", stateName.c_str() ).c_str() );
		animationState = nullptr;
	}
	else
	{
		animationState = iter->second;
	}

	if (currentState)
	{
		CheckForEvents( currentState->GetAnimation()->m_duration );
		ResetEvents();
		duration = MIN( duration, (currentState->GetAnimation()->m_duration - startTime) * currentState->GetAnimation()->m_playbackSpeed );
	}

	if (crossfadeRemainTimer == 0.f)
	{
		currentLoopCount = 0;

		previousState = currentState;
		previousAnimationTimer = currentAnimationTimer;
		previousInterrupted = interrupting;
		crossfadeRemainTimer = duration;
		crossfadeFullDuration = duration;

		currentState = animationState;
		currentAnimationTimer = startTime;
		CheckForEvents( startTime );
	}
	else
	{
		previousInterrupted = interrupting;
		crossfadeRemainTimer = duration - crossfadeFullDuration + crossfadeRemainTimer;
		crossfadeFullDuration = crossfadeRemainTimer;

		if (!previousState)
		{
			previousState = currentState;
			previousAnimationTimer = currentAnimationTimer;
		}

		currentLoopCount = 0;
		currentState = animationState;
		currentAnimationTimer = startTime;
		
		CheckForEvents( startTime );
	}
	
}

AnimationState* AnimationStateMachine::StateSet::GetStateByName( std::string const& stateName ) const
{
	auto iter = stateMachineRef->m_states.find( stateName );
	if (iter == stateMachineRef->m_states.end())
	{
		DebuggerPrintf( Stringf( "State with name %s doesn't exist", stateName.c_str() ).c_str() );
	}
	if (iter->second == nullptr)
	{
		ERROR_AND_DIE( "State is nullptr" );
	}
	return iter->second;
}

AnimationState* AnimationStateMachine::StateSet::GetCurrentState() const
{
// 	if (currentState == nullptr)
// 	{
// 		ERROR_AND_DIE( "Current state is nullptr (should be initialized)" );
// 	}

	return currentState;
}

float AnimationStateMachine::StateSet::GetCurrentAnimationPlaybackTime() const
{
	return currentAnimationTimer;
}

AnimationState* AnimationStateMachine::StateSet::GetPreviousState() const
{
	return previousState;
}

float AnimationStateMachine::StateSet::GetPreviousAnimationPlaybackTime() const
{
	return previousAnimationTimer;
}

float AnimationStateMachine::StateSet::GetCrossfadeRemainTime() const
{
	return crossfadeRemainTimer;
}

float AnimationStateMachine::StateSet::GetCrossfadeAlpha() const
{
	if (crossfadeFullDuration == 0.f)
		return 1.f;
	return (crossfadeFullDuration - crossfadeRemainTimer) / crossfadeFullDuration;
}

bool AnimationStateMachine::StateSet::HasCurrentAnimationEnded() const
{
	if (!currentState)
		ERROR_AND_DIE( "Current state is nullptr (should be initialized)" );

	if (currentState->GetAnimation()->m_looping)
		return false;

	return currentAnimationTimer >= currentState->GetAnimation()->m_duration;
}

bool AnimationStateMachine::StateSet::HasCurrentAnimationReadyForCrossfade( float crossfadeDuration ) const
{
	if (!currentState)
		ERROR_AND_DIE( "Current state is nullptr (should be initialized)" );

	if (currentState->GetAnimation()->m_looping)
		return true;

	return currentAnimationTimer >= currentState->GetAnimation()->m_duration - crossfadeDuration;
}

bool AnimationStateMachine::StateSet::HasPreviousAnimationEnded() const
{
	if (!previousState)
		return true;

	return crossfadeRemainTimer <= 0;
}

int AnimationStateMachine::StateSet::GetCurrentLoopCount() const
{
	if (!currentState)
		return 0;
	if (!currentState->GetAnimation())
		return 0;
	if (!currentState->GetAnimation()->m_looping)
		return 0;

	return currentLoopCount;
}

void AnimationStateMachine::StateSet::CheckForEvents( float time, bool transitting )
{
	if (currentState)
	{
		AnimationSequence* animation = currentState->GetAnimation();
		std::vector<AnimationEvent> events = animation->GetEvents();
		for (int i = 0; i < events.size(); i++)
		{
			if (m_eventTriggered.find( i ) != m_eventTriggered.end())
			{
				continue;
			}
			else
			{
				if (events[i].time <= time /*-epsilon*/)
				{
					if (!transitting || events[i].persisting) // if not transiting, fire regardless; if transiting, fire if persisting
					{
						g_eventSystem->FireEventEX( events[i].name, stateMachineRef->m_parent->GetCharaRef(), 
							(int)events[i].collisionIndex, (bool)events[i].flag, 
							&events[i].damageTypeIndex, &events[i].damageValue );
					}
					m_eventTriggered[i] = true;
				}
			}
		}
	}
}

void AnimationStateMachine::StateSet::ResetEvents()
{
	m_eventTriggered.clear();
}

void AnimationStateMachine::StateSet::Nullify( float deltaSeconds, bool instant )
{
	if (instant || blendFullDuration == 0.f)
	{
		blendAlpha = 0.f;
		return;
	}
	blendAlpha -= (1.f / blendFullDuration) * deltaSeconds;
	blendAlpha = Clamp( blendAlpha, 0.f, 1.f );
}

void AnimationStateMachine::StateSet::Validate( float deltaSeconds, bool instant )
{
	if (instant || blendFullDuration == 1.f)
	{
		blendAlpha = 1.f;
		return;
	}
	blendAlpha += (1.f / blendFullDuration) * deltaSeconds;
	blendAlpha = Clamp( blendAlpha, 0.f, 1.f );
}
