#pragma once

#include "Engine/Animation/AnimationStateMachine.hpp"

class Character;

class AnimationController
{
public:
	AnimationController();
	AnimationController( Character* character );
	~AnimationController();

	virtual void Update( float deltaSeconds );

	virtual AnimationStateMachine* GetStateMachine();
	virtual Character* GetCharaRef();

protected:
	AnimationStateMachine* m_stateMachine = nullptr;
	Character* m_charaRef = nullptr;
};
