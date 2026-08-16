#include "Game/AIController.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"

#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <cmath>

namespace
{
	constexpr float AI_REPATH_INTERVAL = 0.35f;
	constexpr float AI_REPATH_GOAL_MOVE_THRESHOLD = 1.25f;
	constexpr float AI_ARRIVE_RADIUS = 0.3f;
	constexpr float AI_IDLE_SPEED_EPSILON = 0.05f;
	constexpr bool AI_DEBUG_DRAW_PATH = true;

	// Continuous lateral preference (soft lane across portal width).
	constexpr float AI_PORTAL_BIAS_MIN_WIDTH = 1.2f;	// ignore narrow doors
	constexpr float AI_PORTAL_SNAP_DISTANCE = 1.75f;	// waypoint/edge near portal to tweak
	constexpr float AI_LATERAL_EDGE_MARGIN = 0.08f;	// keep off exact portal endpoints
	constexpr float AI_LATERAL_JITTER_RANGE = 0.05f;	// tiny repath noise, not discrete lanes
	constexpr float AI_GOLDEN_FRAC = 0.6180339887f;

	// Weak flock separation.
	constexpr float AI_SEP_RADIUS = 0.85f;
	constexpr float AI_SEP_SCALE = 0.55f;
	constexpr float AI_SEP_SCALE_ON_SLOPE = 0.30f;

	// Capsule contact sits near radius+radius; melee reach already includes both radii.
}

float AIController::GetDistanceXY( Vec3 const& a, Vec3 const& b )
{
	float const dx = a.x - b.x;
	float const dy = a.y - b.y;
	return sqrtf( dx * dx + dy * dy );
}

float AIController::DistPointToSegment3D( Vec3 const& point, Vec3 const& a, Vec3 const& b )
{
	Vec3 ab = b - a;
	float abLenSq = ab.GetLengthSquared();
	if (abLenSq < 0.000001f)
	{
		return (point - a).GetLength();
	}
	float t = DotProduct3D( point - a, ab ) / abLenSq;
	t = Clamp( t, 0.f, 1.f );
	Vec3 closest = a + ab * t;
	return (point - closest).GetLength();
}

void AIController::ClearPath()
{
	m_path.clear();
	m_pathIndex = 0;
}

void AIController::StopHorizontalMovement()
{
	if (m_entity == nullptr)
	{
		return;
	}
	m_entity->m_velocity.x = 0.f;
	m_entity->m_velocity.y = 0.f;
}

void AIController::StopMoving()
{
	StopHorizontalMovement();
	if (m_entity != nullptr)
	{
		m_entity->SetAnimationGroup( EntityState::IDLE );
	}
}

float AIController::GetLateralT() const
{
	if (m_entity == nullptr)
	{
		return 0.5f;
	}

	// Deterministic continuous hash in (0,1) — not id % N lanes.
	unsigned int const id = m_entity->GetUID().GetIndex() + m_entity->GetUID().GetSalt() * 4099u;
	float t = (float)id * AI_GOLDEN_FRAC;
	t = t - floorf( t );
	t = AI_LATERAL_EDGE_MARGIN + t * (1.f - 2.f * AI_LATERAL_EDGE_MARGIN);
	t = Clamp( t + m_lateralJitter, AI_LATERAL_EDGE_MARGIN, 1.f - AI_LATERAL_EDGE_MARGIN );
	return t;
}

void AIController::ApplyLateralBiasToPath( std::vector<NavPortal> const& portals )
{
	if (m_path.size() < 2 || portals.empty())
	{
		return;
	}

	float const lateralT = GetLateralT();

	for (NavPortal const& portal : portals)
	{
		Vec3 portalDelta = portal.m_right - portal.m_left;
		float const width = portalDelta.GetLength();
		if (width < AI_PORTAL_BIAS_MIN_WIDTH)
		{
			continue;
		}

		Vec3 const biased = portal.m_left + portalDelta * lateralT;
		Vec3 const portalMid = (portal.m_left + portal.m_right) * 0.5f;

		// Prefer replacing a waypoint already near this portal segment.
		int bestWp = -1;
		float bestWpDist = AI_PORTAL_SNAP_DISTANCE;
		for (int i = 1; i + 1 < (int)m_path.size(); ++i) // keep start/goal anchors
		{
			float const d = DistPointToSegment3D( m_path[i], portal.m_left, portal.m_right );
			if (d < bestWpDist)
			{
				bestWpDist = d;
				bestWp = i;
			}
		}

		if (bestWp >= 0)
		{
			m_path[bestWp] = biased;
			continue;
		}

		// Otherwise insert on the path edge that passes closest to the portal.
		int bestEdge = -1;
		float bestEdgeDist = AI_PORTAL_SNAP_DISTANCE;
		for (int i = 0; i + 1 < (int)m_path.size(); ++i)
		{
			float const d = DistPointToSegment3D( portalMid, m_path[i], m_path[i + 1] );
			if (d < bestEdgeDist)
			{
				bestEdgeDist = d;
				bestEdge = i;
			}
		}
		if (bestEdge >= 0)
		{
			m_path.insert( m_path.begin() + bestEdge + 1, biased );
		}
	}
}

void AIController::RequestRepath( Entity const* player )
{
	ClearPath();
	m_loggedPathFail = false;

	if (m_entity == nullptr || player == nullptr || m_entity->m_map == nullptr)
	{
		return;
	}

	// Tiny deterministic jitter on repath so agents don't stay perfectly phase-locked.
	++m_repathCount;
	unsigned int const id = m_entity->GetUID().GetIndex() + m_repathCount * 17u;
	float jitterFrac = (float)id * 0.37f;
	jitterFrac = jitterFrac - floorf( jitterFrac );
	m_lateralJitter = (jitterFrac - 0.5f) * (2.f * AI_LATERAL_JITTER_RANGE);

	m_lastRepathGoal = player->m_position;
	NavPath const navPath = m_entity->m_map->FindNavPath( m_entity->m_position, player->m_position );
	if (!navPath.m_isValid || navPath.m_waypoints.empty())
	{
		ClearPath();
		if (!m_loggedPathFail)
		{
			DebuggerPrintf( "AIController: FindNavPath failed (FindPoly miss or empty graph)\n" );
			m_loggedPathFail = true;
		}
		return;
	}

	m_path = navPath.m_waypoints;
	ApplyLateralBiasToPath( navPath.m_portals );

	m_pathIndex = 0;
	while (m_pathIndex < (int)m_path.size() &&
		GetDistanceXY( m_entity->m_position, m_path[m_pathIndex] ) <= AI_ARRIVE_RADIUS)
	{
		++m_pathIndex;
	}
}

void AIController::FaceTowardXY( Vec3 const& targetPos, float turnSpeed, float deltaSeconds )
{
	if (m_entity == nullptr)
	{
		return;
	}

	Vec3 toTarget( targetPos.x - m_entity->m_position.x, targetPos.y - m_entity->m_position.y, 0.f );
	if (toTarget.GetLengthSquared() < 0.000001f)
	{
		return;
	}

	float const goalYaw = Atan2Degrees( toTarget.y, toTarget.x );
	m_entity->m_orientation.m_yawDegrees = GetTurnedTowardDegrees(
		m_entity->m_orientation.m_yawDegrees,
		goalYaw,
		turnSpeed * deltaSeconds );
}

void AIController::MoveTowardXY( Vec3 const& targetPos, float walkSpeed, float turnSpeed, float deltaSeconds )
{
	Vec3 toTarget( targetPos.x - m_entity->m_position.x, targetPos.y - m_entity->m_position.y, 0.f );
	if (toTarget.GetLengthSquared() < 0.000001f)
	{
		StopMoving();
		return;
	}

	FaceTowardXY( targetPos, turnSpeed, deltaSeconds );

	Vec3 forwardXY = Vec3( m_entity->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetXY() );
	if (forwardXY.GetLengthSquared() < 0.0001f)
	{
		StopMoving();
		return;
	}
	forwardXY = forwardXY.GetNormalized();

	m_entity->m_velocity.x = forwardXY.x * walkSpeed;
	m_entity->m_velocity.y = forwardXY.y * walkSpeed;
	m_entity->SetAnimationGroup( EntityState::WALK );
}

void AIController::ApplySeparationToVelocity( float walkSpeed )
{
	if (m_entity == nullptr || m_entity->m_map == nullptr || walkSpeed <= 0.f)
	{
		return;
	}

	Vec3 separation = Vec3::ZERO;
	int neighborCount = 0;
	Vec3 const selfPos = m_entity->m_position;

	for (Entity* other : m_entity->m_map->GetEntities())
	{
		if (other == nullptr || other == m_entity || other->m_isDead)
		{
			continue;
		}
		if (dynamic_cast<AIController*>( other->m_controller ) == nullptr)
		{
			continue;
		}

		Vec3 delta( selfPos.x - other->m_position.x, selfPos.y - other->m_position.y, 0.f );
		float const distSq = delta.GetLengthSquared();
		if (distSq >= AI_SEP_RADIUS * AI_SEP_RADIUS || distSq < 0.000001f)
		{
			continue;
		}

		float const dist = sqrtf( distSq );
		float const strength = 1.f - (dist / AI_SEP_RADIUS);
		separation += (delta / dist) * strength;
		++neighborCount;
	}

	if (neighborCount == 0 || separation.GetLengthSquared() < 0.000001f)
	{
		return;
	}

	float sepScale = AI_SEP_SCALE;
	NavMesh const* mesh = m_entity->m_map->FindNavMeshForPosition( selfPos );
	if (mesh != nullptr)
	{
		NavPoly const* poly = mesh->GetPoly( mesh->FindPoly( selfPos ) );
		if (poly != nullptr && poly->m_kind == NavSurfaceKind::Slope)
		{
			sepScale = AI_SEP_SCALE_ON_SLOPE;
		}
	}

	Vec3 pathDir( m_entity->m_velocity.x, m_entity->m_velocity.y, 0.f );
	if (pathDir.GetLengthSquared() < 0.000001f)
	{
		pathDir = Vec3( m_entity->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetXY() );
	}
	if (pathDir.GetLengthSquared() < 0.000001f)
	{
		return;
	}
	pathDir = pathDir.GetNormalized();

	Vec3 sepDir = separation.GetNormalized();
	Vec3 moveDir = pathDir + sepDir * sepScale;
	if (moveDir.GetLengthSquared() < 0.000001f)
	{
		return;
	}
	moveDir = moveDir.GetNormalized();

	m_entity->m_velocity.x = moveDir.x * walkSpeed;
	m_entity->m_velocity.y = moveDir.y * walkSpeed;
}

void AIController::FollowPath( float deltaSeconds, Entity const* player )
{
	EntityDefinition const* definition = m_entity->GetDefinition();
	float walkSpeed = (definition != nullptr && definition->walkSpeed > 0.f) ? definition->walkSpeed : 2.f;
	float turnSpeed = (definition != nullptr && definition->turnSpeed > 0.f) ? definition->turnSpeed : 180.f;

	if (m_path.empty() || m_pathIndex >= (int)m_path.size())
	{
		MoveTowardXY( player->m_position, walkSpeed, turnSpeed, deltaSeconds );
		ApplySeparationToVelocity( walkSpeed );
		return;
	}

	Vec3 const& waypoint = m_path[m_pathIndex];
	if (GetDistanceXY( m_entity->m_position, waypoint ) <= AI_ARRIVE_RADIUS)
	{
		++m_pathIndex;
		if (m_pathIndex >= (int)m_path.size())
		{
			m_repathTimer = 0.f;
			MoveTowardXY( player->m_position, walkSpeed, turnSpeed, deltaSeconds );
			ApplySeparationToVelocity( walkSpeed );
			return;
		}
	}

	MoveTowardXY( m_path[m_pathIndex], walkSpeed, turnSpeed, deltaSeconds );
	ApplySeparationToVelocity( walkSpeed );
}

WeaponAttackDefinition const* AIController::GetMeleeAttackDef() const
{
	if (m_entity == nullptr)
	{
		return nullptr;
	}

	Weapon* weapon = m_entity->GetCurrentWeapon();
	if (weapon == nullptr || weapon->GetDefinition() == nullptr)
	{
		return nullptr;
	}

	WeaponAttackDefinition const& attack = weapon->GetDefinition()->mainAttack;
	if (attack.meleeCount <= 0)
	{
		return nullptr;
	}

	return &attack;
}

bool AIController::IsInMeleeRange( Entity const* target, WeaponAttackDefinition const& attack ) const
{
	if (m_entity == nullptr || target == nullptr)
	{
		return false;
	}

	Weapon* weapon = m_entity->GetCurrentWeapon();
	if (weapon == nullptr)
	{
		return false;
	}

	return weapon->IsTargetInMeleeReach( target, attack );
}

float AIController::GetMeleeAttackDuration( WeaponAttackDefinition const& attack ) const
{
	float duration = attack.damageDelay;
	if (m_entity != nullptr && m_entity->GetDefinition() != nullptr)
	{
		AnimationGroup* group = m_entity->GetDefinition()->GetAnimationGroupByName( "Attack" );
		if (group != nullptr && !group->directions.empty()
			&& group->directions[0] != nullptr && group->directions[0]->animationDef != nullptr)
		{
			float const animDuration = group->directions[0]->animationDef->m_durationSeconds;
			if (animDuration > duration)
			{
				duration = animDuration;
			}
		}
	}

	if (duration <= 0.f)
	{
		duration = 0.5f;
	}
	return duration;
}

void AIController::BeginMeleeAttack( WeaponAttackDefinition const& attack )
{
	m_isAttacking = true;
	m_meleeDamagePending = true;
	m_meleeDamageDelayTimer = (attack.damageDelay > 0.f) ? attack.damageDelay : 0.f;
	m_attackLockTimer = GetMeleeAttackDuration( attack );
	m_attackCooldownTimer = (attack.refireTime > 0.f) ? attack.refireTime : m_attackLockTimer;

	StopHorizontalMovement();
	if (m_entity->m_currentState == EntityState::ATTACK)
	{
		m_entity->m_animationTimer = 0.f;
	}
	else
	{
		m_entity->SetAnimationGroup( EntityState::ATTACK );
	}
}

void AIController::UpdateMeleeAttack( float deltaSeconds, Entity* player, WeaponAttackDefinition const& attack )
{
	UNUSED( attack );
	if (!m_entity->IsHitStunned())
	{
		StopHorizontalMovement();
	}

	EntityDefinition const* definition = m_entity->GetDefinition();
	float const turnSpeed = (definition != nullptr && definition->turnSpeed > 0.f) ? definition->turnSpeed : 180.f;
	if (player != nullptr && !m_entity->IsHitStunned())
	{
		FaceTowardXY( player->m_position, turnSpeed, deltaSeconds );
	}

	if (m_meleeDamagePending)
	{
		m_meleeDamageDelayTimer -= deltaSeconds;
		if (m_meleeDamageDelayTimer <= 0.f)
		{
			m_meleeDamagePending = false;
			m_meleeDamageDelayTimer = 0.f;
			Weapon* weapon = m_entity->GetCurrentWeapon();
			if (weapon != nullptr)
			{
				weapon->PerformMeleeStrike();
			}
		}
	}

	m_attackLockTimer -= deltaSeconds;
	if (m_attackLockTimer <= 0.f)
	{
		ResetAttackState();
	}
}

void AIController::ResetAttackState()
{
	m_isAttacking = false;
	m_meleeDamagePending = false;
	m_meleeDamageDelayTimer = 0.f;
	m_attackLockTimer = 0.f;
}

void AIController::DebugDrawOwnPath() const
{
	if (!AI_DEBUG_DRAW_PATH || !DebugRenderIsVisible() || m_path.size() < 2)
	{
		return;
	}
	if (m_entity == nullptr || m_entity->m_map == nullptr)
	{
		return;
	}

	NavMesh const* mesh = m_entity->m_map->FindNavMeshForPosition( m_entity->m_position );
	if (mesh != nullptr)
	{
		mesh->DebugRenderPath( m_path );
	}
}

void AIController::Update( float deltaSeconds )
{
	if (m_entity == nullptr || m_entity->m_isDead)
	{
		ClearPath();
		ResetAttackState();
		return;
	}

	Map* map = m_entity->m_map;
	if (map == nullptr)
	{
		StopMoving();
		ClearPath();
		ResetAttackState();
		return;
	}

	PlayerController* playerController = map->GetPlayerController();
	Entity* player = (playerController != nullptr) ? playerController->GetEntity() : nullptr;
	if (player == nullptr || player->m_isDead)
	{
		StopMoving();
		ClearPath();
		ResetAttackState();
		return;
	}

	// Let knockback / physics own horizontal velocity during hit stun.
	if (m_entity->IsHitStunned())
	{
		WeaponAttackDefinition const* meleeAttackWhileStunned = GetMeleeAttackDef();
		if (meleeAttackWhileStunned != nullptr && m_isAttacking)
		{
			UpdateMeleeAttack( deltaSeconds, player, *meleeAttackWhileStunned );
		}
		DebugDrawOwnPath();
		return;
	}

	if (m_attackCooldownTimer > 0.f)
	{
		m_attackCooldownTimer -= deltaSeconds;
		if (m_attackCooldownTimer < 0.f)
		{
			m_attackCooldownTimer = 0.f;
		}
	}

	WeaponAttackDefinition const* meleeAttack = GetMeleeAttackDef();
	if (meleeAttack != nullptr)
	{
		if (m_isAttacking)
		{
			UpdateMeleeAttack( deltaSeconds, player, *meleeAttack );
			if (m_isAttacking)
			{
				DebugDrawOwnPath();
				return;
			}
		}

		if (IsInMeleeRange( player, *meleeAttack ))
		{
			StopHorizontalMovement();
			EntityDefinition const* definition = m_entity->GetDefinition();
			float const turnSpeed = (definition != nullptr && definition->turnSpeed > 0.f) ? definition->turnSpeed : 180.f;
			FaceTowardXY( player->m_position, turnSpeed, deltaSeconds );

			if (m_attackCooldownTimer <= 0.f)
			{
				BeginMeleeAttack( *meleeAttack );
			}
			else
			{
				m_entity->SetAnimationGroup( EntityState::IDLE );
			}

			DebugDrawOwnPath();
			return;
		}
	}

	m_repathTimer -= deltaSeconds;
	bool const goalMovedFar = GetDistanceXY( player->m_position, m_lastRepathGoal ) >= AI_REPATH_GOAL_MOVE_THRESHOLD;
	bool const needRepath = m_path.empty() || m_repathTimer <= 0.f || goalMovedFar;
	if (needRepath)
	{
		RequestRepath( player );
		m_repathTimer = AI_REPATH_INTERVAL;
	}

	FollowPath( deltaSeconds, player );

	float const horizontalSpeed = Vec3( m_entity->m_velocity.x, m_entity->m_velocity.y, 0.f ).GetLength();
	if (horizontalSpeed < AI_IDLE_SPEED_EPSILON)
	{
		m_entity->SetAnimationGroup( EntityState::IDLE );
	}

	DebugDrawOwnPath();
}
