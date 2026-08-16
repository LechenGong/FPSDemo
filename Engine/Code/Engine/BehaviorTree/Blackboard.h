#pragma once

#include <string>
#include <any>
#include <unordered_map>

#include "Engine/Core/HashedCaseInsensitiveString.hpp"

// Case Insensitive
class Blackboard
{
public:
	template<typename T>
	void SetValue( std::string const& key, T value )
	{
		m_data[HashedCaseInsensitiveString{ key }] = value;
	}

	template<typename T>
	T GetValue( std::string const& key )
	{
		return std::any_cast<T>(m_data.at( key ));
	}

	template<typename T>
	T SafeGetValue( std::string const& key )
	{
		auto& value = m_data[key];
		if (!value.has_value())
		{
			value = T();
		}
		return std::any_cast<T>(value);
	}
	template<typename T>
	T GetValueWithDefault( std::string const& key, T defaultValue )
	{
		auto& value = m_data[key];
		if (!value.has_value())
		{
			return defaultValue;
		}
		return std::any_cast<T>(value);
	}

	void RemoveValue( std::string const& key )
	{
		m_data.erase( key );
	}

private:
	std::unordered_map<HashedCaseInsensitiveString, std::any> m_data;
};

