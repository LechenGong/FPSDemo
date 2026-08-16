#pragma once

#include <string>
#include <vector>

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Entity.hpp"

enum class MapTextureWrapMode
{
	Stretch,
	Tile
};

enum class MapGeometryType
{
	AABB,
	OBB
};

struct MapFaceMaterialDefinition
{
	bool m_enabled = false; // false when uv is omitted in XML
	std::string m_material;
	IntVec2 m_spriteCoords = IntVec2( 0, 0 );
	MapTextureWrapMode m_wrapMode = MapTextureWrapMode::Stretch;
};

struct MapGeometryDefinition
{
	MapGeometryType m_type = MapGeometryType::AABB;
	std::string m_name;
	Vec3 m_size = Vec3( 1.f, 1.f, 1.f ); // full dimensions
	Vec3 m_center = Vec3::ZERO;
	Vec3 m_iBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 m_jBasis = Vec3( 0.f, 1.f, 0.f );
	Vec3 m_kBasis = Vec3( 0.f, 0.f, 1.f );
	bool m_navWalkable = false; // opt-in: only marked solids become nav candidates

	std::string m_defaultMaterial = "Terrain";
	MapTextureWrapMode m_defaultWrapMode = MapTextureWrapMode::Stretch;

	MapFaceMaterialDefinition m_top;
	MapFaceMaterialDefinition m_bottom;
	MapFaceMaterialDefinition m_side;
};

struct NavMeshVolumeDefinition
{
	std::string m_name;
	Vec3 m_center = Vec3::ZERO;
	Vec3 m_size = Vec3( 1.f, 1.f, 1.f ); // full dimensions (also filled from mins/maxs)

	AABB3 GetBounds() const
	{
		Vec3 halfSize = m_size * 0.5f;
		return AABB3( m_center - halfSize, m_center + halfSize );
	}
};

struct MapInteractableDefinition
{
	std::string m_name;
	std::string m_type; // "Shop", "StartTrigger"
	Vec3 m_center = Vec3::ZERO;
	Vec3 m_size = Vec3::ZERO; // StartTrigger AABB; zero = unused
	float m_radius = 2.f;
};

struct SurvivalPhaseDefinition
{
	float m_startTime = 0.f;
	float m_endTime = 0.f;		// used when m_waves == 0: spawn while t < end
	float m_period = 10.f;
	int m_count = 1;			// demons per wave
	int m_waves = 0;			// 0 = derive from start/end/period
};

struct SurvivalOvertimeDefinition
{
	int m_count = 0;
	float m_stagger = 0.4f;
	float m_yStep = -0.7f;
	std::vector<int> m_groupSizes;
};

struct SurvivalWaveDefinition
{
	bool m_enabled = false;
	std::string m_entityTypeName = "Demon";
	float m_durationSeconds = 180.f;
	int m_maxLiveBeforeOvertime = 12;
	EulerAngles m_orientation;
	float m_slotY = 10.5f;
	float m_slotZ = 0.f;
	std::vector<float> m_slotXs;
	std::vector<SurvivalPhaseDefinition> m_phases;
	SurvivalOvertimeDefinition m_overtime;
};

class MapDefinition
{
public:
	MapDefinition() = default;
	explicit MapDefinition( XmlElement const& element );

	std::string m_name;
	std::vector<MapGeometryDefinition> m_geometries;
	std::vector<NavMeshVolumeDefinition> m_navMeshVolumes;
	std::vector<MapInteractableDefinition> m_interactables;
	std::vector<SpawnInfo> m_spawnInfos;
	SurvivalWaveDefinition m_survivalWaves;

public:
	static void InitializeDefinitions( char const* path = "Data/Definitions/MapDefinitions.xml" );
	static void ClearDefinitions();
	static MapDefinition const* GetByName( std::string const& name );

	static std::vector<MapDefinition> s_definitions;
};
