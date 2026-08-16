#include "Engine/Core/HashedCaseInsensitiveString.hpp"

HashedCaseInsensitiveString::HashedCaseInsensitiveString()
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString( HashedCaseInsensitiveString const& copyFrom )
	: m_caseIntactText( copyFrom.m_caseIntactText )
	, m_lowerCaseHash( copyFrom.m_lowerCaseHash )
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString( char const* text )
{
	m_lowerCaseHash = 0;
	for (int i = 0; text[i] != '\0'; i++) 
	{
		m_lowerCaseHash *= 31;
		m_lowerCaseHash += (unsigned int)tolower( text[i] );
	}
	m_caseIntactText = text;
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString( std::string const& text )
{
	m_lowerCaseHash = 0;
	for (int i = 0; i < text.length(); i++)
	{
		m_lowerCaseHash *= 31;
		m_lowerCaseHash += (unsigned int)tolower( text[i] );
	}
	m_caseIntactText = text;
}

HashedCaseInsensitiveString::~HashedCaseInsensitiveString()
{
}

bool HashedCaseInsensitiveString::operator<( HashedCaseInsensitiveString const& compare ) const
{
	if (m_lowerCaseHash != compare.m_lowerCaseHash)
	{
		return m_lowerCaseHash < compare.m_lowerCaseHash;
	}
	return _stricmp( m_caseIntactText.c_str(), compare.m_caseIntactText.c_str() ) < 0;
}

bool HashedCaseInsensitiveString::operator==( HashedCaseInsensitiveString const& compare ) const
{
	return m_lowerCaseHash == compare.m_lowerCaseHash
		&& _stricmp( m_caseIntactText.c_str(), compare.m_caseIntactText.c_str() ) == 0;
}

bool HashedCaseInsensitiveString::operator!=( HashedCaseInsensitiveString const& compare ) const
{
	return !(m_lowerCaseHash == compare.m_lowerCaseHash
		&& _stricmp( m_caseIntactText.c_str(), compare.m_caseIntactText.c_str() ) == 0);
}

bool HashedCaseInsensitiveString::operator==( char const* text ) const
{
	return _stricmp( m_caseIntactText.c_str(), text ) == 0;
}

bool HashedCaseInsensitiveString::operator!=( char const* text ) const
{
	return _stricmp( m_caseIntactText.c_str(), text ) != 0;
}

bool HashedCaseInsensitiveString::operator==( std::string const& text ) const
{
	return _stricmp( m_caseIntactText.c_str(), text.c_str() ) == 0;
}

bool HashedCaseInsensitiveString::operator!=( std::string const& text ) const
{
	return _stricmp( m_caseIntactText.c_str(), text.c_str() ) != 0;
}

void HashedCaseInsensitiveString::operator=( HashedCaseInsensitiveString const& assignFrom )
{
	m_lowerCaseHash = assignFrom.m_lowerCaseHash;
	m_caseIntactText = assignFrom.m_caseIntactText;
}

void HashedCaseInsensitiveString::operator=( char const* text )
{
	m_lowerCaseHash = 0;
	for (int i = 0; text[i] != '\0'; i++)
	{
		m_lowerCaseHash *= 31;
		m_lowerCaseHash += (unsigned int)tolower( text[i] );
	}
	m_caseIntactText = text;
}

void HashedCaseInsensitiveString::operator=( std::string const& text )
{
	m_lowerCaseHash = 0;
	for (int i = 0; i < text.length(); i++)
	{
		m_lowerCaseHash *= 31;
		m_lowerCaseHash += (unsigned int)tolower( text[i] );
	}
	m_caseIntactText = text;
}

namespace std
{
	size_t hash<HashedCaseInsensitiveString>::operator()( const HashedCaseInsensitiveString& obj ) const
	{
		return obj.GetHash();
	}
}