#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType )
	: m_spriteSheet( sheet )
	, m_startSpriteIndex( startSpriteIndex )
	, m_endSpriteIndex( endSpriteIndex )
	, m_durationSeconds( durationSeconds )
	, m_playbackType( playbackType )
{
	m_cycleFrames = endSpriteIndex - startSpriteIndex + 1;
}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime( float seconds ) const
{
	while (seconds >= m_durationSeconds)
	{
		seconds -= m_durationSeconds;
	}
	int indexOfFrame = static_cast<int>(seconds / m_durationSeconds * m_cycleFrames);
	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::ONCE:
		return (indexOfFrame > m_cycleFrames) ? m_spriteSheet.GetSpriteDef( m_endSpriteIndex ) : m_spriteSheet.GetSpriteDef( m_startSpriteIndex + indexOfFrame );
	case SpriteAnimPlaybackType::LOOP:
		while (indexOfFrame > m_cycleFrames)
			indexOfFrame -= m_cycleFrames;
		return m_spriteSheet.GetSpriteDef( m_startSpriteIndex + indexOfFrame );
	case SpriteAnimPlaybackType::PINGPONG:
		while (indexOfFrame > m_cycleFrames * 2 - 1)
			indexOfFrame -= m_cycleFrames * 2 - 1;
		return m_spriteSheet.GetSpriteDef( m_startSpriteIndex + m_cycleFrames - abs( indexOfFrame - m_cycleFrames ) );
	default:
		ERROR_AND_DIE( "Unknown Animation Type" );
	}
}
