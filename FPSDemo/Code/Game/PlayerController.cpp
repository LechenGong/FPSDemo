#include "Game/PlayerController.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Weapon.hpp"
#include "Game/Map.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include <unordered_map>

void PlayerController::Possess( Entity* entity )
{
	Controller::Possess( entity );
	SetupCamera();
	SetupHudCamera();
}

void PlayerController::SetupCamera()
{
	if (m_entity == nullptr)
	{
		return;
	}

	float cameraFov = 60.f;
	EntityDefinition const* definition = m_entity->GetDefinition();
	if (definition != nullptr && definition->cameraFOV > 0.f)
	{
		cameraFov = definition->cameraFOV;
	}

	m_camera.SetPerspeciveView( g_theWindow->GetAspect(), cameraFov, 0.1f, 100.f );
	m_camera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	UpdateCameraFromEntity();
}

void PlayerController::SetupHudCamera()
{
	m_hudCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ), 0.f, 1.f );
	m_hudCamera.SetTransform( Vec3::ZERO, EulerAngles() );
	m_hudCamera.SetRenderBasis( Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ) );
}

void PlayerController::UpdateCameraFromEntity( float deltaSeconds )
{
	if (m_entity == nullptr)
	{
		return;
	}

	if (m_entity->m_isDead)
	{
		UpdateDeathCamera( deltaSeconds );
		return;
	}

	float eyeHeight = 0.f;
	float cameraFov = 60.f;
	EntityDefinition const* definition = m_entity->GetDefinition();
	if (definition != nullptr)
	{
		eyeHeight = definition->eyeHeight;
		if (definition->cameraFOV > 0.f)
		{
			cameraFov = definition->cameraFOV;
		}
	}

	m_camera.m_position = m_entity->m_position + Vec3( 0.f, 0.f, eyeHeight );
	m_camera.m_orientation.m_yawDegrees = m_entity->m_orientation.m_yawDegrees;
	m_camera.m_orientation.m_rollDegrees = 0.f;
	m_camera.SetFov( cameraFov );
}

void PlayerController::UpdateDeathCamera( float deltaSeconds )
{
	if (m_entity == nullptr)
	{
		return;
	}

	if (!m_deathFallStarted)
	{
		m_deathFallStarted = true;
		m_deathFallSeconds = 0.f;
		m_deathFallPitchStart = m_camera.m_orientation.m_pitchDegrees;
		m_deathFallSide = (g_theRNG != nullptr && g_theRNG->RollRandomIntInRange( 0, 1 ) == 0) ? -1.f : 1.f;
		m_entity->m_velocity.x = 0.f;
		m_entity->m_velocity.y = 0.f;
	}

	m_deathFallSeconds += deltaSeconds;
	constexpr float deathFallDuration = 1.35f;
	constexpr float deathFallRoll = 82.f;
	constexpr float deathFallEyeEnd = 0.12f;
	float t = (deathFallDuration > 0.f) ? Clamp( m_deathFallSeconds / deathFallDuration, 0.f, 1.f ) : 1.f;
	t = SmoothStep3( t );

	float eyeHeight = 0.5f;
	float cameraFov = 60.f;
	EntityDefinition const* definition = m_entity->GetDefinition();
	if (definition != nullptr)
	{
		eyeHeight = definition->eyeHeight;
		if (definition->cameraFOV > 0.f)
		{
			cameraFov = definition->cameraFOV;
		}
	}

	Vec3 const worldUp( 0.f, 0.f, 1.f );
	EulerAngles yawOnly = m_entity->m_orientation;
	yawOnly.m_pitchDegrees = 0.f;
	yawOnly.m_rollDegrees = 0.f;
	Vec3 forwardXY( yawOnly.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetXY() );
	if (forwardXY.GetLengthSquared() < 0.0001f)
	{
		forwardXY = Vec3( 1.f, 0.f, 0.f );
	}
	else
	{
		forwardXY = forwardXY.GetNormalized();
	}
	Vec3 const left = CrossProduct3D( worldUp, forwardXY ).GetNormalized();
	Vec3 const standingOffset = worldUp * eyeHeight;
	Vec3 const fallenOffset = (left * m_deathFallSide * 0.18f) + (worldUp * deathFallEyeEnd);
	m_camera.m_position = m_entity->m_position + Interpolate( standingOffset, fallenOffset, t );
	m_camera.m_orientation.m_yawDegrees = m_entity->m_orientation.m_yawDegrees;
	m_camera.m_orientation.m_pitchDegrees = Interpolate( m_deathFallPitchStart, 12.f, t );
	m_camera.m_orientation.m_rollDegrees = deathFallRoll * m_deathFallSide * t;
	m_camera.SetFov( cameraFov );

	m_entity->m_orientation.m_rollDegrees = m_camera.m_orientation.m_rollDegrees;
}

void PlayerController::Update( float deltaSeconds )
{
	if (m_entity == nullptr)
	{
		return;
	}

	m_entity->m_moveIntentXY = Vec3::ZERO;

	if (m_entity->m_isDead)
	{
		return;
	}

	InputResponse( deltaSeconds );
	UpdateCameraFromEntity( deltaSeconds );
	UpdateWeaponInput( deltaSeconds );
	UpdateShopInput();

	if (m_dashCooldownTimer > 0.f)
	{
		m_dashCooldownTimer -= deltaSeconds;
	}
}

void PlayerController::UpdateWeaponInput( float deltaSeconds )
{
	if (m_entity == nullptr || m_entity->m_isDead)
	{
		return;
	}

	int weaponCount = m_entity->GetWeaponCount();
	if (weaponCount <= 0)
	{
		return;
	}

	if (g_theInput->IsNewKeyPressed( "firstWeapon" ))
	{
		m_entity->EquipWeapon( 0 );
	}
	if (g_theInput->IsNewKeyPressed( "secondWeapon" ))
	{
		m_entity->EquipWeapon( 1 );
	}
	if (g_theInput->IsNewKeyPressed( "nextWeapon" ))
	{
		int nextIndex = (m_entity->GetCurrentWeaponIndex() + 1) % weaponCount;
		m_entity->EquipWeapon( nextIndex );
	}
	if (g_theInput->IsNewKeyPressed( "previousWeapon" ))
	{
		int prevIndex = (m_entity->GetCurrentWeaponIndex() + weaponCount - 1) % weaponCount;
		m_entity->EquipWeapon( prevIndex );
	}

	Weapon* weapon = m_entity->GetCurrentWeapon();
	if (weapon == nullptr)
	{
		return;
	}

	if (g_theInput->IsNewKeyPressed( "reload" ) || g_theInput->IsNewKeyPressed( 'R' ))
	{
		weapon->TryBeginReload();
	}

	bool mainDown = g_theInput->IsKeyDown( "mainAttack" );
	bool mainReleased = g_theInput->WasKeyJustReleased( "mainAttack" );
	weapon->UpdateAttackInput( WeaponAttackSlot::Main, mainDown, mainReleased, deltaSeconds );

	if (weapon->HasSubAttack())
	{
		bool subDown = g_theInput->IsKeyDown( "subAttack" );
		bool subReleased = g_theInput->WasKeyJustReleased( "subAttack" );
		weapon->UpdateAttackInput( WeaponAttackSlot::Sub, subDown, subReleased, deltaSeconds );
	}
}

void PlayerController::UpdateShopInput()
{
	if (m_entity == nullptr || m_entity->m_isDead || m_entity->m_map == nullptr || g_theInput == nullptr)
	{
		return;
	}

	if (!m_entity->m_map->IsInsideShopRange( m_entity->m_position ))
	{
		return;
	}

	std::unordered_map<std::string, Key> const bindings = g_theInput->GetKeybinding();
	auto const isBindingPressed = [&bindings]( char const* name ) -> bool
	{
		auto found = bindings.find( name );
		if (found == bindings.end() || found->second.value == 0)
		{
			return false;
		}
		return g_theInput->IsNewKeyPressed( name );
	};

	if (isBindingPressed( "interactButton1" ))
	{
		float const maxHealth = (m_entity->m_definition != nullptr) ? m_entity->m_definition->health : m_entity->m_health;
		if (m_entity->m_health >= maxHealth)
		{
			DebugAddMessage( "Shop: health already full", 2.f, Rgba8::RED, Rgba8::RED );
		}
		else if (!m_entity->m_map->TrySpendMoney( SHOP_HEALTH_COST ))
		{
			DebugAddMessage( Stringf( "Shop: need $%d", SHOP_HEALTH_COST ), 2.f, Rgba8::RED, Rgba8::RED );
		}
		else
		{
			m_entity->AddHealth( (float)SHOP_HEALTH_RESTORE );
			m_entity->m_map->NotifyHealPurchased();
			DebugAddMessage( Stringf( "Shop: +%d HP", SHOP_HEALTH_RESTORE ), 2.f, Rgba8::GREEN, Rgba8::GREEN );
		}
	}
	if (isBindingPressed( "interactButton2" ))
	{
		bool hasAmmoWeapon = false;
		for (Weapon* weapon : m_entity->m_weapons)
		{
			if (weapon != nullptr && weapon->UsesAmmo())
			{
				hasAmmoWeapon = true;
				break;
			}
		}
		if (!hasAmmoWeapon)
		{
			DebugAddMessage( "Shop: no ammo weapons", 2.f, Rgba8::RED, Rgba8::RED );
		}
		else if (!m_entity->m_map->TrySpendMoney( SHOP_AMMO_COST ))
		{
			DebugAddMessage( Stringf( "Shop: need $%d", SHOP_AMMO_COST ), 2.f, Rgba8::RED, Rgba8::RED );
		}
		else
		{
			for (Weapon* weapon : m_entity->m_weapons)
			{
				if (weapon == nullptr || !weapon->UsesAmmo())
				{
					continue;
				}
				weapon->AddReserveAmmo( weapon->GetMagazineSize() * SHOP_AMMO_MAGAZINES );
			}
			DebugAddMessage( Stringf( "Shop: +%d mag all weapons", SHOP_AMMO_MAGAZINES ), 2.f, Rgba8::GREEN, Rgba8::GREEN );
			m_entity->m_map->NotifyAmmoPurchased();
		}
	}
}

void PlayerController::InputResponse( float deltaSeconds )
{
	if (m_entity == nullptr)
	{
		return;
	}

	if (Window::IsWindowFocused())
	{
		m_entity->m_orientation.m_yawDegrees += g_theInput->GetCursorClientDelta().x * -0.075f;
		m_camera.m_orientation.m_pitchDegrees += g_theInput->GetCursorClientDelta().y * 0.075f;
	}

	Vec3 forwardVec = Vec3( m_entity->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetXY() ).GetNormalized();
	Vec3 upVec = Vec3( 0.f, 0.f, 1.f );
	Vec3 leftVec = CrossProduct3D( upVec, forwardVec );

	float walkSpeed = 2.f;
	float runSpeed = 4.f;
	EntityDefinition const* definition = m_entity->GetDefinition();
	if (definition != nullptr)
	{
		if (definition->walkSpeed > 0.f)
		{
			walkSpeed = definition->walkSpeed;
		}
		if (definition->runSpeed > 0.f)
		{
			runSpeed = definition->runSpeed;
		}
	}

	float ifSprint = walkSpeed;
	if (m_sprintEnabled && g_theInput->GetKeyValueByName( "sprint" ) != 0 && g_theInput->IsKeyDown( "sprint" ))
	{
		ifSprint = runSpeed;
	}

	Vec3 force = Vec3::ZERO;

	if (g_theInput->IsKeyDown( "forward" ))
	{
		force += forwardVec * ifSprint;
	}
	if (g_theInput->IsKeyDown( "backward" ))
	{
		force += forwardVec * -ifSprint;
	}
	if (g_theInput->IsKeyDown( "leftward" ))
	{
		force += leftVec * ifSprint;
	}
	if (g_theInput->IsKeyDown( "rightward" ))
	{
		force += leftVec * -ifSprint;
	}

	if (g_theInput->IsNewKeyPressed( "jump" ))
	{
		if (m_entity->IsGrounded() && m_entity->m_velocity.z <= 0.f)
		{
			// Strong upward impulse. Also clear the ground-snap band this frame so
			// high FPS cannot immediately re-stick before vertical motion accumulates.
			m_entity->m_velocity.z = ENTITY_JUMP_SPEED;
			m_entity->m_position.z += ENTITY_GROUND_LAND_DISTANCE + 0.05f;
			m_entity->m_isGrounded = false;
		}
	}

	m_camera.m_orientation.m_pitchDegrees = Clamp( m_camera.m_orientation.m_pitchDegrees, -85.f, 85.f );

	float forceLength = force.GetLength();
	if (m_dashEnabled && g_theInput->IsNewKeyPressed( "Dash" ) && m_dashCooldownTimer <= 0.f)
	{
		Vec3 dashDir = (forceLength == 0.f) ? forwardVec : (force / forceLength);
		m_entity->m_velocity += dashDir * 15.f;
		m_dashCooldownTimer = 5.f;
	}

	if (forceLength == 0.f)
	{
		return;
	}

	Vec3 forceNormal = force / forceLength;
	if (forceLength > ifSprint)
	{
		forceLength = ifSprint;
	}

	Vec3 const walkDelta = forceNormal * forceLength * deltaSeconds;
	m_entity->m_moveIntentXY += walkDelta;
	m_entity->m_position += walkDelta;
}
