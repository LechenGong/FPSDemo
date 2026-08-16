#pragma once

#include <string>

class CharacterDefinition
{
public:
	std::string m_skeletalMeshPath;
	std::string m_animationPath;

	CharacterDefinition() {};
	CharacterDefinition( std::string const& xmlPath );
};
