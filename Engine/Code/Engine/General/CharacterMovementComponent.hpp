#pragma once

#include "Engine/Math/RaycastUtil.hpp"
#include "Engine/Math/Vec3.hpp"

class Character;

struct CharacterStepConfig
{
	float maxStepHeight = 1.1f;
	float stepDownSnapDistance = 0.15f;
	float forwardProbeDistance = 0.45f;
	float stepDuration = 0.15f;
	float minStepMoveDistance = 0.0001f;
};

struct CharacterStepState
{
	bool isStepping = false;
	Vec3 startPos = Vec3::ZERO;
	Vec3 targetPos = Vec3::ZERO;
	float timer = 0.f;
};

class CharacterMovementComponent
{
public:
	CharacterMovementComponent( Character* owner );

	void Update( float deltaSeconds );
	void MoveWithCollision( Vec3 const& worldDisplacement );
	bool TryStartStepFromBlockedMove();

	bool IsStepping() const;
	CharacterStepConfig& GetStepConfig();
	CharacterStepState const& GetStepState() const;

private:
	bool FindStepTarget( Vec3& outTargetPos ) const;
	void StartStep( Vec3 const& targetPos );
	void UpdateStepping( float deltaSeconds );

private:
	Character* m_owner = nullptr;
	Vec3 m_preMoveWorldPosition = Vec3::ZERO;
	Vec3 m_requestedMovementDelta = Vec3::ZERO;
	bool m_wasGroundedBeforeMove = false;
	bool m_isResolvingMovement = false;

	CharacterStepConfig m_stepConfig;
	CharacterStepState m_stepState;
};
