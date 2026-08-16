#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <cstdlib>

std::vector<MapDefinition> MapDefinition::s_definitions;

static MapTextureWrapMode ParseWrapMode( std::string const& wrapModeText )
{
	std::string lower = wrapModeText;
	for (char& c : lower)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = (char)(c - 'A' + 'a');
		}
	}

	if (lower == "tile" || lower == "wrap" || lower == "repeat")
	{
		return MapTextureWrapMode::Tile;
	}

	return MapTextureWrapMode::Stretch;
}

static void ParseFaceMaterialDefinition( XmlElement const& faceElement, MapFaceMaterialDefinition& outFace,
	std::string const& defaultMaterial, MapTextureWrapMode defaultWrapMode )
{
	char const* uvAttribute = faceElement.Attribute( "uv" );
	if (uvAttribute == nullptr || uvAttribute[0] == '\0')
	{
		outFace.m_enabled = false;
		return;
	}

	outFace.m_enabled = true;
	outFace.m_spriteCoords = IntVec2( std::string( uvAttribute ) );
	outFace.m_material = ParseXmlAttribute( faceElement, "material", defaultMaterial );
	outFace.m_wrapMode = defaultWrapMode;

	char const* wrapAttribute = faceElement.Attribute( "wrapMode" );
	if (wrapAttribute != nullptr && wrapAttribute[0] != '\0')
	{
		outFace.m_wrapMode = ParseWrapMode( wrapAttribute );
	}
}

static void ParseFaceChildren( XmlElement const& element, MapGeometryDefinition& geometry )
{
	XmlElement const* faceElement = element.FirstChildElement();
	while (faceElement)
	{
		std::string tagName = faceElement->Name();
		if (tagName == "Top" || tagName == "top")
		{
			ParseFaceMaterialDefinition( *faceElement, geometry.m_top, geometry.m_defaultMaterial, geometry.m_defaultWrapMode );
		}
		else if (tagName == "Bottom" || tagName == "bottom")
		{
			ParseFaceMaterialDefinition( *faceElement, geometry.m_bottom, geometry.m_defaultMaterial, geometry.m_defaultWrapMode );
		}
		else if (tagName == "Side" || tagName == "Sides" || tagName == "side" || tagName == "sides")
		{
			ParseFaceMaterialDefinition( *faceElement, geometry.m_side, geometry.m_defaultMaterial, geometry.m_defaultWrapMode );
		}
		faceElement = faceElement->NextSiblingElement();
	}
}

static MapGeometryDefinition ParseAABBGeometryDefinition( XmlElement const& element )
{
	MapGeometryDefinition geometry;
	geometry.m_type = MapGeometryType::AABB;
	geometry.m_name = ParseXmlAttribute( element, "name", "" );
	geometry.m_size = ParseXmlAttribute( element, "size", Vec3( 1.f, 1.f, 1.f ) );
	geometry.m_center = ParseXmlAttribute( element, "center", Vec3::ZERO );
	geometry.m_navWalkable = ParseXmlAttribute( element, "navWalkable", false );
	geometry.m_defaultMaterial = ParseXmlAttribute( element, "material", "Terrain" );
	geometry.m_defaultWrapMode = ParseWrapMode( ParseXmlAttribute( element, "wrapMode", "Stretch" ) );
	ParseFaceChildren( element, geometry );
	return geometry;
}

static MapGeometryDefinition ParseOBBGeometryDefinition( XmlElement const& element )
{
	MapGeometryDefinition geometry;
	geometry.m_type = MapGeometryType::OBB;
	geometry.m_name = ParseXmlAttribute( element, "name", "" );
	geometry.m_navWalkable = ParseXmlAttribute( element, "navWalkable", false );
	geometry.m_defaultMaterial = ParseXmlAttribute( element, "material", "Terrain" );
	geometry.m_defaultWrapMode = ParseWrapMode( ParseXmlAttribute( element, "wrapMode", "Stretch" ) );

	char const* rampStartAttr = element.Attribute( "rampStart" );
	char const* rampEndAttr = element.Attribute( "rampEnd" );
	if (rampStartAttr != nullptr && rampEndAttr != nullptr)
	{
		Vec3 rampStart = ParseXmlAttribute( element, "rampStart", Vec3::ZERO );
		Vec3 rampEnd = ParseXmlAttribute( element, "rampEnd", Vec3::ZERO );
		float width = ParseXmlAttribute( element, "width", 4.f );
		float thickness = ParseXmlAttribute( element, "thickness", 0.5f );
		bool solidUnder = ParseXmlAttribute( element, "solid", false );

		Vec3 along = rampEnd - rampStart;
		float length = along.GetLength();
		GUARANTEE_OR_DIE( length > 0.0001f, "OBB rampStart/rampEnd produced zero length" );

		geometry.m_iBasis = along.GetNormalized();
		geometry.m_jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), geometry.m_iBasis );
		if (geometry.m_jBasis.GetLengthSquared() < 0.001f)
		{
			geometry.m_jBasis = Vec3( 1.f, 0.f, 0.f );
		}
		else
		{
			geometry.m_jBasis = geometry.m_jBasis.GetNormalized();
		}
		geometry.m_kBasis = CrossProduct3D( geometry.m_iBasis, geometry.m_jBasis ).GetNormalized();
		geometry.m_jBasis = CrossProduct3D( geometry.m_kBasis, geometry.m_iBasis ).GetNormalized();

		if (solidUnder)
		{
			float zTop = (rampStart.z > rampEnd.z) ? rampStart.z : rampEnd.z;
			float zBottom = (rampStart.z < rampEnd.z) ? rampStart.z : rampEnd.z;
			float kZ = fabsf( geometry.m_kBasis.z );
			GUARANTEE_OR_DIE( kZ > 0.001f, "Cannot make solid ramp with near-horizontal K basis" );
			thickness = (zTop - zBottom) / kZ;
		}

		// Place so the TOP (+K) surface follows rampStart->rampEnd.
		Vec3 topMid = rampStart + along * 0.5f;
		geometry.m_center = topMid - geometry.m_kBasis * (thickness * 0.5f);
		geometry.m_size = Vec3( length, width, thickness );
	}
	else
	{
		geometry.m_center = ParseXmlAttribute( element, "center", Vec3::ZERO );
		geometry.m_size = ParseXmlAttribute( element, "size", Vec3( 1.f, 1.f, 1.f ) );
		geometry.m_iBasis = ParseXmlAttribute( element, "iBasis", Vec3( 1.f, 0.f, 0.f ) ).GetNormalized();
		geometry.m_jBasis = ParseXmlAttribute( element, "jBasis", Vec3( 0.f, 1.f, 0.f ) ).GetNormalized();
		geometry.m_kBasis = CrossProduct3D( geometry.m_iBasis, geometry.m_jBasis ).GetNormalized();
		geometry.m_jBasis = CrossProduct3D( geometry.m_kBasis, geometry.m_iBasis ).GetNormalized();
	}

	ParseFaceChildren( element, geometry );
	return geometry;
}

static NavMeshVolumeDefinition ParseNavMeshVolumeDefinition( XmlElement const& element )
{
	NavMeshVolumeDefinition volume;
	volume.m_name = ParseXmlAttribute( element, "name", "" );
	GUARANTEE_OR_DIE( !volume.m_name.empty(), "NavMeshVolume missing name attribute" );

	char const* minsAttr = element.Attribute( "mins" );
	char const* maxsAttr = element.Attribute( "maxs" );
	if (minsAttr != nullptr && maxsAttr != nullptr)
	{
		Vec3 mins = ParseXmlAttribute( element, "mins", Vec3::ZERO );
		Vec3 maxs = ParseXmlAttribute( element, "maxs", Vec3::ZERO );
		GUARANTEE_OR_DIE( maxs.x > mins.x && maxs.y > mins.y && maxs.z > mins.z,
			Stringf( "NavMeshVolume '%s' requires maxs > mins", volume.m_name.c_str() ).c_str() );
		volume.m_center = (mins + maxs) * 0.5f;
		volume.m_size = maxs - mins;
	}
	else
	{
		volume.m_center = ParseXmlAttribute( element, "center", Vec3::ZERO );
		volume.m_size = ParseXmlAttribute( element, "size", Vec3( 1.f, 1.f, 1.f ) );
	}

	GUARANTEE_OR_DIE( volume.m_size.x > 0.f && volume.m_size.y > 0.f && volume.m_size.z > 0.f,
		Stringf( "NavMeshVolume '%s' requires positive size", volume.m_name.c_str() ).c_str() );
	return volume;
}

static MapInteractableDefinition ParseInteractableDefinition( XmlElement const& element )
{
	MapInteractableDefinition interactable;
	interactable.m_name = ParseXmlAttribute( element, "name", "" );
	GUARANTEE_OR_DIE( !interactable.m_name.empty(), "Interactable missing name attribute" );

	interactable.m_type = ParseXmlAttribute( element, "type", "" );
	GUARANTEE_OR_DIE( !interactable.m_type.empty(),
		Stringf( "Interactable '%s' missing type attribute", interactable.m_name.c_str() ).c_str() );

	interactable.m_center = ParseXmlAttribute( element, "center", Vec3::ZERO );
	interactable.m_size = ParseXmlAttribute( element, "size", Vec3::ZERO );
	interactable.m_radius = ParseXmlAttribute( element, "radius", 0.f );

	if (interactable.m_type == "StartTrigger")
	{
		GUARANTEE_OR_DIE( interactable.m_size.x > 0.f && interactable.m_size.y > 0.f && interactable.m_size.z > 0.f,
			Stringf( "StartTrigger '%s' requires positive size", interactable.m_name.c_str() ).c_str() );
	}
	else
	{
		if (interactable.m_radius <= 0.f)
		{
			interactable.m_radius = 2.f;
		}
		GUARANTEE_OR_DIE( interactable.m_radius > 0.f,
			Stringf( "Interactable '%s' requires positive radius", interactable.m_name.c_str() ).c_str() );
	}
	return interactable;
}

static void ParseFloatListAttribute( XmlElement const& element, char const* attributeName, std::vector<float>& outValues )
{
	outValues.clear();
	Strings const tokens = ParseXmlAttribute( element, attributeName, Strings() );
	for (std::string const& token : tokens)
	{
		if (token.empty())
		{
			continue;
		}
		outValues.push_back( (float)atof( token.c_str() ) );
	}
}

static void ParseIntListAttribute( XmlElement const& element, char const* attributeName, std::vector<int>& outValues )
{
	outValues.clear();
	Strings const tokens = ParseXmlAttribute( element, attributeName, Strings() );
	for (std::string const& token : tokens)
	{
		if (token.empty())
		{
			continue;
		}
		outValues.push_back( atoi( token.c_str() ) );
	}
}

static SurvivalPhaseDefinition ParseSurvivalPhaseDefinition( XmlElement const& element )
{
	SurvivalPhaseDefinition phase;
	phase.m_startTime = ParseXmlAttribute( element, "start", 0.f );
	phase.m_endTime = ParseXmlAttribute( element, "end", 0.f );
	phase.m_period = ParseXmlAttribute( element, "period", 10.f );
	phase.m_count = ParseXmlAttribute( element, "count", 1 );
	phase.m_waves = ParseXmlAttribute( element, "waves", 0 );
	GUARANTEE_OR_DIE( phase.m_period > 0.f, "SurvivalWaves Phase requires positive period" );
	GUARANTEE_OR_DIE( phase.m_count > 0, "SurvivalWaves Phase requires positive count" );
	GUARANTEE_OR_DIE( phase.m_waves > 0 || phase.m_endTime > phase.m_startTime,
		"SurvivalWaves Phase requires waves>0 or end>start" );
	return phase;
}

static SurvivalWaveDefinition ParseSurvivalWaveDefinition( XmlElement const& element )
{
	SurvivalWaveDefinition waves;
	waves.m_enabled = true;
	waves.m_entityTypeName = ParseXmlAttribute( element, "entity", std::string( "Demon" ) );
	waves.m_durationSeconds = ParseXmlAttribute( element, "duration", 180.f );
	waves.m_maxLiveBeforeOvertime = ParseXmlAttribute( element, "maxLiveBeforeOvertime", 12 );
	waves.m_orientation = ParseXmlAttribute( element, "orientation", EulerAngles( -90.f, 0.f, 0.f ) );
	GUARANTEE_OR_DIE( !waves.m_entityTypeName.empty(), "SurvivalWaves missing entity attribute" );
	GUARANTEE_OR_DIE( waves.m_durationSeconds > 0.f, "SurvivalWaves requires positive duration" );

	XmlElement const* childElement = element.FirstChildElement();
	while (childElement)
	{
		std::string const tagName = childElement->Name();
		if (tagName == "CaveSlots")
		{
			waves.m_slotY = ParseXmlAttribute( *childElement, "y", 10.5f );
			waves.m_slotZ = ParseXmlAttribute( *childElement, "z", 0.f );
			ParseFloatListAttribute( *childElement, "x", waves.m_slotXs );
			GUARANTEE_OR_DIE( !waves.m_slotXs.empty(), "SurvivalWaves CaveSlots requires x list" );
		}
		else if (tagName == "Phase")
		{
			waves.m_phases.push_back( ParseSurvivalPhaseDefinition( *childElement ) );
		}
		else if (tagName == "Overtime")
		{
			waves.m_overtime.m_count = ParseXmlAttribute( *childElement, "count", 0 );
			waves.m_overtime.m_stagger = ParseXmlAttribute( *childElement, "stagger", 0.4f );
			waves.m_overtime.m_yStep = ParseXmlAttribute( *childElement, "yStep", -0.7f );
			ParseIntListAttribute( *childElement, "groups", waves.m_overtime.m_groupSizes );
			GUARANTEE_OR_DIE( waves.m_overtime.m_count > 0, "SurvivalWaves Overtime requires positive count" );
			GUARANTEE_OR_DIE( waves.m_overtime.m_stagger >= 0.f, "SurvivalWaves Overtime stagger cannot be negative" );
		}
		childElement = childElement->NextSiblingElement();
	}

	GUARANTEE_OR_DIE( !waves.m_slotXs.empty(), "SurvivalWaves missing CaveSlots" );
	GUARANTEE_OR_DIE( !waves.m_phases.empty(), "SurvivalWaves requires at least one Phase" );
	GUARANTEE_OR_DIE( waves.m_overtime.m_count > 0, "SurvivalWaves missing Overtime" );
	return waves;
}

MapDefinition::MapDefinition( XmlElement const& element )
{
	m_name = ParseXmlAttribute( element, "name", "" );

	XmlElement const* childElement = element.FirstChildElement();
	while (childElement)
	{
		std::string tagName = childElement->Name();
		if (tagName == "AABB" || tagName == "Geometry" || tagName == "AABB3")
		{
			m_geometries.push_back( ParseAABBGeometryDefinition( *childElement ) );
		}
		else if (tagName == "OBB" || tagName == "OBB3")
		{
			m_geometries.push_back( ParseOBBGeometryDefinition( *childElement ) );
		}
		else if (tagName == "NavMeshVolume")
		{
			m_navMeshVolumes.push_back( ParseNavMeshVolumeDefinition( *childElement ) );
		}
		else if (tagName == "Interactable")
		{
			m_interactables.push_back( ParseInteractableDefinition( *childElement ) );
		}
		else if (tagName == "SpawnInfo")
		{
			SpawnInfo spawnInfo;
			spawnInfo.m_entityTypeName = ParseXmlAttribute( *childElement, "entity", "" );
			if (spawnInfo.m_entityTypeName.empty())
			{
				spawnInfo.m_entityTypeName = ParseXmlAttribute( *childElement, "actor", "" );
			}
			spawnInfo.m_position = ParseXmlAttribute( *childElement, "position", Vec3::ZERO );
			spawnInfo.m_orientation = ParseXmlAttribute( *childElement, "orientation", EulerAngles::ZERO );
			GUARANTEE_OR_DIE( !spawnInfo.m_entityTypeName.empty(), "SpawnInfo missing entity/actor attribute" );
			m_spawnInfos.push_back( spawnInfo );
		}
		else if (tagName == "SurvivalWaves")
		{
			m_survivalWaves = ParseSurvivalWaveDefinition( *childElement );
		}
		childElement = childElement->NextSiblingElement();
	}
}

void MapDefinition::InitializeDefinitions( char const* path )
{
	if (!s_definitions.empty())
	{
		return;
	}

	XmlDocument document;
	XmlError result = document.LoadFile( path );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to load map definitions: %s", path ).c_str() );

	XmlElement const* root = document.RootElement();
	GUARANTEE_OR_DIE( root != nullptr, Stringf( "Map definitions missing root: %s", path ).c_str() );

	XmlElement const* defElement = root->FirstChildElement( "MapDefinition" );
	while (defElement)
	{
		s_definitions.emplace_back( *defElement );
		defElement = defElement->NextSiblingElement( "MapDefinition" );
	}
}

void MapDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

MapDefinition const* MapDefinition::GetByName( std::string const& name )
{
	for (MapDefinition const& definition : s_definitions)
	{
		if (definition.m_name == name)
		{
			return &definition;
		}
	}
	return nullptr;
}
