#include "Game/EntityUID.hpp"

EntityUID const EntityUID::INVALID = EntityUID( 0xFFFF0000u, 0x0000FFFFu );

EntityUID::EntityUID()
{
}

EntityUID::EntityUID( unsigned int salt, unsigned int index )
{
	m_data = ((0x0000FFFFu & salt) << 16) | (0x0000FFFFu & index);
}

bool EntityUID::isValid() const
{
	return *this != EntityUID::INVALID;
}

unsigned int EntityUID::GetIndex() const
{
	return m_data & 0x0000FFFFu;
}

unsigned int EntityUID::GetSalt() const
{
	return (m_data >> 16) & 0x0000FFFFu;
}

bool EntityUID::operator==( EntityUID const& other ) const
{
	return this->m_data == other.m_data;
}

bool EntityUID::operator!=( EntityUID const& other ) const
{
	return this->m_data != other.m_data;
}
