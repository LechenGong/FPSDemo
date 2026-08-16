#pragma once

#include <string>
#include <unordered_map>

class AnimationSequence;
class AnimationState;
class AnimationController;

class AnimationStateMachine
{
public:
	AnimationStateMachine( AnimationController* animController );

	void InitializeState( std::string name, std::unordered_map<std::string, std::unordered_map<std::string, AnimationState*>> const& animationMap );
	void AddState( AnimationState* newState );
	void SetInitialState( std::string const& stateName, int index = 0 );
	void Update( float deltaSeconds );

protected:
	AnimationController* m_parent = nullptr;

	std::unordered_map<std::string, AnimationState*> m_states;

public:
	struct StateSet
	{
		AnimationStateMachine* stateMachineRef = nullptr;

		AnimationState* currentState = nullptr;
		float currentAnimationTimer = 0.f;
		int currentLoopCount = 0;

		AnimationState* previousState = nullptr;
		float previousAnimationTimer = 0.f;
		bool previousInterrupted = false;
		float crossfadeRemainTimer = 0.f;
		float crossfadeFullDuration = 0.f;

		std::string rootJoint = "";
		float blendRemainTimer = 0.f;
		float blendFullDuration = 0.f;
		float blendAlpha = 0.f;

		std::unordered_map<int, bool> m_eventTriggered;

		void Update( float deltaSeconds );
		void TransitTo( std::string const& stateName, float duration = 0.1f, bool interrupting = false, float startTime = 0.f );

		AnimationState* GetStateByName( std::string const& stateName ) const;

		AnimationState* GetCurrentState() const;
		float GetCurrentAnimationPlaybackTime() const;

		AnimationState* GetPreviousState() const;
		float GetPreviousAnimationPlaybackTime() const;
		float GetCrossfadeRemainTime() const;
		float GetCrossfadeAlpha() const; // Alpha is 0 on begin, 1 on end

		bool HasCurrentAnimationEnded() const;
		bool HasCurrentAnimationReadyForCrossfade( float crossfadeDuration ) const;
		bool HasPreviousAnimationEnded() const;

		int GetCurrentLoopCount() const;

		void CheckForEvents( float time, bool transitting = false );
		void ResetEvents();

		void Nullify( float deltaSeconds, bool instant );
		void Validate( float deltaSeconds, bool instant );
	};
	
protected:
	std::vector<StateSet> m_ongoingAnimations;

public:
	std::vector<StateSet> &GetOngoingAnimations();
	StateSet& GetOngoingAnimation( int index );
};
