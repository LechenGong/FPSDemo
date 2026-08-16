#include "Engine/General/CharacterDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

CharacterDefinition::CharacterDefinition( std::string const& filePath )
{
	XmlDocument doc;
	if (doc.LoadFile( filePath.c_str() ) != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE( Stringf( "Fail to load xml %s", filePath.c_str() ).c_str() );
		return;
	}

	XmlElement* root = doc.FirstChildElement( "CharacterDefinition" );
	if (!root)
	{
		ERROR_AND_DIE( Stringf( "Invalid XML format: Missing <CharacterDefinition> root element." ).c_str() );
		return;
	}

	XmlElement* skeletalMeshElem = root->FirstChildElement( "SkeletalMeshPath" );
	XmlElement* animationElem = root->FirstChildElement( "AnimationPath" );

	if (skeletalMeshElem && skeletalMeshElem->GetText())
		m_skeletalMeshPath = skeletalMeshElem->GetText();

	if (animationElem && animationElem->GetText())
		m_animationPath = animationElem->GetText();
}

