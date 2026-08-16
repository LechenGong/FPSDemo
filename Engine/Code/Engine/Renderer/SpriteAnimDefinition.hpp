#pragma once

#include <string>

#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/XmlUtils.hpp"

enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};

//------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex,
		float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP );

	SpriteDefinition const& GetSpriteDefAtTime( float seconds ) const;

public:
	const SpriteSheet&	m_spriteSheet;
	int					m_startSpriteIndex = -1;
	int					m_endSpriteIndex = -1;
	float				m_durationSeconds = 1.f;
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;

	int m_cycleFrames = 0;
};
