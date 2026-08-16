#pragma once

#include <string>
#include <vector>

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/RaycastUtil.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/Entity.hpp"
#include "Game/NavMesh.hpp"
#include "Game/MapGeometryBaker.hpp"

class Game;
class MapDefinition;
class PlayerController;
class BitmapFont;

class Map
{
public:
	Map( Game* owner, std::string const& mapName );
	~Map();

	void Startup();
	void Update( float deltaSeconds );
	void Render() const;
	void RenderHUD() const;
	void Shutdown();

	Entity* SpawnActor( SpawnInfo const& spawnInfo );
	Entity* SpawnActor( EntityDefinition const* definition, Vec3 const& position, EulerAngles const& orientation );
	void DestroyEntity( Entity* entity );
	Entity* GetEntityByUID( EntityUID const& uid ) const;
	PlayerController* GetPlayerController() const { return m_playerController; }
	bool IsPlayerEntity( Entity const* entity ) const;
	std::vector<Entity*> const& GetEntities() const { return m_entities; }

	RaycastResult3D RaycastWorld( Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist ) const;
	RaycastResult3D SpherecastWorld( Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, float radius ) const;
	Vec3 GetHitEffectPosition( Vec3 const& impactPos, Vec3 const& impactNormal, Entity const* hitEntity, Vec3 const& viewerPos ) const;

	MapDefinition const* GetDefinition() const { return m_definition; }
	bool IsInsideShopRange( Vec3 const& position ) const;
	int GetMoney() const { return m_money; }
	void AddMoney( int amount );
	bool TrySpendMoney( int amount );
	void NotifyHealPurchased();
	void NotifyAmmoPurchased();
	void NotifyPlayerDamaged( Entity* victim, float amount );
	void NotifyEntityKilled( Entity* victim );
	void NotifyShotImpact( Vec3 const& impactPos );
	bool TogglePlayerInvincible();
	int KillAllDemons();
	bool IsSurvivalVictory() const { return m_survivalVictory; }
	bool IsMatchVictory() const { return m_survivalVictory; }
	bool IsMatchDefeat() const { return m_survivalDefeat; }
	bool IsMatchOver() const { return m_survivalVictory || m_survivalDefeat; }
	bool IsEndPromptVisible() const { return m_endPromptVisible; }
	std::vector<AABB3> const& GetSolidBounds() const { return m_solidBounds; }
	std::vector<OBB3> const& GetSolidObbs() const { return m_solidObbs; }
	std::vector<bool> const& GetSolidBoundsNavWalkable() const { return m_solidBoundsNavWalkable; }
	std::vector<bool> const& GetSolidObbsNavWalkable() const { return m_solidObbsNavWalkable; }
	std::vector<NavMesh> const& GetNavMeshes() const { return m_navMeshes; }
	std::vector<NavMesh>& GetNavMeshes() { return m_navMeshes; }
	NavMesh const* GetNavMeshByName( std::string const& name ) const;
	NavMesh* GetNavMeshByName( std::string const& name );
	NavMesh const* FindNavMeshForPosition( Vec3 const& position ) const;
	NavPath FindNavPath( Vec3 const& startPos, Vec3 const& goalPos ) const;
	bool FindNavPath( Vec3 const& startPos, Vec3 const& goalPos, std::vector<Vec3>& outWaypoints ) const;

private:
	void DebugDrawNavPathQuery() const;
	void TryPossessFromDefinition( Entity* entity );
	void CollidePhysics();
	void CollideEntitiesWithWorld();
	void CollideEntitiesWithEntities();
	void HandleProjectileCollisions();
	void HandleCorpseCleanup( float deltaSeconds );
	void ResolveProjectileImpact( Entity* projectile, Vec3 const& hitPos, Entity* hitEntity );
	void DoExplosion( Entity* source, Vec3 const& position, Entity* hitEntity );
	bool IsCombatProjectile( Entity const* entity ) const;
	bool CanProjectileHurt( Entity const* projectile, Entity const* target ) const;
	float GetEntityCollisionRadius( Entity const* entity ) const;
	Vec3 GetEntityCollisionCenter( Entity const* entity ) const;
	bool TryGetEntityVsWorldMTV( Entity* entity, OBB3 const& worldObb, Vec3& outMtv ) const;
	bool TryGetEntityVsEntityMTV( Entity* entityA, Entity* entityB, Vec3& outMtvA ) const;
	static OBB3 MakeOBBFromAABB( AABB3 const& aabb );

private:
	void BuildGeometry( bool runNavSmokeTests = true );
	void DestroyStartBlock();
	void BuildInteractableOverlays();
	void RenderInteractableOverlays() const;
	void RenderShopPrompt() const;
	void RenderSurvivalHud() const;
	void RenderMatchEndOverlay() const;
	void RenderMatchStatLines( BitmapFont* font, Vec2 const& hudSize, std::string const& titleText ) const;
	void RenderCenteredHudText( BitmapFont* font, std::string const& text, float cellHeight, Rgba8 const& tint, float yOffset = 0.f, float xOffset = 0.f ) const;
	void CacheStartTriggerBounds();
	void BeginSurvivalIntro();
	void UpdateSurvivalIntro( float deltaSeconds );
	void UpdateMinuteBanners( float deltaSeconds );
	void PushMinuteBanner( std::string const& text, Rgba8 const& tint );
	bool IsPointOnStartTrigger( Vec3 const& point ) const;
	void BuildScheduledSurvivalWaves();
	void UpdateSurvivalWaves( float deltaSeconds );
	void TrySpawnScheduledWaves();
	void UpdateOvertimeSpawns( float deltaSeconds );
	void SpawnCaveGroup( int count, float yOffset );
	void SelectCaveSlotIndexes( int pickCount, std::vector<int>& outIndexes ) const;
	int CountLivingDemons() const;
	bool IsPlayerAlive() const;
	void CheckSurvivalVictory();
	void CheckSurvivalDefeat();
	void BeginVictory();
	void BeginDefeat();
	void SnapshotMatchStats();
	void UpdateMatchEndScreen( float deltaSeconds );

private:
	Game* m_game = nullptr;
	std::string m_mapName;
	MapDefinition const* m_definition = nullptr;
	std::vector<AABB3> m_solidBounds;
	std::vector<OBB3> m_solidObbs;
	std::vector<bool> m_solidBoundsNavWalkable;
	std::vector<bool> m_solidObbsNavWalkable;
	std::vector<MapRenderBatch> m_renderBatches;
	std::vector<NavMesh> m_navMeshes;
	std::vector<Vertex_PCU> m_interactableOverlayVerts;

	std::vector<Entity*> m_entities;
	unsigned int m_entitySalt = 0;
	PlayerController* m_playerController = nullptr;
	int m_money = 0;

	struct ScheduledSurvivalWave
	{
		float m_time = 0.f;
		int m_count = 0;
	};
	std::vector<ScheduledSurvivalWave> m_scheduledWaves;
	std::vector<int> m_overtimeGroupSizes;
	float m_survivalSeconds = 0.f;
	int m_nextScheduledWaveIndex = 0;
	int m_overtimeGroupIndex = 0;
	float m_overtimeGroupTimer = 0.f;
	bool m_overtimeActive = false;
	bool m_overtimeFinishedSpawning = false;
	bool m_survivalVictory = false;
	bool m_survivalDefeat = false;
	float m_endScreenSeconds = 0.f;
	float m_endScreenFade = 0.f;
	bool m_endPromptVisible = false;
	int m_killCount = 0;
	int m_healPurchaseCount = 0;
	int m_ammoPurchaseCount = 0;
	float m_damageTaken = 0.f;
	int m_resultKills = 0;
	float m_resultClearSeconds = 0.f;
	int m_resultDamageTaken = 0;
	int m_resultHealth = 0;
	int m_resultHeals = 0;
	int m_resultAmmoBuys = 0;

	AABB3 m_startTriggerBounds;
	bool m_hasStartTrigger = false;
	bool m_startTriggered = false;
	std::vector<std::string> m_destroyedGeometryNames;
	bool m_introActive = false;
	bool m_combatStarted = false;
	float m_introSeconds = 0.f;
	int m_introStep = 0;
	bool m_shownBanner02 = false;
	bool m_shownBanner01 = false;
	bool m_shownBannerOvertime = false;
	std::string m_minuteBannerText;
	Rgba8 m_minuteBannerTint = Rgba8::WHITE;
	float m_minuteBannerAge = 0.f;
	float m_minuteBannerDuration = 0.7f;
};
