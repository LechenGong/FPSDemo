#pragma once

#include <string>

class AnimationSequence;

class AnimationState
{
public:
	AnimationState( AnimationSequence* animationSequence, std::string name );
	~AnimationState();

	virtual void Update( float deltaSeconds );

	AnimationSequence* GetAnimation() const;
	std::string GetStateName() const;

protected:
	AnimationSequence* m_animationSequence = nullptr;
	std::string m_stateName;
};
