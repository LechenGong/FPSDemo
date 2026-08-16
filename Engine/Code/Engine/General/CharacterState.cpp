#include "Engine/General/CharacterState.hpp"

CharacterState::CharacterState( std::string stateName )
    : m_stateName( stateName )
{
}

void CharacterState::SetStateName( std::string stateName )
{
    m_stateName = stateName;
}

std::string const CharacterState::GetStateName() const
{
    return m_stateName;
}
