#include "Engine/General/CharacterMovementComponent.hpp"

#include "Engine/General/Character.hpp"
#include "Engine/Math/MathUtils.hpp"

CharacterMovementComponent::CharacterMovementComponent( Character* owner )
	: m_owner( owner )
{
}

void CharacterMovementComponent::Update( float deltaSeconds )
{
	if (m_stepState.isStepping)
	{
		UpdateStepping( deltaSeconds );
	}
}

void CharacterMovementComponent::MoveWithCollision( Vec3 const& worldDisplacement )
{
	if (!m_owner || worldDisplacement == Vec3::ZERO)
	{
		return;
	}
	if (m_stepState.isStepping)
	{
		Vec3 horizontalDisplacement( worldDisplacement.x, worldDisplacement.y, 0.f );
		if (horizontalDisplacement != Vec3::ZERO)
		{
			m_owner->SetActorWorldPosition( m_owner->GetActorWorldPosition() + horizontalDisplacement );
		}
		m_owner->m_isGrounded = true;
		m_owner->m_velocity.z = 0.f;
		return;
	}

	m_preMoveWorldPosition = m_owner->GetActorWorldPosition();
	m_requestedMovementDelta = worldDisplacement;
	m_wasGroundedBeforeMove = m_owner->m_isGrounded;
	m_isResolvingMovement = true;

	m_owner->SetActorWorldPosition( m_preMoveWorldPosition + worldDisplacement );
	m_owner->ComponentCollisionCheck();

	m_isResolvingMovement = false;
	m_requestedMovementDelta = Vec3::ZERO;
}

bool CharacterMovementComponent::TryStartStepFromBlockedMove()
{
	Vec3 targetPos;
	if (!FindStepTarget( targetPos ))
	{
		return false;
	}

	StartStep( targetPos );
	return true;
}

bool CharacterMovementComponent::IsStepping() const
{
	return m_stepState.isStepping;
}

CharacterStepConfig& CharacterMovementComponent::GetStepConfig()
{
	return m_stepConfig;
}

CharacterStepState const& CharacterMovementComponent::GetStepState() const
{
	return m_stepState;
}

bool CharacterMovementComponent::FindStepTarget( Vec3& outTargetPos ) const
{
	if (!m_owner || !m_isResolvingMovement)
	{
		return false;
	}

	Vec3 horizontalMove( m_requestedMovementDelta.x, m_requestedMovementDelta.y, 0.f );
	if (horizontalMove.GetLengthSquared() <= m_stepConfig.minStepMoveDistance || m_requestedMovementDelta.z < -0.05f)
	{
		return false;
	}
	if (!m_wasGroundedBeforeMove && m_owner->GetAboveGroundHeight() > 0.15f)
	{
		return false;
	}

	Vec3 horizontalMoveNormal = horizontalMove.GetNormalized();
	float stepForwardDistance = MAX( horizontalMove.GetLength(), m_stepConfig.forwardProbeDistance );
	Vec3 stepHorizontalMove = horizontalMoveNormal * stepForwardDistance;

	Vec3 raisedActorPos = m_preMoveWorldPosition + stepHorizontalMove + Vec3::UP * m_stepConfig.maxStepHeight;
	if (m_owner->DoesBoundingCollisionOverlapWorldAt( raisedActorPos ))
	{
		return false;
	}

	RaycastResult3D groundResult = m_owner->GetAboveGroundHeight( m_owner->GetGroundProbePositionAt( raisedActorPos ) );
	if (!groundResult.m_didImpact || groundResult.m_impactDist > m_stepConfig.maxStepHeight + m_stepConfig.stepDownSnapDistance)
	{
		return false;
	}

	float groundDot = DotProduct3D( groundResult.m_impactNormal.GetNormalized(), Vec3::UP );
	if (groundDot < CosDegrees( 50.f ))
	{
		return false;
	}

	Vec3 finalActorPos = raisedActorPos - Vec3::UP * groundResult.m_impactDist + Vec3::UP * 0.01f;
	float stepHeight = finalActorPos.z - m_preMoveWorldPosition.z;
	if (stepHeight < -m_stepConfig.stepDownSnapDistance || stepHeight > m_stepConfig.maxStepHeight + 0.02f)
	{
		return false;
	}
	if (m_owner->DoesBoundingCollisionOverlapWorldAt( finalActorPos ))
	{
		return false;
	}

	outTargetPos = finalActorPos;
	return true;
}

void CharacterMovementComponent::StartStep( Vec3 const& targetPos )
{
	if (!m_owner)
	{
		return;
	}

	m_stepState.isStepping = true;
	m_stepState.startPos = m_owner->GetActorWorldPosition();
	m_stepState.targetPos = Vec3( m_stepState.startPos.x, m_stepState.startPos.y, targetPos.z );
	m_stepState.timer = 0.f;

	m_owner->m_isGrounded = true;
	m_owner->m_velocity.z = 0.f;
}

void CharacterMovementComponent::UpdateStepping( float deltaSeconds )
{
	if (!m_owner || !m_stepState.isStepping)
	{
		return;
	}

	m_stepState.timer += deltaSeconds;
	float stepDuration = MAX( m_stepConfig.stepDuration, 0.001f );
	float t = ClampZeroToOne( m_stepState.timer / stepDuration );
	float smoothT = SmoothStep3( t );
	Vec3 currentPos = m_owner->GetActorWorldPosition();
	float stepZ = Interpolate( m_stepState.startPos.z, m_stepState.targetPos.z, smoothT );
	m_owner->SetActorWorldPosition( Vec3( currentPos.x, currentPos.y, stepZ ) );
	m_owner->m_isGrounded = true;
	m_owner->m_velocity.z = 0.f;

	if (t >= 1.f)
	{
		m_stepState.isStepping = false;
		currentPos = m_owner->GetActorWorldPosition();
		m_owner->SetActorWorldPosition( Vec3( currentPos.x, currentPos.y, m_stepState.targetPos.z ) );
	}
}
