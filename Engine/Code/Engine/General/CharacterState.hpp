#pragma once

#include <string>

class CharacterState
{
public:
	CharacterState() = default;
	CharacterState( std::string stateName );
	void SetStateName( std::string stateName );
	std::string const GetStateName() const;

protected:
	std::string m_stateName = "";
};