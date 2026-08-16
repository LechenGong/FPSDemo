#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/MapGeometryBaker.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/EntityUID.hpp"
#include "Game/Controller.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Weapon.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RaycastUtil.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace
{
	constexpr float HUD_STATUS_PAD = 24.f;
	constexpr float HUD_STATUS_ICON_SIZE = 40.f;
	constexpr float HUD_STATUS_ICON_TEXT_GAP = 10.f;
	constexpr float HUD_STATUS_TEXT_HEIGHT = 32.f;
	constexpr float HUD_STATUS_TEXT_ASPECT = 0.7f;
	constexpr float HUD_SHOP_PROMPT_HEIGHT = 28.f;
	constexpr float HUD_SHOP_PROMPT_ASPECT = 0.7f;
	constexpr float HUD_SHOP_PROMPT_TOP_PAD = 28.f;
	constexpr float HUD_SHOP_PROMPT_LINE_GAP = 8.f;
	constexpr float HUD_SURVIVAL_TEXT_HEIGHT = 36.f;
	constexpr float HUD_SURVIVAL_TEXT_ASPECT = 0.7f;
	constexpr float HUD_SURVIVAL_LEFT_PAD = 48.f;
	constexpr float HUD_INTRO_TEXT_HEIGHT = 140.f;
	constexpr float HUD_SURVIVE_TEXT_HEIGHT = 96.f;
	constexpr float HUD_BANNER_TEXT_HEIGHT = 120.f;
	constexpr float HUD_INTRO_BEAT_SECONDS = 0.85f;
	constexpr float HUD_SURVIVE_BEAT_SECONDS = 0.9f;
	constexpr float HUD_BANNER_DURATION = 0.7f;
	constexpr float START_TRIGGER_HIT_PAD = 0.15f;
	char const* const HUD_INTRO_STEPS[] = { "3", "2", "1", "SURVIVE" };
	constexpr int HUD_INTRO_STEP_COUNT = 4;

	char const* const SHOP_OFFER_HEALTH = "20HP for $100";
	char const* const SHOP_OFFER_AMMO = "2 Mags all guns for $80";
	constexpr float HUD_END_FADE_SECONDS = 1.6f;
	constexpr float HUD_END_TITLE_HEIGHT = 96.f;
	constexpr float HUD_END_PROMPT_HEIGHT = 32.f;
	constexpr float HUD_END_MAX_OVERLAY_ALPHA = 200.f;
	constexpr float HUD_END_TITLE_X_OFFSET = -180.f;
	constexpr int HUD_END_STAT_COUNT = 6;
	constexpr float HUD_END_STAT_START_SECONDS = 0.28f;
	constexpr float HUD_END_STAT_STAGGER_SECONDS = 0.18f;
	constexpr float HUD_END_STAT_POP_SECONDS = 0.32f;
	constexpr float HUD_END_STAT_HEIGHT = 30.f;
	constexpr float HUD_END_STAT_LINE_GAP = 12.f;
	constexpr float HUD_END_STAT_GAP_FROM_TITLE = 44.f;
	constexpr float PLAYER_CROWD_PUSH_SHARE = 0.55f;
	constexpr float PLAYER_CROWD_INCOMING_DOT = 0.2f;
	constexpr float PLAYER_CROWD_MAX_STEP_FRACTION = 0.75f;

	// Hits reported at the camera (ray/projectile started inside the target).
	constexpr float HIT_VFX_NEAR_VIEWER_DIST = 0.15f;
	// Sit just outside the surface so the billboard isn't inside the actor sprite.
	constexpr float HIT_VFX_SURFACE_OFFSET = 0.06f;

	void RenderHudIconText( Vec2 const& iconMins, Texture* icon, SamplerMode iconSampler, std::string const& text, Rgba8 const& textTint )
	{
		Shader* shader = g_theRenderer->CreateShader( "Default", VertexType::VERTEX_PCU );
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindShader( shader );

		if (icon != nullptr)
		{
			AABB2 const iconBounds( iconMins, iconMins + Vec2( HUD_STATUS_ICON_SIZE, HUD_STATUS_ICON_SIZE ) );
			std::vector<Vertex_PCU> iconVerts;
			AddVertsForAABB2D( iconVerts, iconBounds, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
			g_theRenderer->SetSamplerMode( iconSampler );
			g_theRenderer->BindTexture( icon );
			g_theRenderer->DrawVertexArray( iconVerts );
		}

		BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
		if (font == nullptr)
		{
			return;
		}

		float const textY = iconMins.y + (HUD_STATUS_ICON_SIZE - HUD_STATUS_TEXT_HEIGHT) * 0.5f;
		Vec2 const textMins( iconMins.x + HUD_STATUS_ICON_SIZE + HUD_STATUS_ICON_TEXT_GAP, textY );
		std::vector<Vertex_PCU> textVerts;
		font->AddVertsForText2D( textVerts, textMins, HUD_STATUS_TEXT_HEIGHT, text, textTint, HUD_STATUS_TEXT_ASPECT );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->BindTexture( &font->GetTexture() );
		g_theRenderer->DrawVertexArray( textVerts );
	}

	std::string GetInteractButtonLabel( std::string const& bindingName, char const* unboundFallback )
	{
		if (g_theInput == nullptr)
		{
			return unboundFallback;
		}

		std::unordered_map<std::string, Key> const bindings = g_theInput->GetKeybinding();
		auto found = bindings.find( bindingName );
		if (found == bindings.end() || found->second.value == 0)
		{
			return unboundFallback;
		}

		std::string const label = g_theInput->GetButtonNameByValue( found->second.value );
		if (label.empty())
		{
			return unboundFallback;
		}
		return label;
	}
}

Map::Map( Game* owner, std::string const& mapName )
	: m_game( owner )
	, m_mapName( mapName )
{
}

Map::~Map()
{
	Shutdown();
}

void Map::Startup()
{
	MapDefinition::InitializeDefinitions();
	m_definition = MapDefinition::GetByName( m_mapName );
	GUARANTEE_OR_DIE( m_definition != nullptr, Stringf( "Unknown map definition: %s", m_mapName.c_str() ).c_str() );

	BuildGeometry();

	for (SpawnInfo const& spawnInfo : m_definition->m_spawnInfos)
	{
		SpawnActor( spawnInfo );
	}

	BuildScheduledSurvivalWaves();
	CacheStartTriggerBounds();
}

void Map::Update( float deltaSeconds )
{
	if (m_survivalVictory)
	{
		UpdateMatchEndScreen( deltaSeconds );
		return;
	}

	UpdateSurvivalWaves( deltaSeconds );

	// Index-based iteration: controllers may SpawnActor/DestroyEntity mid-frame.
	// Range-for iterators are invalidated if m_entities reallocates.
	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		Entity* entity = m_entities[i];
		if (entity == nullptr)
		{
			continue;
		}

		if (entity->m_controller != nullptr)
		{
			entity->m_controller->Update( deltaSeconds );
		}
	}

	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		Entity* entity = m_entities[i];
		if (entity == nullptr)
		{
			continue;
		}

		entity->Update( deltaSeconds );
	}

	CollidePhysics();
	HandleProjectileCollisions();
	HandleCorpseCleanup( deltaSeconds );

	CheckSurvivalDefeat();
	CheckSurvivalVictory();
	if (m_survivalDefeat)
	{
		UpdateMatchEndScreen( deltaSeconds );
	}

	if (m_playerController != nullptr)
	{
		m_playerController->UpdateCameraFromEntity( deltaSeconds );
		Entity* playerEntity = m_playerController->GetEntity();
		Weapon* weapon = (playerEntity != nullptr) ? playerEntity->GetCurrentWeapon() : nullptr;
		if (weapon != nullptr)
		{
			weapon->SyncChargingProjectile();
		}
	}

	for (NavMesh const& navMesh : m_navMeshes)
	{
		navMesh.DebugRender();
	}

	DebugDrawNavPathQuery();
}

void Map::Render() const
{
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );

	for (MapRenderBatch const& batch : m_renderBatches)
	{
		g_theRenderer->BindTexture( batch.m_texture );
		g_theRenderer->SetSamplerMode( batch.m_samplerMode );
		g_theRenderer->DrawVertexArray( batch.m_verts );
	}

	for (Entity* entity : m_entities)
	{
		if (entity != nullptr && entity->m_definition != nullptr && entity->m_definition->visible)
		{
			entity->Render();
		}
	}

	RenderInteractableOverlays();

	// UE-style navmesh walkable overlay (respects DRToggle).
	for (NavMesh const& navMesh : m_navMeshes)
	{
		navMesh.RenderWalkableOverlay();
	}

	// Restore opaque state for any subsequent world draws this frame.
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
}

void Map::RenderHUD() const
{
	// Screen-space overlay: no depth test against the world, identity model.
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetModelConstants();

	bool const matchOver = m_survivalVictory || m_survivalDefeat;
	Entity* playerEntity = (m_playerController != nullptr) ? m_playerController->GetEntity() : nullptr;
	bool const playerAlive = (playerEntity != nullptr && !playerEntity->m_isDead);

	if (!matchOver && playerAlive)
	{
		Weapon* weapon = playerEntity->GetCurrentWeapon();
		if (weapon != nullptr)
		{
			weapon->Render();
		}

		Vec2 const hudSize = m_playerController->m_hudCamera.GetDimensions();

		int const currentHealth = (int)playerEntity->m_health;
		int const maxHealth = (playerEntity->m_definition != nullptr) ? (int)playerEntity->m_definition->health : currentHealth;
		Rgba8 const healthTint = (currentHealth > 0) ? Rgba8::WHITE : Rgba8::RED;
		RenderHudIconText(
			Vec2( HUD_STATUS_PAD, HUD_STATUS_PAD ),
			g_textures[TEXTURE_HUD_HEART],
			SamplerMode::POINT_CLAMP,
			Stringf( "%d / %d", currentHealth, maxHealth ),
			healthTint );

		if (weapon != nullptr && weapon->UsesAmmo())
		{
			std::string const ammoText = Stringf( "%d / %d", weapon->GetMagazineAmmo(), weapon->GetReserveAmmo() );
			BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
			float textWidth = (float)ammoText.length() * HUD_STATUS_TEXT_HEIGHT * HUD_STATUS_TEXT_ASPECT;
			if (font != nullptr)
			{
				textWidth = font->GetTextWidth( HUD_STATUS_TEXT_HEIGHT, ammoText, HUD_STATUS_TEXT_ASPECT );
			}
			float const groupWidth = HUD_STATUS_ICON_SIZE + HUD_STATUS_ICON_TEXT_GAP + textWidth;
			Rgba8 const ammoTint = (weapon->GetMagazineAmmo() > 0) ? Rgba8::WHITE : Rgba8::RED;
			RenderHudIconText(
				Vec2( hudSize.x - HUD_STATUS_PAD - groupWidth, HUD_STATUS_PAD ),
				g_textures[TEXTURE_HUD_AMMO],
				SamplerMode::BILINEAR_CLAMP,
				ammoText,
				ammoTint );
		}

		std::string const moneyText = Stringf( "%d", m_money );
		BitmapFont* moneyFont = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
		float moneyTextWidth = (float)moneyText.length() * HUD_STATUS_TEXT_HEIGHT * HUD_STATUS_TEXT_ASPECT;
		if (moneyFont != nullptr)
		{
			moneyTextWidth = moneyFont->GetTextWidth( HUD_STATUS_TEXT_HEIGHT, moneyText, HUD_STATUS_TEXT_ASPECT );
		}
		float const moneyGroupWidth = HUD_STATUS_ICON_SIZE + HUD_STATUS_ICON_TEXT_GAP + moneyTextWidth;
		RenderHudIconText(
			Vec2( hudSize.x - HUD_STATUS_PAD - moneyGroupWidth, hudSize.y - HUD_STATUS_PAD - HUD_STATUS_ICON_SIZE ),
			g_textures[TEXTURE_HUD_COIN],
			SamplerMode::BILINEAR_CLAMP,
			moneyText,
			Rgba8::WHITE );

		RenderShopPrompt();
		RenderSurvivalHud();
	}
	else if (!matchOver)
	{
		RenderSurvivalHud();
	}

	RenderMatchEndOverlay();
}

void Map::Shutdown()
{
	m_playerController = nullptr;

	for (Entity*& entity : m_entities)
	{
		delete entity;
		entity = nullptr;
	}
	m_entities.clear();
	m_entitySalt = 0;

	m_solidBounds.clear();
	m_solidObbs.clear();
	m_solidBoundsNavWalkable.clear();
	m_solidObbsNavWalkable.clear();
	m_renderBatches.clear();
	m_navMeshes.clear();
	m_interactableOverlayVerts.clear();
	m_definition = nullptr;
	m_money = 0;
	m_scheduledWaves.clear();
	m_overtimeGroupSizes.clear();
	m_survivalSeconds = 0.f;
	m_nextScheduledWaveIndex = 0;
	m_overtimeGroupIndex = 0;
	m_overtimeGroupTimer = 0.f;
	m_overtimeActive = false;
	m_overtimeFinishedSpawning = false;
	m_survivalVictory = false;
	m_survivalDefeat = false;
	m_endScreenSeconds = 0.f;
	m_endScreenFade = 0.f;
	m_endPromptVisible = false;
	m_killCount = 0;
	m_healPurchaseCount = 0;
	m_ammoPurchaseCount = 0;
	m_damageTaken = 0.f;
	m_resultKills = 0;
	m_resultClearSeconds = 0.f;
	m_resultDamageTaken = 0;
	m_resultHealth = 0;
	m_resultHeals = 0;
	m_resultAmmoBuys = 0;
	m_hasStartTrigger = false;
	m_startTriggered = false;
	m_introActive = false;
	m_combatStarted = false;
	m_introSeconds = 0.f;
	m_introStep = 0;
	m_shownBanner02 = false;
	m_shownBanner01 = false;
	m_shownBannerOvertime = false;
	m_minuteBannerText.clear();
	m_minuteBannerAge = 0.f;
	m_destroyedGeometryNames.clear();
}

Entity* Map::SpawnActor( SpawnInfo const& spawnInfo )
{
	EntityDefinition const* definition = EntityDefinition::GetEntityDef( spawnInfo.m_entityTypeName );
	GUARANTEE_OR_DIE( definition != nullptr, Stringf( "Unknown entity definition: %s", spawnInfo.m_entityTypeName.c_str() ).c_str() );
	return SpawnActor( definition, spawnInfo.m_position, spawnInfo.m_orientation );
}

bool Map::IsInsideShopRange( Vec3 const& position ) const
{
	if (m_definition == nullptr)
	{
		return false;
	}

	for (MapInteractableDefinition const& interactable : m_definition->m_interactables)
	{
		if (interactable.m_type != "Shop")
		{
			continue;
		}

		float const dx = position.x - interactable.m_center.x;
		float const dy = position.y - interactable.m_center.y;
		float const radius = interactable.m_radius;
		if ((dx * dx + dy * dy) <= (radius * radius))
		{
			return true;
		}
	}
	return false;
}

void Map::AddMoney( int amount )
{
	if (amount <= 0)
	{
		return;
	}
	m_money += amount;
}

bool Map::TrySpendMoney( int amount )
{
	if (amount <= 0)
	{
		return true;
	}
	if (m_money < amount)
	{
		return false;
	}
	m_money -= amount;
	return true;
}

void Map::NotifyHealPurchased()
{
	++m_healPurchaseCount;
}

void Map::NotifyAmmoPurchased()
{
	++m_ammoPurchaseCount;
}

void Map::NotifyPlayerDamaged( Entity* victim, float amount )
{
	if (amount <= 0.f || !IsPlayerEntity( victim ))
	{
		return;
	}

	m_damageTaken += amount;
}

bool Map::TogglePlayerInvincible()
{
	if (m_playerController == nullptr)
	{
		return false;
	}
	Entity* playerEntity = m_playerController->GetEntity();
	if (playerEntity == nullptr || playerEntity->m_isDead)
	{
		return false;
	}

	playerEntity->SetInvincible( !playerEntity->IsInvincible() );
	return playerEntity->IsInvincible();
}

int Map::KillAllDemons()
{
	std::vector<Entity*> demons;
	demons.reserve( m_entities.size() );
	for (Entity* entity : m_entities)
	{
		if (entity == nullptr || entity->m_isDead || entity->GetDefinition() == nullptr)
		{
			continue;
		}
		if (entity->GetDefinition()->faction != Faction::DEMON)
		{
			continue;
		}
		demons.push_back( entity );
	}

	int killed = 0;
	for (Entity* demon : demons)
	{
		if (demon == nullptr || demon->m_isDead)
		{
			continue;
		}
		float const damage = (demon->m_health > 0.f) ? demon->m_health : 1.f;
		demon->TakeDamage( damage );
		if (demon->m_isDead)
		{
			++killed;
		}
	}
	return killed;
}

void Map::NotifyEntityKilled( Entity* victim )
{
	if (victim == nullptr || victim->GetDefinition() == nullptr)
	{
		return;
	}
	if (m_playerController != nullptr && m_playerController->GetEntity() == victim)
	{
		BeginDefeat();
		return;
	}
	if (victim->GetDefinition()->faction != Faction::DEMON)
	{
		return;
	}
	if (!m_survivalVictory && !m_survivalDefeat)
	{
		++m_killCount;
	}
	AddMoney( MAP_MONEY_PER_KILL );
	CheckSurvivalVictory();
}

void Map::BuildScheduledSurvivalWaves()
{
	m_scheduledWaves.clear();
	m_overtimeGroupSizes.clear();
	m_survivalSeconds = 0.f;
	m_nextScheduledWaveIndex = 0;
	m_overtimeGroupIndex = 0;
	m_overtimeGroupTimer = 0.f;
	m_overtimeActive = false;
	m_overtimeFinishedSpawning = false;
	m_survivalVictory = false;
	m_survivalDefeat = false;
	m_endScreenSeconds = 0.f;
	m_endScreenFade = 0.f;
	m_endPromptVisible = false;
	m_killCount = 0;
	m_healPurchaseCount = 0;
	m_ammoPurchaseCount = 0;
	m_damageTaken = 0.f;
	m_resultKills = 0;
	m_resultClearSeconds = 0.f;
	m_resultDamageTaken = 0;
	m_resultHealth = 0;
	m_resultHeals = 0;
	m_resultAmmoBuys = 0;

	if (m_definition == nullptr || !m_definition->m_survivalWaves.m_enabled)
	{
		return;
	}

	SurvivalWaveDefinition const& waves = m_definition->m_survivalWaves;
	for (SurvivalPhaseDefinition const& phase : waves.m_phases)
	{
		if (phase.m_waves > 0)
		{
			for (int waveIndex = 0; waveIndex < phase.m_waves; ++waveIndex)
			{
				ScheduledSurvivalWave scheduled;
				scheduled.m_time = phase.m_startTime + (float)waveIndex * phase.m_period;
				scheduled.m_count = phase.m_count;
				m_scheduledWaves.push_back( scheduled );
			}
		}
		else
		{
			for (float time = phase.m_startTime; time < phase.m_endTime; time += phase.m_period)
			{
				ScheduledSurvivalWave scheduled;
				scheduled.m_time = time;
				scheduled.m_count = phase.m_count;
				m_scheduledWaves.push_back( scheduled );
			}
		}
	}

	std::sort( m_scheduledWaves.begin(), m_scheduledWaves.end(),
		[]( ScheduledSurvivalWave const& a, ScheduledSurvivalWave const& b )
		{
			return a.m_time < b.m_time;
		} );

	int remaining = waves.m_overtime.m_count;
	if (!waves.m_overtime.m_groupSizes.empty())
	{
		for (int groupSize : waves.m_overtime.m_groupSizes)
		{
			if (groupSize <= 0 || remaining <= 0)
			{
				continue;
			}
			int const clamped = (groupSize < remaining) ? groupSize : remaining;
			m_overtimeGroupSizes.push_back( clamped );
			remaining -= clamped;
		}
	}
	if (remaining > 0)
	{
		int const slotCount = (int)waves.m_slotXs.size();
		int const chunk = (slotCount > 0) ? slotCount : remaining;
		while (remaining > 0)
		{
			int const groupSize = (remaining < chunk) ? remaining : chunk;
			m_overtimeGroupSizes.push_back( groupSize );
			remaining -= groupSize;
		}
	}
}

void Map::UpdateSurvivalWaves( float deltaSeconds )
{
	if (m_definition == nullptr || !m_definition->m_survivalWaves.m_enabled || m_survivalVictory)
	{
		return;
	}
	if (!IsPlayerAlive())
	{
		return;
	}

	if (!m_startTriggered)
	{
		return;
	}

	if (m_introActive)
	{
		UpdateSurvivalIntro( deltaSeconds );
		return;
	}

	if (!m_combatStarted)
	{
		return;
	}

	m_survivalSeconds += deltaSeconds;
	TrySpawnScheduledWaves();
	UpdateOvertimeSpawns( deltaSeconds );
	UpdateMinuteBanners( deltaSeconds );
	CheckSurvivalVictory();
}

void Map::CacheStartTriggerBounds()
{
	m_hasStartTrigger = false;
	m_startTriggered = false;
	m_introActive = false;
	m_combatStarted = false;
	m_introSeconds = 0.f;
	m_introStep = 0;
	m_shownBanner02 = false;
	m_shownBanner01 = false;
	m_shownBannerOvertime = false;
	m_minuteBannerText.clear();
	m_minuteBannerAge = 0.f;

	if (m_definition == nullptr)
	{
		m_startTriggered = true;
		m_combatStarted = true;
		return;
	}

	for (MapInteractableDefinition const& interactable : m_definition->m_interactables)
	{
		if (interactable.m_type != "StartTrigger")
		{
			continue;
		}

		Vec3 const halfSize = interactable.m_size * 0.5f;
		m_startTriggerBounds = AABB3( interactable.m_center - halfSize, interactable.m_center + halfSize );
		m_hasStartTrigger = true;
		break;
	}

	if (!m_hasStartTrigger)
	{
		m_startTriggered = true;
		m_combatStarted = true;
	}
}

bool Map::IsPointOnStartTrigger( Vec3 const& point ) const
{
	if (!m_hasStartTrigger)
	{
		return false;
	}

	return point.x >= m_startTriggerBounds.m_mins.x - START_TRIGGER_HIT_PAD
		&& point.x <= m_startTriggerBounds.m_maxs.x + START_TRIGGER_HIT_PAD
		&& point.y >= m_startTriggerBounds.m_mins.y - START_TRIGGER_HIT_PAD
		&& point.y <= m_startTriggerBounds.m_maxs.y + START_TRIGGER_HIT_PAD
		&& point.z >= m_startTriggerBounds.m_mins.z - START_TRIGGER_HIT_PAD
		&& point.z <= m_startTriggerBounds.m_maxs.z + START_TRIGGER_HIT_PAD;
}

void Map::NotifyShotImpact( Vec3 const& impactPos )
{
	if (m_startTriggered || !m_hasStartTrigger)
	{
		return;
	}
	if (!IsPointOnStartTrigger( impactPos ))
	{
		return;
	}
	if (!IsPlayerAlive())
	{
		return;
	}

	BeginSurvivalIntro();
}

void Map::BeginSurvivalIntro()
{
	m_startTriggered = true;
	m_introActive = true;
	m_combatStarted = false;
	m_introSeconds = 0.f;
	m_introStep = 0;
	DestroyStartBlock();
}

void Map::DestroyStartBlock()
{
	if (m_definition == nullptr)
	{
		return;
	}

	bool alreadyDestroyed = false;
	for (std::string const& name : m_destroyedGeometryNames)
	{
		if (name == "StartBlock")
		{
			alreadyDestroyed = true;
			break;
		}
	}
	if (!alreadyDestroyed)
	{
		m_destroyedGeometryNames.push_back( "StartBlock" );
	}

	BuildGeometry( false );
}

void Map::UpdateSurvivalIntro( float deltaSeconds )
{
	m_introSeconds += deltaSeconds;

	float elapsed = m_introSeconds;
	int step = 0;
	for ( ; step < HUD_INTRO_STEP_COUNT; ++step )
	{
		float const beat = (step < HUD_INTRO_STEP_COUNT - 1) ? HUD_INTRO_BEAT_SECONDS : HUD_SURVIVE_BEAT_SECONDS;
		if (elapsed < beat)
		{
			break;
		}
		elapsed -= beat;
	}

	m_introStep = (step < HUD_INTRO_STEP_COUNT) ? step : (HUD_INTRO_STEP_COUNT - 1);
	if (step >= HUD_INTRO_STEP_COUNT)
	{
		m_introActive = false;
		m_combatStarted = true;
		m_survivalSeconds = 0.f;
	}
}

void Map::PushMinuteBanner( std::string const& text, Rgba8 const& tint )
{
	m_minuteBannerText = text;
	m_minuteBannerTint = tint;
	m_minuteBannerAge = 0.f;
	m_minuteBannerDuration = HUD_BANNER_DURATION;
}

void Map::UpdateMinuteBanners( float deltaSeconds )
{
	if (!m_minuteBannerText.empty())
	{
		m_minuteBannerAge += deltaSeconds;
		if (m_minuteBannerAge >= m_minuteBannerDuration)
		{
			m_minuteBannerText.clear();
		}
	}

	if (m_survivalVictory)
	{
		return;
	}

	if (!m_shownBanner02 && m_survivalSeconds >= 60.f)
	{
		m_shownBanner02 = true;
		PushMinuteBanner( "02:00", Rgba8::WHITE );
	}
	if (!m_shownBanner01 && m_survivalSeconds >= 120.f)
	{
		m_shownBanner01 = true;
		PushMinuteBanner( "01:00", Rgba8::WHITE );
	}
	if (!m_shownBannerOvertime && m_overtimeActive)
	{
		m_shownBannerOvertime = true;
		PushMinuteBanner( "OVERTIME", Rgba8::ORANGE );
	}
}

void Map::TrySpawnScheduledWaves()
{
	SurvivalWaveDefinition const& waves = m_definition->m_survivalWaves;
	if (m_overtimeActive || m_survivalSeconds >= waves.m_durationSeconds)
	{
		return;
	}

	while (m_nextScheduledWaveIndex < (int)m_scheduledWaves.size())
	{
		ScheduledSurvivalWave const& wave = m_scheduledWaves[m_nextScheduledWaveIndex];
		if (wave.m_time >= waves.m_durationSeconds)
		{
			break;
		}
		if (m_survivalSeconds < wave.m_time)
		{
			break;
		}
		if (waves.m_maxLiveBeforeOvertime > 0 && CountLivingDemons() >= waves.m_maxLiveBeforeOvertime)
		{
			break;
		}

		SpawnCaveGroup( wave.m_count, 0.f );
		++m_nextScheduledWaveIndex;
		break; // at most one regular wave per frame; delayed waves catch up without dumping all at once
	}
}

void Map::UpdateOvertimeSpawns( float deltaSeconds )
{
	SurvivalWaveDefinition const& waves = m_definition->m_survivalWaves;
	if (m_survivalSeconds < waves.m_durationSeconds || m_overtimeFinishedSpawning)
	{
		return;
	}

	if (!m_overtimeActive)
	{
		m_overtimeActive = true;
		m_overtimeGroupIndex = 0;
		m_overtimeGroupTimer = 0.f;
		DebugAddMessage( "OVERTIME", 3.f, Rgba8( 255, 160, 40, 255 ), Rgba8( 255, 160, 40, 255 ) );
	}

	if (m_overtimeGroupSizes.empty())
	{
		m_overtimeFinishedSpawning = true;
		return;
	}

	m_overtimeGroupTimer += deltaSeconds;
	float const stagger = waves.m_overtime.m_stagger;
	while (m_overtimeGroupIndex < (int)m_overtimeGroupSizes.size())
	{
		float const dueTime = (float)m_overtimeGroupIndex * stagger;
		if (m_overtimeGroupTimer + 0.0001f < dueTime)
		{
			break;
		}

		float const yOffset = (float)m_overtimeGroupIndex * waves.m_overtime.m_yStep;
		SpawnCaveGroup( m_overtimeGroupSizes[m_overtimeGroupIndex], yOffset );
		++m_overtimeGroupIndex;
	}

	if (m_overtimeGroupIndex >= (int)m_overtimeGroupSizes.size())
	{
		m_overtimeFinishedSpawning = true;
	}
}

void Map::SelectCaveSlotIndexes( int pickCount, std::vector<int>& outIndexes ) const
{
	outIndexes.clear();
	if (m_definition == nullptr)
	{
		return;
	}

	int const slotCount = (int)m_definition->m_survivalWaves.m_slotXs.size();
	if (slotCount <= 0 || pickCount <= 0)
	{
		return;
	}

	int const useCount = (pickCount < slotCount) ? pickCount : slotCount;
	if (useCount == slotCount - 1 && (slotCount % 2) == 1)
	{
		int const mid = slotCount / 2;
		for (int i = 0; i < slotCount; ++i)
		{
			if (i != mid)
			{
				outIndexes.push_back( i );
			}
		}
		return;
	}

	int const start = (slotCount - useCount) / 2;
	for (int i = 0; i < useCount; ++i)
	{
		outIndexes.push_back( start + i );
	}
}

void Map::SpawnCaveGroup( int count, float yOffset )
{
	if (m_definition == nullptr || count <= 0)
	{
		return;
	}

	SurvivalWaveDefinition const& waves = m_definition->m_survivalWaves;
	EntityDefinition const* definition = EntityDefinition::GetEntityDef( waves.m_entityTypeName );
	GUARANTEE_OR_DIE( definition != nullptr,
		Stringf( "SurvivalWaves unknown entity: %s", waves.m_entityTypeName.c_str() ).c_str() );

	int const slotCount = (int)waves.m_slotXs.size();
	if (slotCount <= 0)
	{
		return;
	}

	std::vector<int> slotIndexes;
	SelectCaveSlotIndexes( (count < slotCount) ? count : slotCount, slotIndexes );
	if (slotIndexes.empty())
	{
		return;
	}

	int remaining = count;
	int row = 0;
	while (remaining > 0)
	{
		int const rowCount = (remaining < (int)slotIndexes.size()) ? remaining : (int)slotIndexes.size();
		float const rowY = waves.m_slotY + yOffset + (float)row * waves.m_overtime.m_yStep;
		for (int i = 0; i < rowCount; ++i)
		{
			int const slotIndex = slotIndexes[i];
			Vec3 const position( waves.m_slotXs[slotIndex], rowY, waves.m_slotZ );
			SpawnActor( definition, position, waves.m_orientation );
		}
		remaining -= rowCount;
		++row;
	}
}

int Map::CountLivingDemons() const
{
	int count = 0;
	for (Entity const* entity : m_entities)
	{
		if (entity == nullptr || entity->m_isDead || entity->GetDefinition() == nullptr)
		{
			continue;
		}
		if (entity->GetDefinition()->faction == Faction::DEMON)
		{
			++count;
		}
	}
	return count;
}

bool Map::IsPlayerAlive() const
{
	if (m_playerController == nullptr)
	{
		return false;
	}
	Entity const* playerEntity = m_playerController->GetEntity();
	return playerEntity != nullptr && !playerEntity->m_isDead;
}

void Map::CheckSurvivalVictory()
{
	if (m_survivalVictory || m_survivalDefeat || m_definition == nullptr || !m_definition->m_survivalWaves.m_enabled)
	{
		return;
	}
	if (!m_overtimeFinishedSpawning || !IsPlayerAlive())
	{
		return;
	}
	if (CountLivingDemons() > 0)
	{
		return;
	}

	BeginVictory();
}

void Map::CheckSurvivalDefeat()
{
	if (m_survivalVictory || m_survivalDefeat)
	{
		return;
	}
	if (!IsPlayerAlive())
	{
		BeginDefeat();
	}
}

void Map::BeginVictory()
{
	if (m_survivalVictory || m_survivalDefeat)
	{
		return;
	}

	m_survivalVictory = true;
	m_endScreenSeconds = 0.f;
	m_endScreenFade = 0.f;
	m_endPromptVisible = false;
	SnapshotMatchStats();
}

void Map::BeginDefeat()
{
	if (m_survivalVictory || m_survivalDefeat)
	{
		return;
	}

	m_survivalDefeat = true;
	m_endScreenSeconds = 0.f;
	m_endScreenFade = 0.f;
	m_endPromptVisible = false;
	SnapshotMatchStats();
}

void Map::SnapshotMatchStats()
{
	m_resultKills = m_killCount;
	m_resultClearSeconds = m_survivalSeconds;
	m_resultDamageTaken = (int)(m_damageTaken + 0.5f);
	m_resultHeals = m_healPurchaseCount;
	m_resultAmmoBuys = m_ammoPurchaseCount;
	m_resultHealth = 0;
	if (m_playerController != nullptr)
	{
		Entity const* playerEntity = m_playerController->GetEntity();
		if (playerEntity != nullptr)
		{
			m_resultHealth = (int)playerEntity->m_health;
			if (m_resultHealth < 0)
			{
				m_resultHealth = 0;
			}
		}
	}
}

void Map::UpdateMatchEndScreen( float deltaSeconds )
{
	if (!m_survivalVictory && !m_survivalDefeat)
	{
		return;
	}

	m_endScreenSeconds += deltaSeconds;
	m_endScreenFade = (HUD_END_FADE_SECONDS > 0.f) ? Clamp( m_endScreenSeconds / HUD_END_FADE_SECONDS, 0.f, 1.f ) : 1.f;
	float const contentAge = m_endScreenSeconds - HUD_END_FADE_SECONDS;
	float const statsDoneAt = HUD_END_STAT_START_SECONDS
		+ (float)(HUD_END_STAT_COUNT - 1) * HUD_END_STAT_STAGGER_SECONDS
		+ HUD_END_STAT_POP_SECONDS;
	m_endPromptVisible = (contentAge >= statsDoneAt);
}

void Map::RenderCenteredHudText( BitmapFont* font, std::string const& text, float cellHeight, Rgba8 const& tint, float yOffset, float xOffset ) const
{
	if (font == nullptr || text.empty() || m_playerController == nullptr)
	{
		return;
	}

	Vec2 const hudSize = m_playerController->m_hudCamera.GetDimensions();
	float const textWidth = font->GetTextWidth( cellHeight, text, HUD_SURVIVAL_TEXT_ASPECT );
	Vec2 const textMins( (hudSize.x - textWidth) * 0.5f + xOffset, (hudSize.y - cellHeight) * 0.5f + yOffset );
	std::vector<Vertex_PCU> textVerts;
	font->AddVertsForText2D( textVerts, textMins, cellHeight, text, tint, HUD_SURVIVAL_TEXT_ASPECT );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->BindTexture( &font->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}

void Map::RenderMatchEndOverlay() const
{
	if (!m_survivalVictory && !m_survivalDefeat)
	{
		return;
	}
	if (m_playerController == nullptr)
	{
		return;
	}

	Vec2 const hudSize = m_playerController->m_hudCamera.GetDimensions();
	unsigned char const overlayAlpha = (unsigned char)Clamp( m_endScreenFade * HUD_END_MAX_OVERLAY_ALPHA, 0.f, 255.f );
	if (overlayAlpha > 0)
	{
		std::vector<Vertex_PCU> overlayVerts;
		AddVertsForAABB2D( overlayVerts, AABB2( Vec2::ZERO, hudSize ), Rgba8( 0, 0, 0, overlayAlpha ) );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default", VertexType::VERTEX_PCU ) );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( overlayVerts );
	}

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	if (font == nullptr)
	{
		return;
	}

	if (m_endScreenFade < 1.f)
	{
		return;
	}

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	if (m_survivalVictory)
	{
		RenderCenteredHudText( font, "VICTORY", HUD_END_TITLE_HEIGHT, Rgba8::GREEN, 36.f, HUD_END_TITLE_X_OFFSET );
		RenderMatchStatLines( font, hudSize, "VICTORY" );
		if (m_endPromptVisible)
		{
			RenderCenteredHudText( font, "Press ESC to return", HUD_END_PROMPT_HEIGHT, Rgba8::WHITE, -240.f );
		}
		return;
	}

	RenderCenteredHudText( font, "YOU DIED", HUD_END_TITLE_HEIGHT, Rgba8::RED, 36.f, HUD_END_TITLE_X_OFFSET );
	RenderMatchStatLines( font, hudSize, "YOU DIED" );
	if (m_endPromptVisible)
	{
		RenderCenteredHudText( font, "SPACE: Retry", HUD_END_PROMPT_HEIGHT, Rgba8::WHITE, -220.f );
		RenderCenteredHudText( font, "ESC: Return", HUD_END_PROMPT_HEIGHT, Rgba8::WHITE, -260.f );
	}
}

void Map::RenderMatchStatLines( BitmapFont* font, Vec2 const& hudSize, std::string const& titleText ) const
{
	if (font == nullptr)
	{
		return;
	}

	int const clearSeconds = (int)m_resultClearSeconds;
	int const minutes = (clearSeconds > 0) ? (clearSeconds / 60) : 0;
	int const seconds = (clearSeconds > 0) ? (clearSeconds % 60) : 0;

	std::string const lines[HUD_END_STAT_COUNT] = {
		Stringf( "Kills: %d", m_resultKills ),
		Stringf( "Time: %02d:%02d", minutes, seconds ),
		Stringf( "Dmg Taken: %d", m_resultDamageTaken ),
		Stringf( "HP Left: %d", m_resultHealth ),
		Stringf( "Heals: %d", m_resultHeals ),
		Stringf( "Ammo Buys: %d", m_resultAmmoBuys )
	};

	float const titleWidth = font->GetTextWidth( HUD_END_TITLE_HEIGHT, titleText, HUD_SURVIVAL_TEXT_ASPECT );
	float const titleMinsX = (hudSize.x - titleWidth) * 0.5f + HUD_END_TITLE_X_OFFSET;
	float const titleMinsY = (hudSize.y - HUD_END_TITLE_HEIGHT) * 0.5f + 36.f;
	float const statsX = titleMinsX + titleWidth + HUD_END_STAT_GAP_FROM_TITLE;
	float const firstLineY = titleMinsY + HUD_END_TITLE_HEIGHT - HUD_END_STAT_HEIGHT;
	float const contentAge = m_endScreenSeconds - HUD_END_FADE_SECONDS;

	for (int i = 0; i < HUD_END_STAT_COUNT; ++i)
	{
		float const appearAt = HUD_END_STAT_START_SECONDS + (float)i * HUD_END_STAT_STAGGER_SECONDS;
		float const local = (HUD_END_STAT_POP_SECONDS > 0.f)
			? (contentAge - appearAt) / HUD_END_STAT_POP_SECONDS
			: 1.f;
		if (local <= 0.f)
		{
			continue;
		}

		float const t = Clamp( local, 0.f, 1.f );
		float const pop = SmoothStop3( t );
		float const scale = Interpolate( 1.55f, 1.f, pop );
		float const jumpY = Interpolate( -26.f, 0.f, pop );
		unsigned char const alpha = (unsigned char)Clamp( pop * 255.f, 0.f, 255.f );
		Rgba8 tint = Rgba8::WHITE;
		tint.a = alpha;

		float const drawHeight = HUD_END_STAT_HEIGHT * scale;
		float const lineRestY = firstLineY - (float)i * (HUD_END_STAT_HEIGHT + HUD_END_STAT_LINE_GAP);
		Vec2 const textMins(
			statsX,
			lineRestY + jumpY - (drawHeight - HUD_END_STAT_HEIGHT) * 0.5f );

		std::vector<Vertex_PCU> textVerts;
		font->AddVertsForText2D( textVerts, textMins, drawHeight, lines[i], tint, HUD_SURVIVAL_TEXT_ASPECT );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->BindTexture( &font->GetTexture() );
		g_theRenderer->DrawVertexArray( textVerts );
	}
}

void Map::RenderSurvivalHud() const
{
	if (m_definition == nullptr || !m_definition->m_survivalWaves.m_enabled || m_playerController == nullptr)
	{
		return;
	}

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	if (font == nullptr)
	{
		return;
	}

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );

	if (m_introActive)
	{
		int const step = (m_introStep >= 0 && m_introStep < HUD_INTRO_STEP_COUNT) ? m_introStep : 0;
		std::string const introText = HUD_INTRO_STEPS[step];
		float const height = (step == HUD_INTRO_STEP_COUNT - 1) ? HUD_SURVIVE_TEXT_HEIGHT : HUD_INTRO_TEXT_HEIGHT;
		Rgba8 const tint = (step == HUD_INTRO_STEP_COUNT - 1) ? Rgba8::GREEN : Rgba8::WHITE;
		RenderCenteredHudText( font, introText, height, tint );
		return;
	}

	if (!m_combatStarted)
	{
		return;
	}

	SurvivalWaveDefinition const& waves = m_definition->m_survivalWaves;
	Vec2 const hudSize = m_playerController->m_hudCamera.GetDimensions();

	if (!m_survivalVictory)
	{
		std::string timerText;
		Rgba8 timerTint = Rgba8::WHITE;
		if (m_overtimeActive)
		{
			timerText = "OVERTIME";
			timerTint = Rgba8::ORANGE;
		}
		else
		{
			float remaining = waves.m_durationSeconds - m_survivalSeconds;
			if (remaining < 0.f)
			{
				remaining = 0.f;
			}
			int const totalSeconds = (int)ceilf( remaining );
			int const minutes = totalSeconds / 60;
			int const seconds = totalSeconds % 60;
			timerText = Stringf( "%02d:%02d", minutes, seconds );
		}

		float const timerY = (hudSize.y - HUD_SURVIVAL_TEXT_HEIGHT) * 0.5f;
		Vec2 const timerMins( HUD_SURVIVAL_LEFT_PAD, timerY );
		std::vector<Vertex_PCU> timerVerts;
		font->AddVertsForText2D( timerVerts, timerMins, HUD_SURVIVAL_TEXT_HEIGHT, timerText, timerTint, HUD_SURVIVAL_TEXT_ASPECT );
		g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
		g_theRenderer->BindTexture( &font->GetTexture() );
		g_theRenderer->DrawVertexArray( timerVerts );
	}

	if (m_survivalVictory)
	{
		return;
	}

	if (!m_minuteBannerText.empty() && m_minuteBannerDuration > 0.f)
	{
		float const t = Clamp( m_minuteBannerAge / m_minuteBannerDuration, 0.f, 1.f );
		float const fade = (1.f - t) * (1.f - t);
		float const scale = 1.55f - (1.3f * t);
		unsigned char const alpha = (unsigned char)Clamp( fade * 255.f, 0.f, 255.f );
		Rgba8 tint = m_minuteBannerTint;
		tint.a = alpha;
		RenderCenteredHudText( font, m_minuteBannerText, HUD_BANNER_TEXT_HEIGHT * scale, tint );
	}
}

void Map::RenderShopPrompt() const
{
	if (m_playerController == nullptr)
	{
		return;
	}

	Entity* playerEntity = m_playerController->GetEntity();
	if (playerEntity == nullptr || playerEntity->m_isDead)
	{
		return;
	}

	if (!IsInsideShopRange( playerEntity->m_position ))
	{
		return;
	}

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	if (font == nullptr)
	{
		return;
	}

	Vec2 const hudSize = m_playerController->m_hudCamera.GetDimensions();
	std::string const line1 = Stringf( "%s: %s",
		GetInteractButtonLabel( "interactButton1", "InteractButton 1" ).c_str(),
		SHOP_OFFER_HEALTH );
	std::string const line2 = Stringf( "%s: %s",
		GetInteractButtonLabel( "interactButton2", "InteractButton 2" ).c_str(),
		SHOP_OFFER_AMMO );

	float const line1Top = hudSize.y - HUD_SHOP_PROMPT_TOP_PAD;
	float const line1Bottom = line1Top - HUD_SHOP_PROMPT_HEIGHT;
	float const line2Top = line1Bottom - HUD_SHOP_PROMPT_LINE_GAP;
	float const line2Bottom = line2Top - HUD_SHOP_PROMPT_HEIGHT;

	std::vector<Vertex_PCU> textVerts;
	font->AddVertsForTextInBox2D(
		textVerts,
		AABB2( 0.f, line1Bottom, hudSize.x, line1Top ),
		HUD_SHOP_PROMPT_HEIGHT,
		line1,
		Rgba8::WHITE,
		HUD_SHOP_PROMPT_ASPECT,
		Vec2( 0.5f, 0.5f ) );
	font->AddVertsForTextInBox2D(
		textVerts,
		AABB2( 0.f, line2Bottom, hudSize.x, line2Top ),
		HUD_SHOP_PROMPT_HEIGHT,
		line2,
		Rgba8::WHITE,
		HUD_SHOP_PROMPT_ASPECT,
		Vec2( 0.5f, 0.5f ) );

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default", VertexType::VERTEX_PCU ) );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->BindTexture( &font->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}

NavMesh const* Map::GetNavMeshByName( std::string const& name ) const
{
	for (NavMesh const& navMesh : m_navMeshes)
	{
		if (navMesh.GetName() == name)
		{
			return &navMesh;
		}
	}
	return nullptr;
}

NavMesh* Map::GetNavMeshByName( std::string const& name )
{
	for (NavMesh& navMesh : m_navMeshes)
	{
		if (navMesh.GetName() == name)
		{
			return &navMesh;
		}
	}
	return nullptr;
}

NavMesh const* Map::FindNavMeshForPosition( Vec3 const& position ) const
{
	NavMesh const* bestMesh = nullptr;
	float bestDistSq = 1e30f;

	for (NavMesh const& navMesh : m_navMeshes)
	{
		int polyId = navMesh.FindPoly( position );
		NavPoly const* poly = navMesh.GetPoly( polyId );
		if (poly == nullptr)
		{
			continue;
		}

		Vec3 projected = navMesh.ProjectOntoPoly( *poly, position );
		float distSq = (projected - position).GetLengthSquared();
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			bestMesh = &navMesh;
		}
	}

	return bestMesh;
}

NavPath Map::FindNavPath( Vec3 const& startPos, Vec3 const& goalPos ) const
{
	NavPath path;
	NavMesh const* startMesh = FindNavMeshForPosition( startPos );
	NavMesh const* goalMesh = FindNavMeshForPosition( goalPos );
	if (startMesh == nullptr || goalMesh == nullptr)
	{
		return path;
	}

	// Multiple volumes (if any) are separate graphs. TestMap uses one volume covering cave+arena.
	if (startMesh != goalMesh)
	{
		return path;
	}

	return startMesh->FindPath( startPos, goalPos );
}

bool Map::FindNavPath( Vec3 const& startPos, Vec3 const& goalPos, std::vector<Vec3>& outWaypoints ) const
{
	NavPath path = FindNavPath( startPos, goalPos );
	outWaypoints = path.m_waypoints;
	return path.m_isValid;
}

void Map::DebugDrawNavPathQuery() const
{
	// Intentionally empty: each AIController draws its own nav path.
}

Entity* Map::SpawnActor( EntityDefinition const* definition, Vec3 const& position, EulerAngles const& orientation )
{
	GUARANTEE_OR_DIE( definition != nullptr, "SpawnActor requires a valid EntityDefinition" );

	int slotIndex = -1;
	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		if (m_entities[i] == nullptr)
		{
			slotIndex = i;
			break;
		}
	}

	if (slotIndex < 0)
	{
		slotIndex = (int)m_entities.size();
		m_entities.push_back( nullptr );
	}

	Entity* entity = new Entity( this, definition );
	entity->m_game = m_game;
	entity->m_position = position;
	entity->m_previousPosition = position;
	entity->m_orientation = orientation;
	entity->SyncCollisionTransform();
	entity->SetUID( EntityUID( m_entitySalt++, (unsigned int)slotIndex ) );
	m_entities[slotIndex] = entity;

	TryPossessFromDefinition( entity );
	return entity;
}

void Map::DestroyEntity( Entity* entity )
{
	if (entity == nullptr)
	{
		return;
	}

	for (Entity* other : m_entities)
	{
		if (other == nullptr || other == entity)
		{
			continue;
		}
		other->ClearChargingProjectileRef( entity );
	}

	for (Entity*& slot : m_entities)
	{
		if (slot == entity)
		{
			if (m_playerController != nullptr && m_playerController->GetEntity() == entity)
			{
				m_playerController = nullptr;
			}
			delete slot;
			slot = nullptr;
			return;
		}
	}
}

Entity* Map::GetEntityByUID( EntityUID const& uid ) const
{
	if (!uid.isValid())
	{
		return nullptr;
	}

	unsigned int const index = uid.GetIndex();
	if (index >= (unsigned int)m_entities.size())
	{
		return nullptr;
	}

	Entity* entity = m_entities[index];
	if (entity == nullptr || entity->GetUID() != uid)
	{
		return nullptr;
	}
	return entity;
}

void Map::TryPossessFromDefinition( Entity* entity )
{
	GUARANTEE_OR_DIE( entity != nullptr, "TryPossessFromDefinition requires a valid Entity" );
	GUARANTEE_OR_DIE( entity->GetDefinition() != nullptr, "Entity missing definition for controller possess" );

	std::string const& controllerName = entity->GetDefinition()->controllerName;
	if (ControllerFactory::IsNone( controllerName ))
	{
		return;
	}

	Controller* controller = ControllerFactory::Create( controllerName );
	GUARANTEE_OR_DIE( controller != nullptr, Stringf( "Failed to create controller: %s", controllerName.c_str() ).c_str() );
	controller->Possess( entity );

	if (controllerName == "PlayerController" && m_playerController == nullptr)
	{
		m_playerController = static_cast<PlayerController*>( controller );
	}
}

RaycastResult3D Map::RaycastWorld( Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist ) const
{
	RaycastResult3D bestResult;
	bestResult.m_rayStartPos = startPos;
	bestResult.m_rayFwdNormal = fwdNormal;
	bestResult.m_rayMaxLength = maxDist;

	for (AABB3 const& bounds : m_solidBounds)
	{
		RaycastResult3D hitResult;
		if (!RaycastAABB3D( hitResult, startPos, fwdNormal, maxDist, bounds ))
		{
			continue;
		}
		if (!bestResult.m_didImpact || hitResult.m_impactDist < bestResult.m_impactDist)
		{
			bestResult = hitResult;
		}
	}

	for (OBB3 const& bounds : m_solidObbs)
	{
		RaycastResult3D hitResult;
		if (!RaycastOBB3D( hitResult, startPos, fwdNormal, maxDist, bounds ))
		{
			continue;
		}
		if (!bestResult.m_didImpact || hitResult.m_impactDist < bestResult.m_impactDist)
		{
			bestResult = hitResult;
		}
	}

	return bestResult;
}

RaycastResult3D Map::SpherecastWorld( Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, float radius ) const
{
	RaycastResult3D bestResult;
	bestResult.m_rayStartPos = startPos;
	bestResult.m_rayFwdNormal = fwdNormal;
	bestResult.m_rayMaxLength = maxDist;

	if (radius <= 0.f)
	{
		return RaycastWorld( startPos, fwdNormal, maxDist );
	}

	Vec3 const expansion( radius, radius, radius );

	for (AABB3 const& bounds : m_solidBounds)
	{
		AABB3 const expanded( bounds.m_mins - expansion, bounds.m_maxs + expansion );
		RaycastResult3D hitResult;
		if (!RaycastAABB3D( hitResult, startPos, fwdNormal, maxDist, expanded ))
		{
			continue;
		}
		if (!bestResult.m_didImpact || hitResult.m_impactDist < bestResult.m_impactDist)
		{
			bestResult = hitResult;
		}
	}

	for (OBB3 const& bounds : m_solidObbs)
	{
		OBB3 expanded = bounds;
		expanded.m_halfDimensions += expansion;
		RaycastResult3D hitResult;
		if (!RaycastOBB3D( hitResult, startPos, fwdNormal, maxDist, expanded ))
		{
			continue;
		}
		if (!bestResult.m_didImpact || hitResult.m_impactDist < bestResult.m_impactDist)
		{
			bestResult = hitResult;
		}
	}

	return bestResult;
}

OBB3 Map::MakeOBBFromAABB( AABB3 const& aabb )
{
	Vec3 const halfDimensions = (aabb.m_maxs - aabb.m_mins) * 0.5f;
	return OBB3( aabb.GetCenter(), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), halfDimensions );
}

void Map::CollidePhysics()
{
	CollideEntitiesWithWorld();
	CollideEntitiesWithEntities();
	// Demons absorb player overlap and may be pushed into walls; unstick them after.
	CollideEntitiesWithWorld();

	for (Entity* entity : m_entities)
	{
		if (entity == nullptr || entity->m_definition == nullptr)
		{
			continue;
		}
		if (!entity->IsSimulatePhysics() || entity->m_definition->flying)
		{
			continue;
		}
		entity->ResolveGroundContact();
	}
}

bool Map::IsCombatProjectile( Entity const* entity ) const
{
	if (entity == nullptr || entity->m_definition == nullptr || entity->m_isDead)
	{
		return false;
	}
	if (entity->GetCollision() == nullptr || !entity->GetCollision()->IsCollisionEnabled())
	{
		return false;
	}

	EntityDefinition const* def = entity->m_definition;
	return def->dieOnCollide || def->doesExplode || entity->m_damageOnCollide > 0.f;
}

bool Map::CanProjectileHurt( Entity const* projectile, Entity const* target ) const
{
	if (projectile == nullptr || target == nullptr || target == projectile || target->m_isDead)
	{
		return false;
	}

	EntityDefinition const* targetDef = target->GetDefinition();
	EntityDefinition const* projDef = projectile->GetDefinition();
	if (targetDef == nullptr || projDef == nullptr)
	{
		return false;
	}
	if (targetDef->dieOnSpawn)
	{
		return false;
	}
	if (targetDef->health <= 0.f)
	{
		return false;
	}
	// Held charging projectiles keep collision disabled — never treat as hit targets.
	if (target->GetCollision() == nullptr || !target->GetCollision()->IsCollisionEnabled())
	{
		return false;
	}

	if (projDef->faction == Faction::NEUTRAL)
	{
		return targetDef->faction == Faction::MARINE || targetDef->faction == Faction::DEMON;
	}

	return projDef->faction != targetDef->faction;
}

float Map::GetEntityCollisionRadius( Entity const* entity ) const
{
	if (entity == nullptr || entity->GetCollision() == nullptr)
	{
		return 0.f;
	}

	ShapeComponent* collision = entity->GetCollision();
	if (collision->GetCollisionShape() == CollisionShape::SPHERE)
	{
		return static_cast<SphereComponent*>( collision )->GetScaledRadius();
	}
	if (collision->GetCollisionShape() == CollisionShape::CAPSULE)
	{
		return static_cast<CapsuleComponent*>( collision )->GetScaledRadius();
	}
	if (entity->GetDefinition() != nullptr)
	{
		return entity->GetDefinition()->radius * entity->GetScale();
	}
	return 0.f;
}

Vec3 Map::GetEntityCollisionCenter( Entity const* entity ) const
{
	if (entity == nullptr)
	{
		return Vec3::ZERO;
	}
	if (entity->GetCollision() != nullptr)
	{
		return entity->GetCollision()->GetFunctionalCenter();
	}
	return entity->m_position;
}

Vec3 Map::GetHitEffectPosition( Vec3 const& impactPos, Vec3 const& impactNormal, Entity const* hitEntity, Vec3 const& viewerPos ) const
{
	Vec3 pos = impactPos;

	if (hitEntity != nullptr)
	{
		Vec3 const center = GetEntityCollisionCenter( hitEntity );
		float const radius = GetEntityCollisionRadius( hitEntity );
		float const distToViewer = (impactPos - viewerPos).GetLength();

		// Inside-volume raycasts report the camera / projectile origin as the hit.
		if (distToViewer < HIT_VFX_NEAR_VIEWER_DIST)
		{
			Vec3 outward = viewerPos - center;
			if (outward.GetLengthSquared() <= 0.0001f)
			{
				outward = Vec3( 0.f, 0.f, 1.f );
			}
			else
			{
				outward = outward.GetNormalized();
			}
			pos = center + outward * radius;
		}

		Vec3 fromCenter = pos - center;
		if (fromCenter.GetLengthSquared() > 0.0001f)
		{
			pos += fromCenter.GetNormalized() * HIT_VFX_SURFACE_OFFSET;
		}
		else if (impactNormal.GetLengthSquared() > 0.0001f)
		{
			pos += impactNormal.GetNormalized() * HIT_VFX_SURFACE_OFFSET;
		}
		return pos;
	}

	if (impactNormal.GetLengthSquared() > 0.0001f)
	{
		pos += impactNormal.GetNormalized() * HIT_VFX_SURFACE_OFFSET;
	}
	return pos;
}

void Map::HandleProjectileCollisions()
{
	std::vector<Entity*> projectiles;
	for (Entity* entity : m_entities)
	{
		if (IsCombatProjectile( entity ))
		{
			projectiles.push_back( entity );
		}
	}

	for (Entity* projectile : projectiles)
	{
		bool stillAlive = false;
		for (Entity* slot : m_entities)
		{
			if (slot == projectile)
			{
				stillAlive = true;
				break;
			}
		}
		if (!stillAlive || !IsCombatProjectile( projectile ))
		{
			continue;
		}

		Vec3 const start = projectile->m_previousPosition;
		Vec3 const end = projectile->m_position;
		Vec3 const delta = end - start;
		float const moveDist = delta.GetLength();
		float const radius = GetEntityCollisionRadius( projectile );

		Entity* hitEntity = nullptr;
		bool hitWorld = false;
		float bestDist = moveDist;
		Vec3 hitPos = end;
		bool skippedEmbedded = false;

		if (moveDist > 0.0001f)
		{
			Vec3 const dir = delta / moveDist;
			float const sweepDist = moveDist + radius;
			RaycastResult3D worldHit = SpherecastWorld( start, dir, sweepDist, radius );
			if (worldHit.m_didImpact)
			{
				hitWorld = true;
				bestDist = worldHit.m_impactDist;
				hitPos = worldHit.m_impactPos;
			}

			for (Entity* other : m_entities)
			{
				if (!CanProjectileHurt( projectile, other ) || other->GetCollision() == nullptr)
				{
					continue;
				}

				RaycastResult3D entityHit;
				bool didHit = false;
				ShapeComponent* collision = other->GetCollision();
				if (collision->GetCollisionShape() == CollisionShape::CAPSULE)
				{
					CapsuleComponent* capsule = static_cast<CapsuleComponent*>( collision );
					didHit = RaycastCapsule3D( entityHit, start, dir, sweepDist,
						capsule->GetWorldBoneStart(), capsule->GetWorldBoneEnd(),
						capsule->GetScaledRadius() + radius );
				}
				else if (collision->GetCollisionShape() == CollisionShape::SPHERE)
				{
					SphereComponent* sphere = static_cast<SphereComponent*>( collision );
					didHit = RaycastSphere3D( entityHit, start, dir, sweepDist,
						sphere->GetWorldPosition(), sphere->GetScaledRadius() + radius );
				}
				else
				{
					OBB3 bounds = collision->CalculateBoundsOBB3D();
					bounds.m_halfDimensions += Vec3( radius, radius, radius );
					didHit = RaycastOBB3D( entityHit, start, dir, sweepDist, bounds );
				}

				if (didHit && entityHit.m_didImpact && entityHit.m_impactDist < bestDist)
				{
					// Dist 0 means the sweep started already inside this actor.
					// Only ignore that for the launch-overlap frames so a point-blank
					// charge shot can fly out; later overlaps must still explode.
					if (entityHit.m_impactDist <= 0.0001f && projectile->m_ignoreEmbeddedHits)
					{
						skippedEmbedded = true;
						continue;
					}
					bestDist = entityHit.m_impactDist;
					hitPos = entityHit.m_impactPos;
					hitEntity = other;
					hitWorld = false;
				}
			}
		}

		if (!skippedEmbedded)
		{
			projectile->m_ignoreEmbeddedHits = false;
		}

		if (hitEntity != nullptr || hitWorld)
		{
			ResolveProjectileImpact( projectile, hitPos, hitEntity );
		}
	}
}

void Map::ResolveProjectileImpact( Entity* projectile, Vec3 const& hitPos, Entity* hitEntity )
{
	if (projectile == nullptr || projectile->m_definition == nullptr)
	{
		return;
	}

	if (hitEntity == nullptr)
	{
		NotifyShotImpact( hitPos );
	}

	if (hitEntity != nullptr)
	{
		hitEntity->TakeDamage( projectile->m_damageOnCollide );
		Vec3 impulseDir = projectile->m_velocity;
		if (impulseDir.GetLengthSquared() > 0.0001f)
		{
			impulseDir = impulseDir.GetNormalized();
		}
		else
		{
			impulseDir = (hitEntity->m_position - projectile->m_position);
			if (impulseDir.GetLengthSquared() > 0.0001f)
			{
				impulseDir = impulseDir.GetNormalized();
			}
			else
			{
				impulseDir = Vec3( 1.f, 0.f, 0.f );
			}
		}
		if (projectile->m_impulseOnCollide > 0.f && !hitEntity->IsInvincible())
		{
			hitEntity->m_velocity += impulseDir * projectile->m_impulseOnCollide;
		}
	}

	if (projectile->m_definition->doesExplode)
	{
		DoExplosion( projectile, hitPos, hitEntity );
	}

	if (projectile->m_definition->dieOnCollide)
	{
		DestroyEntity( projectile );
	}
}

void Map::DoExplosion( Entity* source, Vec3 const& position, Entity* hitEntity )
{
	if (source == nullptr || source->m_definition == nullptr)
	{
		return;
	}

	EntityDefinition const* def = source->m_definition;
	float const chargeScale = source->GetScale();
	float const scaleFactor = (chargeScale > 0.f) ? chargeScale : 1.f;
	float const squaredScale = scaleFactor * scaleFactor;
	// Range / VFX follow projectile volume (linear); damage/impulse use charge^2.
	float const range = def->range * scaleFactor;
	float const damage = def->damageOnExplosion * squaredScale;
	float const impulse = def->impulseOnExplosion * squaredScale;

	for (Entity* entity : m_entities)
	{
		if (!CanProjectileHurt( source, entity ))
		{
			continue;
		}

		Vec3 toTarget = GetEntityCollisionCenter( entity ) - position;
		float dist = toTarget.GetLength() - GetEntityCollisionRadius( entity );
		if (dist > range)
		{
			continue;
		}

		entity->TakeDamage( damage );
		if (impulse > 0.f && !entity->IsInvincible() && toTarget.GetLengthSquared() > 0.0001f)
		{
			entity->m_velocity += toTarget.GetNormalized() * impulse;
		}
	}

	if (!def->explosionAnimationActor.empty())
	{
		EntityDefinition const* vfxDef = EntityDefinition::GetEntityDef( def->explosionAnimationActor );
		if (vfxDef != nullptr)
		{
			Vec3 viewerPos = position;
			if (m_playerController != nullptr)
			{
				viewerPos = m_playerController->m_camera.m_position;
			}
			Vec3 const vfxPos = GetHitEffectPosition( position, Vec3::ZERO, hitEntity, viewerPos );
			Entity* vfx = SpawnActor( vfxDef, vfxPos, EulerAngles() );
			if (vfx != nullptr)
			{
				vfx->SetScale( scaleFactor );
			}
		}
	}
}

void Map::HandleCorpseCleanup( float deltaSeconds )
{
	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		Entity* entity = m_entities[i];
		if (entity == nullptr || !entity->m_isDead)
		{
			continue;
		}

		float const corpseLifetime = (entity->m_definition != nullptr) ? entity->m_definition->corpseLifetime : 0.f;
		if (corpseLifetime < 0.f)
		{
			continue;
		}

		entity->m_corpseTimer -= deltaSeconds;
		if (entity->m_corpseTimer <= 0.f)
		{
			DestroyEntity( entity );
		}
	}
}

void Map::CollideEntitiesWithWorld()
{
	constexpr int iterationCount = 3;
	for (int iteration = 0; iteration < iterationCount; ++iteration)
	{
		for (Entity* entity : m_entities)
		{
			if (entity == nullptr || !entity->WantsWorldCollision())
			{
				continue;
			}

			entity->SyncCollisionTransform();

			for (AABB3 const& bounds : m_solidBounds)
			{
				Vec3 mtv;
				if (!TryGetEntityVsWorldMTV( entity, MakeOBBFromAABB( bounds ), mtv ))
				{
					continue;
				}
				entity->ApplyCollisionDisplacement( mtv );
			}

			for (OBB3 const& bounds : m_solidObbs)
			{
				Vec3 mtv;
				if (!TryGetEntityVsWorldMTV( entity, bounds, mtv ))
				{
					continue;
				}
				entity->ApplyCollisionDisplacement( mtv );
			}
		}
	}
}

void Map::CollideEntitiesWithEntities()
{
	int const entityCount = (int)m_entities.size();
	struct PlayerCrowdContact
	{
		Entity* other = nullptr;
		Vec3 playerMtv;
	};
	std::vector<PlayerCrowdContact> crowdContacts;

	for (int i = 0; i < entityCount; ++i)
	{
		Entity* entityA = m_entities[i];
		if (entityA == nullptr || !entityA->WantsEntityCollision())
		{
			continue;
		}

		entityA->SyncCollisionTransform();

		for (int j = i + 1; j < entityCount; ++j)
		{
			Entity* entityB = m_entities[j];
			if (entityB == nullptr || !entityB->WantsEntityCollision())
			{
				continue;
			}

			entityB->SyncCollisionTransform();

			// Combat projectiles deal damage/explode via HandleProjectileCollisions;
			// skip MTV so they do not shove enemies before impact resolves.
			if (IsCombatProjectile( entityA ) || IsCombatProjectile( entityB ))
			{
				continue;
			}

			Vec3 mtvA;
			if (!TryGetEntityVsEntityMTV( entityA, entityB, mtvA ))
			{
				continue;
			}

			bool const aIsPlayer = IsPlayerEntity( entityA );
			bool const bIsPlayer = IsPlayerEntity( entityB );
			if (aIsPlayer != bIsPlayer)
			{
				Entity* const other = aIsPlayer ? entityB : entityA;
				Vec3 const playerMtv = aIsPlayer ? mtvA : (mtvA * -1.f);
				crowdContacts.push_back( PlayerCrowdContact{ other, playerMtv } );

				Vec3 awayXY( playerMtv.x, playerMtv.y, 0.f );
				if (awayXY.GetLengthSquared() > 0.000001f)
				{
					awayXY = awayXY.GetNormalized();
					float const closing = DotProduct3D( other->m_velocity, awayXY * -1.f );
					if (closing > 0.f)
					{
						other->m_velocity += awayXY * closing;
					}
				}
			}
			else
			{
				entityA->ApplyCollisionDisplacement( mtvA * 0.5f );
				entityB->ApplyCollisionDisplacement( mtvA * -0.5f );
			}
		}
	}

	if (crowdContacts.empty())
	{
		return;
	}

	Entity* player = (m_playerController != nullptr) ? m_playerController->GetEntity() : nullptr;
	if (player == nullptr)
	{
		return;
	}

	Vec3 driveXY = player->m_moveIntentXY;
	if (driveXY.GetLengthSquared() < 0.000001f)
	{
		driveXY = Vec3( player->m_velocity.x, player->m_velocity.y, 0.f );
	}
	float const driveLen = driveXY.GetLength();
	Vec3 driveDir = Vec3::ZERO;
	if (driveLen > 0.000001f)
	{
		driveDir = driveXY / driveLen;
	}

	std::vector<float> shares( crowdContacts.size(), 0.f );
	Vec3 accumulatedPlayerMtv = Vec3::ZERO;
	for (int i = 0; i < (int)crowdContacts.size(); ++i)
	{
		Vec3 const& playerMtv = crowdContacts[i].playerMtv;
		float const awayLen = playerMtv.GetLength();
		if (awayLen <= 0.000001f || driveLen <= 0.000001f)
		{
			continue;
		}

		Vec3 const awayDir = playerMtv / awayLen;
		float const incoming = -DotProduct3D( driveDir, awayDir );
		if (incoming > PLAYER_CROWD_INCOMING_DOT)
		{
			shares[i] = PLAYER_CROWD_PUSH_SHARE * incoming;
			accumulatedPlayerMtv += playerMtv * shares[i];
		}
	}

	float scale = 1.f;
	float const accLen = accumulatedPlayerMtv.GetLength();
	float attemptedStep = player->m_moveIntentXY.GetLength();
	if (attemptedStep < 0.000001f)
	{
		attemptedStep = 0.06f;
	}
	float const maxPush = attemptedStep * PLAYER_CROWD_MAX_STEP_FRACTION;
	if (accLen > maxPush && accLen > 0.000001f)
	{
		scale = maxPush / accLen;
		accumulatedPlayerMtv *= scale;
	}

	player->ApplyCollisionDisplacement( accumulatedPlayerMtv );

	for (int i = 0; i < (int)crowdContacts.size(); ++i)
	{
		Entity* other = crowdContacts[i].other;
		if (other == nullptr)
		{
			continue;
		}
		float const otherShare = 1.f - (shares[i] * scale);
		other->ApplyCollisionDisplacement( crowdContacts[i].playerMtv * -otherShare );
	}
}

bool Map::TryGetEntityVsWorldMTV( Entity* entity, OBB3 const& worldObb, Vec3& outMtv ) const
{
	outMtv = Vec3::ZERO;
	if (entity == nullptr || entity->m_collision == nullptr || !entity->m_collision->IsCollisionEnabled())
	{
		return false;
	}

	ShapeComponent* shape = entity->m_collision;
	CollisionShape const collisionShape = shape->GetCollisionShape();

	if (collisionShape == CollisionShape::CAPSULE)
	{
		CapsuleComponent* capsule = static_cast<CapsuleComponent*>( shape );
		return DoCapsuleOverlapOBB3D(
			capsule->GetWorldBoneStart(),
			capsule->GetWorldBoneEnd(),
			capsule->GetScaledRadius(),
			worldObb,
			outMtv );
	}

	if (collisionShape == CollisionShape::SPHERE)
	{
		SphereComponent* sphere = static_cast<SphereComponent*>( shape );
		Vec3 const center = sphere->GetWorldPosition();
		return DoCapsuleOverlapOBB3D( center, center, sphere->GetScaledRadius(), worldObb, outMtv );
	}

	if (collisionShape == CollisionShape::CUBE)
	{
		CubeComponent* cube = static_cast<CubeComponent*>( shape );
		OBB3 mobileObb = cube->CalculateBoundsOBB3D();
		Vec3 mtv;
		if (!DoOBBsOverlap3D( mobileObb, worldObb, mtv ))
		{
			return false;
		}
		// SAT MTV moves A opposite to mtv when separating from fixed B.
		outMtv = mtv * -1.f;
		return true;
	}

	return false;
}

bool Map::TryGetEntityVsEntityMTV( Entity* entityA, Entity* entityB, Vec3& outMtvA ) const
{
	outMtvA = Vec3::ZERO;
	if (entityA == nullptr || entityB == nullptr)
	{
		return false;
	}
	if (entityA->m_collision == nullptr || entityB->m_collision == nullptr)
	{
		return false;
	}
	if (!entityA->m_collision->IsCollisionEnabled() || !entityB->m_collision->IsCollisionEnabled())
	{
		return false;
	}

	auto getCapsuleBone = []( ShapeComponent* shape, Vec3& outStart, Vec3& outEnd, float& outRadius ) -> bool
	{
		if (shape->GetCollisionShape() == CollisionShape::CAPSULE)
		{
			CapsuleComponent* capsule = static_cast<CapsuleComponent*>( shape );
			outStart = capsule->GetWorldBoneStart();
			outEnd = capsule->GetWorldBoneEnd();
			outRadius = capsule->GetScaledRadius();
			return true;
		}
		if (shape->GetCollisionShape() == CollisionShape::SPHERE)
		{
			SphereComponent* sphere = static_cast<SphereComponent*>( shape );
			outStart = sphere->GetWorldPosition();
			outEnd = outStart;
			outRadius = sphere->GetScaledRadius();
			return true;
		}
		return false;
	};

	Vec3 startA;
	Vec3 endA;
	float radiusA = 0.f;
	Vec3 startB;
	Vec3 endB;
	float radiusB = 0.f;

	bool const aIsCapsuleLike = getCapsuleBone( entityA->m_collision, startA, endA, radiusA );
	bool const bIsCapsuleLike = getCapsuleBone( entityB->m_collision, startB, endB, radiusB );

	if (aIsCapsuleLike && bIsCapsuleLike)
	{
		Vec3 closestA;
		Vec3 closestB;
		float const distanceSquared = GetLineSegmentsDistanceSquared3D( startA, endA, startB, endB, closestA, closestB );
		float const radiusSum = radiusA + radiusB;
		if (!isfinite( distanceSquared ) || distanceSquared > (radiusSum * radiusSum))
		{
			return false;
		}

		float const distance = sqrtf( distanceSquared );
		float const overlap = radiusSum - distance;
		Vec3 directionXY( closestA.x - closestB.x, closestA.y - closestB.y, 0.f );
		if (directionXY.GetLengthSquared() <= 0.000001f)
		{
			directionXY = Vec3( 1.f, 0.f, 0.f );
		}
		else
		{
			directionXY = directionXY.GetNormalized();
		}
		outMtvA = directionXY * overlap;
		if (!isfinite( outMtvA.x ) || !isfinite( outMtvA.y ) || !isfinite( outMtvA.z ))
		{
			outMtvA = Vec3::ZERO;
			return false;
		}
		return true;
	}

	// Fallback for cube pairs / mixed shapes via OBB.
	OBB3 obbA = entityA->m_collision->GetBoundsOBB3D();
	OBB3 obbB = entityB->m_collision->GetBoundsOBB3D();
	Vec3 mtv;
	if (!DoOBBsOverlap3D( obbA, obbB, mtv ))
	{
		return false;
	}
	outMtvA = Vec3( -mtv.x, -mtv.y, 0.f );
	return outMtvA.GetLengthSquared() > 0.000001f;
}

void Map::BuildGeometry( bool runNavSmokeTests )
{
	m_solidBounds.clear();
	m_solidObbs.clear();
	m_solidBoundsNavWalkable.clear();
	m_solidObbsNavWalkable.clear();
	m_renderBatches.clear();
	m_navMeshes.clear();
	m_interactableOverlayVerts.clear();

	if (m_definition == nullptr)
	{
		return;
	}

	MapGeometryBuildResult built = MapGeometryBaker::Bake( *m_definition, m_destroyedGeometryNames );
	m_solidBounds = std::move( built.solidBounds );
	m_solidObbs = std::move( built.solidObbs );
	m_solidBoundsNavWalkable = std::move( built.solidBoundsNavWalkable );
	m_solidObbsNavWalkable = std::move( built.solidObbsNavWalkable );
	m_renderBatches = std::move( built.renderBatches );

	BuildInteractableOverlays();

	m_navMeshes.reserve( m_definition->m_navMeshVolumes.size() );
	for (NavMeshVolumeDefinition const& volumeDef : m_definition->m_navMeshVolumes)
	{
		NavMesh navMesh;
		navMesh.BakeFrom( *this, volumeDef );
		m_navMeshes.push_back( navMesh );
	}

	// Acceptance smoke tests on the single TestMap volume (cave + arena + ramp + gallery).
	if (!runNavSmokeTests)
	{
		return;
	}

	NavMesh const* arena = GetNavMeshByName( "Arena" );
	if (arena != nullptr)
	{
		Vec3 const galleryStart( 3.0f, 2.5f, 0.0f );	// ground beside ramp
		Vec3 const galleryGoal( 0.0f, -6.5f, 3.0f );	// south gallery
		NavPath const galleryPath = arena->FindPath( galleryStart, galleryGoal );
		DebugAddMessage(
			Stringf( "Nav path floor->gallery: %s (%d wp)",
				galleryPath.m_isValid ? "OK" : "FAIL",
				(int)galleryPath.m_waypoints.size() ),
			8.f,
			galleryPath.m_isValid ? Rgba8::GREEN : Rgba8::RED,
			galleryPath.m_isValid ? Rgba8::GREEN : Rgba8::RED );

		Vec3 const caveStart( 0.0f, 10.0f, 0.0f );	// cave floor spawn
		Vec3 const arenaGoal( 0.0f, 0.0f, 0.0f );	// arena center
		NavPath const cavePath = arena->FindPath( caveStart, arenaGoal );
		DebugAddMessage(
			Stringf( "Nav path cave->arena: %s (%d wp)",
				cavePath.m_isValid ? "OK" : "FAIL",
				(int)cavePath.m_waypoints.size() ),
			8.f,
			cavePath.m_isValid ? Rgba8::GREEN : Rgba8::RED,
			cavePath.m_isValid ? Rgba8::GREEN : Rgba8::RED );
	}
}

void Map::BuildInteractableOverlays()
{
	m_interactableOverlayVerts.clear();
	if (m_definition == nullptr)
	{
		return;
	}

	constexpr float overlayLift = 0.05f;
	Rgba8 const shopRangeColor( 255, 48, 48, 110 );

	for (MapInteractableDefinition const& interactable : m_definition->m_interactables)
	{
		if (interactable.m_type != "Shop" || interactable.m_radius <= 0.f)
		{
			continue;
		}

		Vec3 overlayCenter = interactable.m_center;
		overlayCenter.z += overlayLift;

		size_t const startIndex = m_interactableOverlayVerts.size();
		AddVertsForDisc2D( m_interactableOverlayVerts, Vec2( overlayCenter.x, overlayCenter.y ), interactable.m_radius, shopRangeColor );
		for (size_t i = startIndex; i < m_interactableOverlayVerts.size(); ++i)
		{
			m_interactableOverlayVerts[i].m_position.z = overlayCenter.z;
		}
	}
}

void Map::RenderInteractableOverlays() const
{
	if (m_interactableOverlayVerts.empty())
	{
		return;
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( m_interactableOverlayVerts );
}

bool Map::IsPlayerEntity( Entity const* entity ) const
{
	return entity != nullptr && m_playerController != nullptr && m_playerController->GetEntity() == entity;
}
