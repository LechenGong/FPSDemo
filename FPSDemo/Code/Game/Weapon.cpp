#include "Game/Weapon.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerController.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Audio/AudioSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtil.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"

namespace
{
	constexpr float HUD_MELEE_HOLD_SECONDS = 0.12f;
	constexpr float HUD_MELEE_RAISE_SECONDS = 0.08f;
	constexpr float HUD_MELEE_SLASH_SECONDS = 0.14f;
	Vec2 const HUD_MELEE_RAISE_OFFSET( 280.f, 300.f );
	Vec2 const HUD_MELEE_SLASH_OFFSET( -720.f, -480.f );

	constexpr float HUD_RELOAD_RING_RADIUS = 22.f;
	constexpr float HUD_RELOAD_RING_THICKNESS = 3.f;
	constexpr float HUD_RELOAD_TEXT_HEIGHT = 18.f;
	constexpr float HUD_RELOAD_TEXT_ASPECT = 0.7f;

	void AddVertsForRingArc2D( std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness,
		float startDegrees, float sweepDegrees, Rgba8 const& color, int numSlices )
	{
		if (numSlices <= 0)
		{
			return;
		}
		float const absSweep = (sweepDegrees < 0.f) ? -sweepDegrees : sweepDegrees;
		if (absSweep < 0.001f)
		{
			return;
		}

		float const innerRadius = radius - thickness * 0.5f;
		float const outerRadius = radius + thickness * 0.5f;
		float const sliceDegrees = sweepDegrees / (float)numSlices;
		for (int i = 0; i < numSlices; ++i)
		{
			float const a0 = startDegrees + sliceDegrees * (float)i;
			float const a1 = startDegrees + sliceDegrees * (float)(i + 1);
			Vec2 const outer0 = center + Vec2( outerRadius * CosDegrees( a0 ), outerRadius * SinDegrees( a0 ) );
			Vec2 const outer1 = center + Vec2( outerRadius * CosDegrees( a1 ), outerRadius * SinDegrees( a1 ) );
			Vec2 const inner1 = center + Vec2( innerRadius * CosDegrees( a1 ), innerRadius * SinDegrees( a1 ) );
			Vec2 const inner0 = center + Vec2( innerRadius * CosDegrees( a0 ), innerRadius * SinDegrees( a0 ) );
			AddVertsForRect2D( verts, outer0, outer1, inner1, inner0, color );
		}
	}
}

Weapon::Weapon( Entity* owner, WeaponDefinition const* definition )
	: m_owner( owner )
	, m_definition( definition )
{
	GUARANTEE_OR_DIE( m_owner != nullptr, "Weapon requires an owner entity" );
	GUARANTEE_OR_DIE( m_definition != nullptr, "Weapon requires a definition" );
	if (m_definition->magazineSize > 0)
	{
		m_magazineAmmo = m_definition->magazineSize;
		m_reserveAmmo = m_definition->startingReserveAmmo;
	}
	PlayHudAnimation( "Idle" );
}

Weapon::~Weapon()
{
	DestroyChargingProjectile();
}

void Weapon::Update( float deltaSeconds )
{
	if (m_mainState.cooldownRemaining > 0.f)
	{
		m_mainState.cooldownRemaining -= deltaSeconds;
		if (m_mainState.cooldownRemaining < 0.f)
		{
			m_mainState.cooldownRemaining = 0.f;
		}
	}

	if (m_subState.cooldownRemaining > 0.f)
	{
		m_subState.cooldownRemaining -= deltaSeconds;
		if (m_subState.cooldownRemaining < 0.f)
		{
			m_subState.cooldownRemaining = 0.f;
		}
	}

	UpdatePendingMeleeDamage( deltaSeconds );
	UpdateHudMeleeSwing( deltaSeconds );
	UpdateReload( deltaSeconds );
	UpdateHudAnimation( deltaSeconds );
}

void Weapon::UpdateAttackInput( WeaponAttackSlot slot, bool isDown, bool wasJustReleased, float deltaSeconds )
{
	if (m_isReloading)
	{
		return;
	}

	if (m_isHudMeleeActive && slot == WeaponAttackSlot::Main)
	{
		return;
	}

	WeaponAttackDefinition const* attackDef = GetAttackDefinition( slot );
	if (attackDef == nullptr)
	{
		return;
	}

	WeaponAttackState& state = GetAttackState( slot );

	if (attackDef->doesCharge)
	{
		if (isDown)
		{
			if (!state.isCharging && state.cooldownRemaining <= 0.f)
			{
				if (CanStartAttack( slot ))
				{
					state.isCharging = true;
					state.currentChargeTime = 0.f;
					state.chargeAmmoConsumed = 0;
					if (attackDef->projectileCount > 0)
					{
						BeginChargingProjectile( *attackDef );
					}
				}
				else
				{
					TryBeginReload();
				}
			}

			if (state.isCharging)
			{
				state.currentChargeTime += deltaSeconds;
				if (attackDef->chargeTime > 0.f && state.currentChargeTime > attackDef->chargeTime)
				{
					state.currentChargeTime = attackDef->chargeTime;
				}
				ConsumeChargeAmmo( slot );
				if (Entity* charging = GetChargingProjectile())
				{
					charging->SetScale( GetChargeFraction( slot ) );
				}
			}
		}

		if (wasJustReleased && state.isCharging)
		{
			float chargeFraction = GetChargeFraction( slot );
			state.isCharging = false;
			state.currentChargeTime = 0.f;
			TryFire( slot, chargeFraction );
			state.chargeAmmoConsumed = 0;
		}
	}
	else
	{
		if (state.isCharging)
		{
			state.isCharging = false;
			state.currentChargeTime = 0.f;
			state.chargeAmmoConsumed = 0;
			DestroyChargingProjectile();
		}

		if (isDown && state.cooldownRemaining <= 0.f)
		{
			if (CanStartAttack( slot ))
			{
				TryFire( slot, 1.f );
			}
			else
			{
				TryBeginReload();
			}
		}
	}
}

void Weapon::CancelCharge()
{
	m_mainState.isCharging = false;
	m_mainState.currentChargeTime = 0.f;
	m_mainState.chargeAmmoConsumed = 0;
	m_subState.isCharging = false;
	m_subState.currentChargeTime = 0.f;
	m_subState.chargeAmmoConsumed = 0;
	DestroyChargingProjectile();
	ResetHudMelee();
}

float Weapon::GetChargeFraction( WeaponAttackSlot slot ) const
{
	WeaponAttackDefinition const* attackDef = GetAttackDefinition( slot );
	if (attackDef == nullptr || !attackDef->doesCharge)
	{
		return 1.f;
	}

	WeaponAttackState const& state = GetAttackState( slot );
	if (attackDef->chargeTime <= 0.f)
	{
		return 1.f;
	}

	return Clamp( state.currentChargeTime / attackDef->chargeTime, 0.f, 1.f );
}

bool Weapon::HasSubAttack() const
{
	return m_definition != nullptr && m_definition->hasSubAttack;
}

WeaponAttackDefinition const* Weapon::GetAttackDefinition( WeaponAttackSlot slot ) const
{
	if (m_definition == nullptr)
	{
		return nullptr;
	}

	if (slot == WeaponAttackSlot::Main)
	{
		return &m_definition->mainAttack;
	}

	if (!m_definition->hasSubAttack)
	{
		return nullptr;
	}

	return &m_definition->subAttack;
}

WeaponAttackState& Weapon::GetAttackState( WeaponAttackSlot slot )
{
	return (slot == WeaponAttackSlot::Main) ? m_mainState : m_subState;
}

WeaponAttackState const& Weapon::GetAttackState( WeaponAttackSlot slot ) const
{
	return (slot == WeaponAttackSlot::Main) ? m_mainState : m_subState;
}

void Weapon::TryFire( WeaponAttackSlot slot, float chargeFraction )
{
	WeaponAttackDefinition const* attackDef = GetAttackDefinition( slot );
	if (attackDef == nullptr || m_owner == nullptr || m_owner->m_isDead)
	{
		return;
	}

	WeaponAttackState& state = GetAttackState( slot );
	if (state.cooldownRemaining > 0.f)
	{
		return;
	}

	if (UsesAmmoForAttack( *attackDef ))
	{
		bool const chargePaidAmmo = attackDef->doesCharge && attackDef->chargeAmmoCost > 0;
		if (chargePaidAmmo)
		{
			if (state.chargeAmmoConsumed <= 0)
			{
				int const shotCost = (attackDef->ammoPerShot > 0) ? attackDef->ammoPerShot : 1;
				ConsumeMagazineAmmo( shotCost );
			}
		}
		else
		{
			int const shotCost = attackDef->ammoPerShot;
			if (m_magazineAmmo < shotCost)
			{
				return;
			}
			ConsumeMagazineAmmo( shotCost );
		}
	}

	state.cooldownRemaining = attackDef->refireTime;

	if (attackDef->meleeCount > 0)
	{
		CancelCharge();
		// Only Plasma (etc.) SubAttack bayonet should shove targets — not DemonMelee main.
		bool const applyKnockback = (slot == WeaponAttackSlot::Sub);
		if (attackDef->damageDelay > 0.f)
		{
			m_pendingMeleeAttack = attackDef;
			m_pendingMeleeApplyKnockback = applyKnockback;
			m_meleeDamagePending = true;
			m_meleeDamageDelayTimer = attackDef->damageDelay;
		}
		else
		{
			FireMelee( *attackDef, applyKnockback );
		}
		BeginHudMeleeSwing();
	}
	else
	{
		if (attackDef->rayCount > 0)
		{
			FireRays( *attackDef );
		}
		if (attackDef->projectileCount > 0)
		{
			FireProjectiles( *attackDef, chargeFraction );
		}
		PlayFireSound();
		PlayHudAnimation( "Attack" );
	}

	m_owner->SetAnimationGroup( EntityState::ATTACK );
}

bool Weapon::HasHud() const
{
	return m_definition != nullptr && m_definition->hasHud;
}

bool Weapon::UsesAmmo() const
{
	return m_definition != nullptr && m_definition->magazineSize > 0;
}

int Weapon::GetMagazineSize() const
{
	return (m_definition != nullptr) ? m_definition->magazineSize : 0;
}

void Weapon::AddReserveAmmo( int amount )
{
	if (amount <= 0 || !UsesAmmo())
	{
		return;
	}
	m_reserveAmmo += amount;
}

bool Weapon::CanStartAttack( WeaponAttackSlot slot ) const
{
	WeaponAttackDefinition const* attackDef = GetAttackDefinition( slot );
	if (attackDef == nullptr)
	{
		return false;
	}

	if (!UsesAmmoForAttack( *attackDef ))
	{
		return true;
	}

	return m_magazineAmmo >= attackDef->ammoPerShot;
}

bool Weapon::UsesAmmoForAttack( WeaponAttackDefinition const& attack ) const
{
	return UsesAmmo() && attack.ammoPerShot > 0;
}

void Weapon::ConsumeChargeAmmo( WeaponAttackSlot slot )
{
	if (!UsesAmmo())
	{
		return;
	}

	WeaponAttackDefinition const* attackDef = GetAttackDefinition( slot );
	if (attackDef == nullptr || attackDef->chargeAmmoCost <= 0)
	{
		return;
	}

	WeaponAttackState& state = GetAttackState( slot );
	int const desired = RoundDownToInt( GetChargeFraction( slot ) * (float)attackDef->chargeAmmoCost );
	int const toConsume = desired - state.chargeAmmoConsumed;
	if (toConsume <= 0)
	{
		return;
	}

	state.chargeAmmoConsumed += ConsumeMagazineAmmo( toConsume );
}

int Weapon::ConsumeMagazineAmmo( int amount )
{
	if (amount <= 0 || !UsesAmmo() || m_magazineAmmo <= 0)
	{
		return 0;
	}

	int const actual = (amount < m_magazineAmmo) ? amount : m_magazineAmmo;
	m_magazineAmmo -= actual;
	return actual;
}

bool Weapon::CanReload() const
{
	if (!UsesAmmo() || m_definition == nullptr || m_isReloading)
	{
		return false;
	}
	if (m_definition->reloadTime <= 0.f)
	{
		return false;
	}
	if (m_reserveAmmo <= 0)
	{
		return false;
	}
	return m_magazineAmmo < m_definition->magazineSize;
}

bool Weapon::TryBeginReload()
{
	if (!CanReload())
	{
		return false;
	}

	CancelCharge();
	m_isReloading = true;
	m_reloadTimer = 0.f;
	return true;
}

void Weapon::CancelReload()
{
	m_isReloading = false;
	m_reloadTimer = 0.f;
}

void Weapon::UpdateReload( float deltaSeconds )
{
	if (!m_isReloading)
	{
		return;
	}

	m_reloadTimer += deltaSeconds;
	float const reloadTime = (m_definition != nullptr) ? m_definition->reloadTime : 0.f;
	if (reloadTime <= 0.f || m_reloadTimer >= reloadTime)
	{
		CompleteReload();
	}
}

void Weapon::CompleteReload()
{
	if (m_definition == nullptr)
	{
		CancelReload();
		return;
	}

	int const space = m_definition->magazineSize - m_magazineAmmo;
	int const loaded = (space < m_reserveAmmo) ? space : m_reserveAmmo;
	if (loaded > 0)
	{
		m_magazineAmmo += loaded;
		m_reserveAmmo -= loaded;
	}

	m_isReloading = false;
	m_reloadTimer = 0.f;
}

void Weapon::PlayHudAnimation( std::string const& name )
{
	if (m_definition == nullptr || !m_definition->hasHud)
	{
		m_currentHudAnimation = nullptr;
		return;
	}

	WeaponHudAnimationDefinition const* animation = m_definition->hud.GetAnimationByName( name );
	if (animation == nullptr || animation->animDef == nullptr)
	{
		return;
	}

	m_currentHudAnimation = animation;
	m_animationTimer = 0.f;
}

void Weapon::UpdateHudAnimation( float deltaSeconds )
{
	if (m_currentHudAnimation == nullptr || m_currentHudAnimation->animDef == nullptr)
	{
		return;
	}

	m_animationTimer += deltaSeconds;

	if (m_isHudMeleeActive)
	{
		return;
	}

	if (m_currentHudAnimation->name != "Idle"
		&& m_animationTimer >= m_currentHudAnimation->animDef->m_durationSeconds)
	{
		PlayHudAnimation( "Idle" );
	}
}

void Weapon::BeginHudMeleeSwing()
{
	m_isHudMeleeActive = true;
	m_hudMeleeTimer = 0.f;
	m_hudSpriteOffset = Vec2::ZERO;
	if (m_definition != nullptr && m_definition->hud.GetAnimationByName( "Melee" ) != nullptr)
	{
		PlayHudAnimation( "Melee" );
	}
	else
	{
		PlayHudAnimation( "Attack" );
	}
}

void Weapon::UpdateHudMeleeSwing( float deltaSeconds )
{
	if (!m_isHudMeleeActive)
	{
		return;
	}

	m_hudMeleeTimer += deltaSeconds;
	m_hudSpriteOffset = GetHudMeleeOffset();

	float const totalSeconds = HUD_MELEE_HOLD_SECONDS + HUD_MELEE_RAISE_SECONDS + HUD_MELEE_SLASH_SECONDS;
	if (m_hudMeleeTimer >= totalSeconds)
	{
		ResetHudMelee();
	}
}

void Weapon::ResetHudMelee()
{
	bool const wasMelee = m_isHudMeleeActive;
	m_isHudMeleeActive = false;
	m_hudMeleeTimer = 0.f;
	m_hudSpriteOffset = Vec2::ZERO;
	m_meleeDamagePending = false;
	m_meleeDamageDelayTimer = 0.f;
	m_pendingMeleeAttack = nullptr;
	m_pendingMeleeApplyKnockback = false;
	if (wasMelee)
	{
		PlayHudAnimation( "Idle" );
	}
}

void Weapon::UpdatePendingMeleeDamage( float deltaSeconds )
{
	if (!m_meleeDamagePending)
	{
		return;
	}

	m_meleeDamageDelayTimer -= deltaSeconds;
	if (m_meleeDamageDelayTimer > 0.f)
	{
		return;
	}

	m_meleeDamagePending = false;
	m_meleeDamageDelayTimer = 0.f;
	if (m_pendingMeleeAttack != nullptr && m_owner != nullptr && !m_owner->m_isDead)
	{
		FireMelee( *m_pendingMeleeAttack, m_pendingMeleeApplyKnockback );
	}
	m_pendingMeleeAttack = nullptr;
	m_pendingMeleeApplyKnockback = false;
}

Vec2 Weapon::GetHudMeleeOffset() const
{
	float t = m_hudMeleeTimer;
	if (t <= HUD_MELEE_HOLD_SECONDS)
	{
		return Vec2::ZERO;
	}

	t -= HUD_MELEE_HOLD_SECONDS;
	if (t <= HUD_MELEE_RAISE_SECONDS)
	{
		float const u = SmoothStop2( Clamp( t / HUD_MELEE_RAISE_SECONDS, 0.f, 1.f ) );
		return Interpolate( Vec2::ZERO, HUD_MELEE_RAISE_OFFSET, u );
	}

	t -= HUD_MELEE_RAISE_SECONDS;
	float const u = SmoothStart3( Clamp( t / HUD_MELEE_SLASH_SECONDS, 0.f, 1.f ) );
	return Interpolate( HUD_MELEE_RAISE_OFFSET, HUD_MELEE_SLASH_OFFSET, u );
}

void Weapon::Render() const
{
	if (!HasHud())
	{
		return;
	}

	PlayerController* playerController = nullptr;
	if (m_owner != nullptr)
	{
		playerController = dynamic_cast<PlayerController*>( m_owner->GetController() );
	}
	if (playerController == nullptr)
	{
		return;
	}

	WeaponHudDefinition const& hud = m_definition->hud;
	Vec2 const hudSize = playerController->m_hudCamera.GetDimensions();

	Shader* shader = hud.shader;
	if (shader == nullptr)
	{
		shader = g_theRenderer->CreateShader( "Default", VertexType::VERTEX_PCU );
	}

	// Viewmodel weapon sprite (bottom center)
	if (m_currentHudAnimation != nullptr && m_currentHudAnimation->animDef != nullptr)
	{
		AABB2 spriteUVs = m_currentHudAnimation->animDef->GetSpriteDefAtTime( m_animationTimer ).GetUVs();
		Vec2 const& size = hud.spriteSize;
		Vec2 const& pivot = hud.spritePivot;
		Vec2 position( hudSize.x * 0.5f, 0.f );
		position += m_hudSpriteOffset;

		AABB2 localBounds(
			position + Vec2( -size.x * pivot.x, -size.y * pivot.y ),
			position + Vec2( size.x * (1.f - pivot.x), size.y * (1.f - pivot.y) )
		);

		std::vector<Vertex_PCU> weaponVerts;
		AddVertsForAABB2D( weaponVerts, localBounds, Rgba8::WHITE, spriteUVs.m_mins, spriteUVs.m_maxs );

		Shader* weaponShader = m_currentHudAnimation->shader;
		if (weaponShader == nullptr)
		{
			weaponShader = shader;
		}

		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader( weaponShader );
		g_theRenderer->BindTexture( &m_currentHudAnimation->animDef->m_spriteSheet.GetTexture() );
		g_theRenderer->DrawVertexArray( weaponVerts );
	}

	// Screen-center reticle
	if (hud.reticleTexture != nullptr && hud.reticleSize.x > 0.f && hud.reticleSize.y > 0.f)
	{
		Vec2 const center = hudSize * 0.5f;
		Vec2 const halfSize = hud.reticleSize * 0.5f;
		AABB2 reticleBounds( center - halfSize, center + halfSize );

		std::vector<Vertex_PCU> reticleVerts;
		AddVertsForAABB2D( reticleVerts, reticleBounds, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );

		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader( shader );
		g_theRenderer->BindTexture( hud.reticleTexture );
		g_theRenderer->DrawVertexArray( reticleVerts );
	}

	RenderReloadHud( hudSize );
}

void Weapon::RenderReloadHud( Vec2 const& hudSize ) const
{
	if (!m_isReloading || m_definition == nullptr)
	{
		return;
	}

	float const reloadTime = m_definition->reloadTime;
	float const fraction = (reloadTime > 0.f) ? Clamp( m_reloadTimer / reloadTime, 0.f, 1.f ) : 1.f;
	Vec2 const center = hudSize * 0.5f;

	std::vector<Vertex_PCU> ringVerts;
	AddVertsForRing2D( ringVerts, center, HUD_RELOAD_RING_RADIUS, HUD_RELOAD_RING_THICKNESS,
		Rgba8( 40, 40, 40, 180 ) );
	if (fraction > 0.f)
	{
		int const slices = (int)(64.f * fraction);
		int const arcSlices = (slices > 1) ? slices : 1;
		AddVertsForRingArc2D( ringVerts, center, HUD_RELOAD_RING_RADIUS, HUD_RELOAD_RING_THICKNESS,
			90.f, -360.f * fraction, Rgba8( 255, 220, 80, 255 ), arcSlices );
	}

	Shader* shader = g_theRenderer->CreateShader( "Default", VertexType::VERTEX_PCU );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( shader );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( ringVerts );

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	if (font == nullptr)
	{
		return;
	}

	float const blink = 0.35f + 0.65f * (0.5f + 0.5f * SinDegrees( m_reloadTimer * 480.f ));
	Rgba8 const textTint( 255, 255, 255, (unsigned char)(blink * 255.f) );
	std::string const reloadText = "reloading";
	float const textWidth = font->GetTextWidth( HUD_RELOAD_TEXT_HEIGHT, reloadText, HUD_RELOAD_TEXT_ASPECT );
	Vec2 const textMins( center.x - textWidth * 0.5f, center.y - HUD_RELOAD_RING_RADIUS - HUD_RELOAD_TEXT_HEIGHT - 6.f );

	std::vector<Vertex_PCU> textVerts;
	font->AddVertsForText2D( textVerts, textMins, HUD_RELOAD_TEXT_HEIGHT, reloadText, textTint, HUD_RELOAD_TEXT_ASPECT );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( shader );
	g_theRenderer->BindTexture( &font->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}

void Weapon::FireRays( WeaponAttackDefinition const& attack )
{
	if (m_owner == nullptr || m_owner->m_map == nullptr)
	{
		return;
	}

	Vec3 muzzle = GetMuzzlePosition();
	Vec3 aimForward = GetAimDirection();

	for (int rayIndex = 0; rayIndex < attack.rayCount; ++rayIndex)
	{
		Vec3 rayDir = GetDirectionInCone( aimForward, attack.rayCone );
		RaycastResult3D worldHit = m_owner->m_map->RaycastWorld( muzzle, rayDir, attack.rayRange );

		float bestDist = worldHit.m_didImpact ? worldHit.m_impactDist : attack.rayRange;
		Entity* bestEntity = nullptr;
		RaycastResult3D bestEntityHit;

		for (Entity* entity : m_owner->m_map->GetEntities())
		{
			if (!CanDamageTarget( entity ) || entity->GetCollision() == nullptr)
			{
				continue;
			}

			RaycastResult3D entityHit;
			bool didHit = false;
			ShapeComponent* collision = entity->GetCollision();
			if (collision->GetCollisionShape() == CollisionShape::CAPSULE)
			{
				CapsuleComponent* capsule = static_cast<CapsuleComponent*>( collision );
				didHit = RaycastCapsule3D( entityHit, muzzle, rayDir, attack.rayRange,
					capsule->GetWorldBoneStart(), capsule->GetWorldBoneEnd(), capsule->GetScaledRadius() );
			}
			else if (collision->GetCollisionShape() == CollisionShape::SPHERE)
			{
				SphereComponent* sphere = static_cast<SphereComponent*>( collision );
				didHit = RaycastSphere3D( entityHit, muzzle, rayDir, attack.rayRange,
					sphere->GetWorldPosition(), sphere->GetScaledRadius() );
			}
			else
			{
				OBB3 bounds = collision->CalculateBoundsOBB3D();
				didHit = RaycastOBB3D( entityHit, muzzle, rayDir, attack.rayRange, bounds );
			}

			if (didHit && entityHit.m_didImpact && entityHit.m_impactDist < bestDist)
			{
				bestDist = entityHit.m_impactDist;
				bestEntity = entity;
				bestEntityHit = entityHit;
			}
		}

		if (bestEntity != nullptr)
		{
			bestEntity->TakeDamage( attack.rayDamage );
			if (attack.rayImpulse > 0.f && !bestEntity->IsInvincible())
			{
				Vec3 impulseDir = rayDir.GetNormalized();
				bestEntity->m_velocity += impulseDir * attack.rayImpulse;
			}

			Vec3 const impactPos = bestEntityHit.m_didImpact
				? bestEntityHit.m_impactPos
				: muzzle + rayDir.GetNormalized() * bestDist;
			Vec3 const vfxPos = m_owner->m_map->GetHitEffectPosition(
				impactPos, bestEntityHit.m_impactNormal, bestEntity, muzzle );
			EntityDefinition const* bloodDef = EntityDefinition::GetEntityDef( "BloodSplatter" );
			if (bloodDef != nullptr)
			{
				m_owner->m_map->SpawnActor( bloodDef, vfxPos, EulerAngles() );
			}
		}
		else if (worldHit.m_didImpact)
		{
			m_owner->m_map->NotifyShotImpact( worldHit.m_impactPos );
			EntityDefinition const* bulletDef = EntityDefinition::GetEntityDef( "BulletHit" );
			if (bulletDef != nullptr)
			{
				Vec3 const vfxPos = m_owner->m_map->GetHitEffectPosition(
					worldHit.m_impactPos, worldHit.m_impactNormal, nullptr, muzzle );
				m_owner->m_map->SpawnActor( bulletDef, vfxPos, EulerAngles() );
			}
		}
	}
}

void Weapon::FireProjectiles( WeaponAttackDefinition const& attack, float chargeFraction )
{
	if (m_owner == nullptr || m_owner->m_map == nullptr || attack.projectileActor.empty())
	{
		return;
	}

	if (GetChargingProjectile() != nullptr)
	{
		ReleaseChargingProjectile( attack, chargeFraction );
		return;
	}

	EntityDefinition const* projectileDef = EntityDefinition::GetEntityDef( attack.projectileActor );
	if (projectileDef == nullptr)
	{
		return;
	}

	Vec3 spawnPos = GetChargingProjectileSpawnPosition();
	Vec3 aimForward = GetAimDirection();
	float speed = attack.projectileSpeed;

	for (int projectileIndex = 0; projectileIndex < attack.projectileCount; ++projectileIndex)
	{
		Vec3 launchDir = aimForward;
		EulerAngles orientation;
		orientation.m_yawDegrees = Atan2Degrees( launchDir.y, launchDir.x );
		orientation.m_pitchDegrees = -ASinDegrees( Clamp( launchDir.z, -1.f, 1.f ) );

		Entity* projectile = m_owner->m_map->SpawnActor( projectileDef, spawnPos, orientation );
		if (projectile != nullptr)
		{
			projectile->m_velocity = launchDir.GetNormalized() * speed;
		}
	}
}

void Weapon::BeginChargingProjectile( WeaponAttackDefinition const& attack )
{
	DestroyChargingProjectile();

	if (m_owner == nullptr || m_owner->m_map == nullptr || attack.projectileActor.empty())
	{
		return;
	}

	EntityDefinition const* projectileDef = EntityDefinition::GetEntityDef( attack.projectileActor );
	if (projectileDef == nullptr)
	{
		return;
	}

	Vec3 aimForward = GetAimDirection();
	EulerAngles orientation;
	orientation.m_yawDegrees = Atan2Degrees( aimForward.y, aimForward.x );
	orientation.m_pitchDegrees = -ASinDegrees( Clamp( aimForward.z, -1.f, 1.f ) );

	Vec3 spawnPos = GetChargingProjectileSpawnPosition();
	Entity* projectile = m_owner->m_map->SpawnActor( projectileDef, spawnPos, orientation );
	if (projectile == nullptr)
	{
		return;
	}

	projectile->SetSimulatePhysics( false );
	projectile->m_velocity = Vec3::ZERO;
	if (projectile->GetCollision() != nullptr)
	{
		projectile->GetCollision()->SetCollisionEnabled( false );
	}
	projectile->SetScale( 0.f );

	SetChargingProjectile( projectile );
}

void Weapon::UpdateChargingProjectile( float chargeFraction )
{
	Entity* charging = GetChargingProjectile();
	if (charging == nullptr)
	{
		return;
	}

	charging->SetScale( Clamp( chargeFraction, 0.f, 1.f ) );
	SyncChargingProjectile();
}

void Weapon::SyncChargingProjectile()
{
	Entity* charging = GetChargingProjectile();
	if (charging == nullptr)
	{
		return;
	}

	Vec3 aimForward = GetAimDirection();
	charging->m_orientation.m_yawDegrees = Atan2Degrees( aimForward.y, aimForward.x );
	charging->m_orientation.m_pitchDegrees = -ASinDegrees( Clamp( aimForward.z, -1.f, 1.f ) );
	charging->m_velocity = Vec3::ZERO;
	charging->m_position = GetChargingProjectileHeldPosition( charging );
	charging->SyncCollisionTransform();
}

void Weapon::ReleaseChargingProjectile( WeaponAttackDefinition const& attack, float chargeFraction )
{
	Entity* projectile = GetChargingProjectile();
	if (projectile == nullptr)
	{
		return;
	}

	SetChargingProjectile( nullptr );

	float const clampedFraction = Clamp( chargeFraction, 0.f, 1.f );
	float const squaredFraction = clampedFraction * clampedFraction; // y = x^2

	EntityDefinition const* projectileDef = projectile->GetDefinition();
	float const fullDamage = (projectileDef != nullptr) ? projectileDef->damageOnCollide : 0.f;
	float const fullImpulse = (projectileDef != nullptr) ? projectileDef->impulseOnCollide : 0.f;
	projectile->m_damageOnCollide = fullDamage * squaredFraction;
	projectile->m_impulseOnCollide = fullImpulse * squaredFraction;
	projectile->SetScale( clampedFraction );

	projectile->SetSimulatePhysics( true );
	if (projectile->GetCollision() != nullptr)
	{
		projectile->GetCollision()->SetCollisionEnabled( true );
	}

	Vec3 launchDir = GetAimDirection();
	projectile->m_position = GetChargingProjectileHeldPosition( projectile );
	projectile->m_previousPosition = projectile->m_position;
	projectile->m_orientation.m_yawDegrees = Atan2Degrees( launchDir.y, launchDir.x );
	projectile->m_orientation.m_pitchDegrees = -ASinDegrees( Clamp( launchDir.z, -1.f, 1.f ) );
	projectile->m_velocity = launchDir.GetNormalized() * attack.projectileSpeed;
	projectile->m_ignoreEmbeddedHits = true;
	projectile->SyncCollisionTransform();
}

void Weapon::DestroyChargingProjectile()
{
	Entity* charging = GetChargingProjectile();
	if (charging == nullptr)
	{
		m_chargingProjectileUID = EntityUID::INVALID;
		return;
	}

	if (m_owner != nullptr && m_owner->m_map != nullptr)
	{
		m_owner->m_map->DestroyEntity( charging );
	}
	m_chargingProjectileUID = EntityUID::INVALID;
}

void Weapon::ClearChargingProjectileRef( Entity* destroyedProjectile )
{
	if (destroyedProjectile == nullptr)
	{
		return;
	}

	// Compare by UID so we never touch a dangling Entity*.
	if (m_chargingProjectileUID.isValid() && destroyedProjectile->GetUID() == m_chargingProjectileUID)
	{
		m_chargingProjectileUID = EntityUID::INVALID;
	}
}

Entity* Weapon::GetChargingProjectile() const
{
	if (!m_chargingProjectileUID.isValid() || m_owner == nullptr || m_owner->m_map == nullptr)
	{
		return nullptr;
	}

	Entity* entity = m_owner->m_map->GetEntityByUID( m_chargingProjectileUID );
	if (entity == nullptr)
	{
		m_chargingProjectileUID = EntityUID::INVALID;
	}
	return entity;
}

void Weapon::SetChargingProjectile( Entity* projectile )
{
	if (projectile == nullptr)
	{
		m_chargingProjectileUID = EntityUID::INVALID;
		return;
	}
	m_chargingProjectileUID = projectile->GetUID();
}

float Weapon::GetOwnerRadius() const
{
	if (m_owner == nullptr || m_owner->GetDefinition() == nullptr)
	{
		return 0.f;
	}
	return m_owner->GetDefinition()->radius;
}

Vec3 Weapon::GetChargingProjectileSpawnPosition() const
{
	// camera - (0,0,0.1) + forward * playerRadius
	Vec3 const cameraPos = GetMuzzlePosition();
	Vec3 const forward = GetAimDirection();
	return cameraPos - Vec3( 0.f, 0.f, 0.1f ) + forward * GetOwnerRadius();
}

Vec3 Weapon::GetChargingProjectileHeldPosition( Entity const* projectile ) const
{
	Vec3 const cameraPos = GetMuzzlePosition();
	Vec3 const forward = GetAimDirection();
	float projectileRadius = 0.f;
	if (projectile != nullptr && projectile->GetDefinition() != nullptr)
	{
		projectileRadius = projectile->GetDefinition()->radius;
	}
	float const forwardDistance = GetOwnerRadius() + projectileRadius;
	return cameraPos - Vec3( 0.f, 0.f, 0.08f ) + forward * forwardDistance;
}

void Weapon::PerformMeleeStrike()
{
	if (m_definition == nullptr || m_definition->mainAttack.meleeCount <= 0)
	{
		return;
	}

	FireMelee( m_definition->mainAttack, false );
	PlayFireSound();
}

void Weapon::PlayFireSound()
{
	if (m_definition == nullptr || m_owner == nullptr || g_theAudio == nullptr)
	{
		return;
	}

	WeaponSoundDefinition const* fireSound = m_definition->GetSoundByType( "Fire" );
	if (fireSound == nullptr || fireSound->soundName.empty())
	{
		return;
	}

	SoundID const soundId = g_theAudio->CreateOrGetSound( fireSound->soundName, true );
	g_theAudio->StartSoundAt( soundId, m_owner->m_position, false, GetSfxVolume() );
}

float Weapon::GetTargetRadius( Entity const* target ) const
{
	if (target == nullptr || target->GetDefinition() == nullptr)
	{
		return 0.f;
	}
	return target->GetDefinition()->radius;
}

float Weapon::GetMeleeReachDistance( Entity const* target, WeaponAttackDefinition const& attack ) const
{
	return attack.meleeRange + GetOwnerRadius() + GetTargetRadius( target );
}

bool Weapon::IsTargetInMeleeReach( Entity const* target, WeaponAttackDefinition const& attack ) const
{
	if (m_owner == nullptr || target == nullptr || target->m_isDead)
	{
		return false;
	}
	if (!CanDamageTarget( target ))
	{
		return false;
	}

	Vec3 toTarget = target->m_position - m_owner->m_position;
	toTarget.z = 0.f;
	float const dist = toTarget.GetLength();
	return dist <= GetMeleeReachDistance( target, attack );
}

bool Weapon::IsTargetInMeleeArc( Entity const* target, WeaponAttackDefinition const& attack ) const
{
	if (!IsTargetInMeleeReach( target, attack ))
	{
		return false;
	}

	Vec3 toTarget = target->m_position - m_owner->m_position;
	toTarget.z = 0.f;
	float const dist = toTarget.GetLength();
	if (dist <= 0.0001f)
	{
		return false;
	}

	Vec3 forward = GetAimDirection();
	forward.z = 0.f;
	if (forward.GetLengthSquared() <= 0.0001f)
	{
		return false;
	}
	forward = forward.GetNormalized();

	Vec3 const toTargetDir = toTarget / dist;
	float const halfArc = attack.meleeArc * 0.5f;
	float const angleDegrees = GetAngleDegreesBetweenVectors2D( forward.GetXY(), toTargetDir.GetXY() );
	return angleDegrees <= halfArc;
}

void Weapon::FireMelee( WeaponAttackDefinition const& attack, bool applyKnockback )
{
	if (m_owner == nullptr || m_owner->m_map == nullptr)
	{
		return;
	}

	int hitsRemaining = attack.meleeCount;

	for (Entity* entity : m_owner->m_map->GetEntities())
	{
		if (hitsRemaining <= 0)
		{
			break;
		}
		if (!IsTargetInMeleeArc( entity, attack ))
		{
			continue;
		}

		Vec3 toTarget = entity->m_position - m_owner->m_position;
		toTarget.z = 0.f;
		float const dist = toTarget.GetLength();
		Vec3 const toTargetDir = toTarget / dist;

		entity->TakeDamage( attack.meleeDamage );
		if (applyKnockback)
		{
			entity->ApplyKnockback( toTargetDir, ENTITY_MELEE_KNOCKBACK_DISTANCE );
		}
		--hitsRemaining;
	}
}

Vec3 Weapon::GetMuzzlePosition() const
{
	if (m_owner == nullptr)
	{
		return Vec3::ZERO;
	}

	float eyeHeight = 0.5f;
	EntityDefinition const* definition = m_owner->GetDefinition();
	if (definition != nullptr && definition->eyeHeight > 0.f)
	{
		eyeHeight = definition->eyeHeight;
	}

	PlayerController* playerController = dynamic_cast<PlayerController*>( m_owner->GetController() );
	if (playerController != nullptr)
	{
		return playerController->m_camera.m_position;
	}

	return m_owner->m_position + Vec3( 0.f, 0.f, eyeHeight );
}

Vec3 Weapon::GetAimDirection() const
{
	if (m_owner == nullptr)
	{
		return Vec3( 1.f, 0.f, 0.f );
	}

	PlayerController* playerController = dynamic_cast<PlayerController*>( m_owner->GetController() );
	if (playerController != nullptr)
	{
		return playerController->m_camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
	}

	return m_owner->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
}

Vec3 Weapon::GetDirectionInCone( Vec3 const& forward, float coneDegrees ) const
{
	Vec3 result = forward.GetNormalized();
	if (coneDegrees <= 0.f || g_theRNG == nullptr)
	{
		return result;
	}

	float yawOffset = g_theRNG->RollRandomFloatInRange( -coneDegrees, coneDegrees );
	float pitchOffset = g_theRNG->RollRandomFloatInRange( -coneDegrees, coneDegrees );

	Mat44 rotation = Mat44::CreateZRotationDegrees( yawOffset );
	rotation.Append( Mat44::CreateYRotationDegrees( pitchOffset ) );
	return rotation.TransformVectorQuantity3D( result ).GetNormalized();
}

bool Weapon::CanDamageTarget( Entity const* target ) const
{
	if (m_owner == nullptr || target == nullptr || target == m_owner || target->m_isDead)
	{
		return false;
	}
	if (target->GetCollision() != nullptr && !target->GetCollision()->IsCollisionEnabled())
	{
		return false;
	}

	EntityDefinition const* ownerDef = m_owner->GetDefinition();
	EntityDefinition const* targetDef = target->GetDefinition();
	if (targetDef != nullptr && (targetDef->dieOnCollide || targetDef->dieOnSpawn))
	{
		return false;
	}
	if (ownerDef == nullptr || targetDef == nullptr)
	{
		return true;
	}

	if (ownerDef->faction == Faction::NEUTRAL || targetDef->faction == Faction::NEUTRAL)
	{
		return false;
	}

	return ownerDef->faction != targetDef->faction;
}
