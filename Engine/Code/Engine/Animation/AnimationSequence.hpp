#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Quat.hpp"

struct KeyFrame;

struct AnimationEvent
{
	std::string name;
	float time;
	int collisionIndex;
	bool flag;
	bool persisting;
	std::vector<int> damageTypeIndex;
	std::vector<float> damageValue;
};

class AnimationSequence
{
public:
	AnimationSequence( std::string name, float frameRate, float duration, float playBackSpeed = true, bool looping = true )
		: m_name( name )
		, m_frameRate( frameRate )
		, m_duration( duration )
		, m_playbackSpeed( playBackSpeed )
		, m_looping( looping )
	{}

	~AnimationSequence() = default;

	KeyFrame* GetKeyFrameHeadOfJoint( const std::string& jointName );
	Vec3 GetRootTranslationAtTime( float currentTime, float deltaSeconds );
	Quat GetRootRotationAtTime( float currentTime, float deltaSeconds );
	std::vector<AnimationEvent> const& GetEvents() const;


	void ExportToXML( const std::string& filePath ) const;
	static AnimationSequence* ImportFromXML( const std::string& filePath );

public:
	std::string m_name;
	float m_frameRate = 24.f;
	float m_duration = 0.f;
	float m_playbackSpeed = 1.f;
	bool m_looping = true;
	std::unordered_map<std::string, KeyFrame*> m_keyFrames;
	std::vector<Vec3> m_rootTranslation;
	std::vector<Vec3> m_rootRotation;
	std::vector<AnimationEvent> m_events;
};
