#include "Engine/General/ActorUID.hpp"

ActorUID const ActorUID::INVALID = ActorUID( 0xFFFF0000u, 0x0000FFFFu );

ActorUID::ActorUID()
{
}

ActorUID::ActorUID( unsigned int salt, unsigned int index )
{
	m_data = ((0x0000FFFFu & salt) << 16) | (0x0000FFFFu & index);
}

bool ActorUID::isValid() const
{
	return *this != ActorUID::INVALID;
}

unsigned int ActorUID::GetIndex() const
{
	return m_data & 0x0000FFFFu;
}

unsigned int ActorUID::GetSalt() const
{
	return m_data & 0xFFFF0000u;
}

bool ActorUID::operator==( ActorUID const& other ) const
{
	return this->m_data == other.m_data;
}

bool ActorUID::operator!=( ActorUID const& other ) const
{
	return this->m_data != other.m_data;
}
