#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"

void NamedStrings::PopulateFromXmlElementAttributes( XmlElement const& element )
{
	XmlAttribute const* iter = element.FirstAttribute();
	while (iter != nullptr)
	{
		SetValue( iter->Name(), iter->Value() );
		iter = iter->Next();

	}
}

bool NamedStrings::IsKeyExist( std::string const& keyName )
{
	return (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end());
}

void NamedStrings::SetValue( std::string const& keyName, std::string const& newValue )
{
	g_gameConfig.m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue( std::string const& keyName, std::string const& defaultValue ) const
{
	return (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end()) ? g_gameConfig.m_keyValuePairs[keyName] : defaultValue;
}

bool NamedStrings::GetValue( std::string const& keyName, bool defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		if (g_gameConfig.m_keyValuePairs[keyName] == "true")
			return true;
		else if (g_gameConfig.m_keyValuePairs[keyName] == "false")
			return false;
	}
	return defaultValue;
}

int NamedStrings::GetValue( std::string const& keyName, int defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		return stoi( g_gameConfig.m_keyValuePairs[keyName] );
	}
	return defaultValue;
}

float NamedStrings::GetValue( std::string const& keyName, float defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		return stof( g_gameConfig.m_keyValuePairs[keyName] );
	}
	return defaultValue;
}

std::string NamedStrings::GetValue( std::string const& keyName, char const* defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		return g_gameConfig.m_keyValuePairs[keyName];
	}
	return defaultValue;
}

Rgba8 NamedStrings::GetValue( std::string const& keyName, Rgba8 const& defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		return Rgba8( g_gameConfig.m_keyValuePairs[keyName] );
	}
	return defaultValue;
}

Vec2 NamedStrings::GetValue( std::string const& keyName, Vec2 const& defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		return Vec2( g_gameConfig.m_keyValuePairs[keyName] );
	}
	return defaultValue;
}

IntVec2 NamedStrings::GetValue( std::string const& keyName, IntVec2 const& defaultValue ) const
{
	if (g_gameConfig.m_keyValuePairs.find( keyName ) != g_gameConfig.m_keyValuePairs.end())
	{
		return IntVec2( g_gameConfig.m_keyValuePairs[keyName] );
	}
	return defaultValue;
}
