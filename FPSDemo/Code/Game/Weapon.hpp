#pragma once

#include <string>

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/EntityUID.hpp"

class Entity;

enum class WeaponAttackSlot
{
	Main,
	Sub
};

struct WeaponAttackState
{
	float cooldownRemaining = 0.f;
	bool isCharging = false;
	float currentChargeTime = 0.f;
	int chargeAmmoConsumed = 0;
};

class Weapon
{
public:
	Weapon( Entity* owner, WeaponDefinition const* definition );
	~Weapon();

	void Update( float deltaSeconds );
	void UpdateAttackInput( WeaponAttackSlot slot, bool isDown, bool wasJustReleased, float deltaSeconds );
	void SyncChargingProjectile();
	void CancelCharge();
	void ClearChargingProjectileRef( Entity* destroyedProjectile );
	void Render() const;

	WeaponDefinition const* GetDefinition() const { return m_definition; }
	float GetChargeFraction( WeaponAttackSlot slot ) const;
	bool HasSubAttack() const;
	bool HasHud() const;
	bool UsesAmmo() const;
	int GetMagazineAmmo() const { return m_magazineAmmo; }
	int GetReserveAmmo() const { return m_reserveAmmo; }
	int GetMagazineSize() const;
	void AddReserveAmmo( int amount );
	bool IsReloading() const { return m_isReloading; }
	bool TryBeginReload();
	void CancelReload();

	// AI-owned windup/cooldown: resolve melee hit + Fire sound without touching weapon cooldown.
	void PerformMeleeStrike();
	bool IsTargetInMeleeReach( Entity const* target, WeaponAttackDefinition const& attack ) const;
	bool IsTargetInMeleeArc( Entity const* target, WeaponAttackDefinition const& attack ) const;

private:
	WeaponAttackDefinition const* GetAttackDefinition( WeaponAttackSlot slot ) const;
	WeaponAttackState& GetAttackState( WeaponAttackSlot slot );
	WeaponAttackState const& GetAttackState( WeaponAttackSlot slot ) const;

	void TryFire( WeaponAttackSlot slot, float chargeFraction );
	void FireRays( WeaponAttackDefinition const& attack );
	void FireProjectiles( WeaponAttackDefinition const& attack, float chargeFraction );
	void FireMelee( WeaponAttackDefinition const& attack, bool applyKnockback );
	void PlayFireSound();
	float GetTargetRadius( Entity const* target ) const;
	float GetMeleeReachDistance( Entity const* target, WeaponAttackDefinition const& attack ) const;

	void BeginChargingProjectile( WeaponAttackDefinition const& attack );
	void UpdateChargingProjectile( float chargeFraction );
	void ReleaseChargingProjectile( WeaponAttackDefinition const& attack, float chargeFraction );
	void DestroyChargingProjectile();
	Entity* GetChargingProjectile() const;
	void SetChargingProjectile( Entity* projectile );
	Vec3 GetChargingProjectileSpawnPosition() const;
	Vec3 GetChargingProjectileHeldPosition( Entity const* projectile ) const;
	float GetOwnerRadius() const;

	void PlayHudAnimation( std::string const& name );
	void UpdateHudAnimation( float deltaSeconds );
	void BeginHudMeleeSwing();
	void UpdateHudMeleeSwing( float deltaSeconds );
	void ResetHudMelee();
	void UpdatePendingMeleeDamage( float deltaSeconds );
	Vec2 GetHudMeleeOffset() const;
	bool UsesAmmoForAttack( WeaponAttackDefinition const& attack ) const;

	bool CanStartAttack( WeaponAttackSlot slot ) const;
	void ConsumeChargeAmmo( WeaponAttackSlot slot );
	int ConsumeMagazineAmmo( int amount );
	bool CanReload() const;
	void UpdateReload( float deltaSeconds );
	void CompleteReload();
	void RenderReloadHud( Vec2 const& hudSize ) const;

	Vec3 GetMuzzlePosition() const;
	Vec3 GetAimDirection() const;
	Vec3 GetDirectionInCone( Vec3 const& forward, float coneDegrees ) const;
	bool CanDamageTarget( Entity const* target ) const;

private:
	Entity* m_owner = nullptr;
	WeaponDefinition const* m_definition = nullptr;
	WeaponAttackState m_mainState;
	WeaponAttackState m_subState;

	mutable EntityUID m_chargingProjectileUID = EntityUID::INVALID;

	int m_magazineAmmo = 0;
	int m_reserveAmmo = 0;
	bool m_isReloading = false;
	float m_reloadTimer = 0.f;

	float m_animationTimer = 0.f;
	WeaponHudAnimationDefinition const* m_currentHudAnimation = nullptr;

	bool m_isHudMeleeActive = false;
	float m_hudMeleeTimer = 0.f;
	Vec2 m_hudSpriteOffset = Vec2::ZERO;
	bool m_meleeDamagePending = false;
	float m_meleeDamageDelayTimer = 0.f;
	WeaponAttackDefinition const* m_pendingMeleeAttack = nullptr;
	bool m_pendingMeleeApplyKnockback = false;
};
