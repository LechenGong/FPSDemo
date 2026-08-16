#pragma once

#include "Game/Controller.hpp"
#include "Game/NavMesh.hpp"

#include "Engine/Math/Vec3.hpp"

#include <vector>

struct WeaponAttackDefinition;

class AIController : public Controller
{
public:
	AIController() = default;
	virtual ~AIController() = default;

	void Update( float deltaSeconds ) override;

private:
	void ClearPath();
	void StopHorizontalMovement();
	void StopMoving();
	void RequestRepath( class Entity const* player );
	void FollowPath( float deltaSeconds, Entity const* player );
	void FaceTowardXY( Vec3 const& targetPos, float turnSpeed, float deltaSeconds );
	void MoveTowardXY( Vec3 const& targetPos, float walkSpeed, float turnSpeed, float deltaSeconds );
	void ApplySeparationToVelocity( float walkSpeed );
	void ApplyLateralBiasToPath( std::vector<NavPortal> const& portals );
	float GetLateralT() const;
	void DebugDrawOwnPath() const;
	static float GetDistanceXY( Vec3 const& a, Vec3 const& b );
	static float DistPointToSegment3D( Vec3 const& point, Vec3 const& a, Vec3 const& b );

	WeaponAttackDefinition const* GetMeleeAttackDef() const;
	bool IsInMeleeRange( Entity const* target, WeaponAttackDefinition const& attack ) const;
	float GetMeleeAttackDuration( WeaponAttackDefinition const& attack ) const;
	void BeginMeleeAttack( WeaponAttackDefinition const& attack );
	void UpdateMeleeAttack( float deltaSeconds, Entity* player, WeaponAttackDefinition const& attack );
	void ResetAttackState();

private:
	std::vector<Vec3> m_path;
	int m_pathIndex = 0;
	float m_repathTimer = 0.f;
	Vec3 m_lastRepathGoal = Vec3::ZERO;
	bool m_loggedPathFail = false;
	float m_lateralJitter = 0.f;
	unsigned int m_repathCount = 0;

	bool m_isAttacking = false;
	bool m_meleeDamagePending = false;
	float m_meleeDamageDelayTimer = 0.f;
	float m_attackLockTimer = 0.f;
	float m_attackCooldownTimer = 0.f;
};
