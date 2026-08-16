#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Model/ModelUtility.hpp"
#include "Engine/Core/EngineCommon.hpp"

KeyFrame* AnimationSequence::GetKeyFrameHeadOfJoint( const std::string& jointName )
{
	auto iter = m_keyFrames.find( jointName );
	if (iter != m_keyFrames.end())
	{
		return iter->second;
	}
	return nullptr;
}

Vec3 AnimationSequence::GetRootTranslationAtTime( float currentTime, float deltaSeconds )
{
	while (currentTime >= m_duration)
	{
		currentTime -= m_duration;
	}
	float currentFrame = currentTime * m_frameRate;
	int leftFrame = int( floorf( currentFrame ) );
	int rightFrame = leftFrame + 1;
	float frameRemainTime = (float)rightFrame / m_frameRate - currentTime;
	if (deltaSeconds <= frameRemainTime)
	{
		return m_rootTranslation[leftFrame] * deltaSeconds * m_frameRate * m_playbackSpeed;
	}
	else
	{
		Vec3 firstHalf = m_rootTranslation[leftFrame] * frameRemainTime * m_frameRate * m_playbackSpeed;
		float deltaSecondsRemain = deltaSeconds - frameRemainTime;
		if (rightFrame == (int)m_rootTranslation.size() - 1)
		{
			return firstHalf + m_rootTranslation[0] * deltaSecondsRemain * m_frameRate * m_playbackSpeed;
		}
		else
		{
			return firstHalf + m_rootTranslation[rightFrame + 1] * deltaSecondsRemain * m_frameRate * m_playbackSpeed;
		}
	}
}

Quat AnimationSequence::GetRootRotationAtTime( float currentTime, float deltaSeconds )
{
	while (currentTime >= m_duration)
	{
		currentTime -= m_duration;
	}
	float currentFrame = currentTime * m_frameRate;
	int leftFrame = int( floorf( currentFrame ) );
	int rightFrame = leftFrame + 1;
	float frameRemainTime = (float)rightFrame / m_frameRate - currentTime;
	if (deltaSeconds <= frameRemainTime)
	{
		return Quat( EulerAngles( m_rootRotation[leftFrame] * deltaSeconds * m_frameRate * m_playbackSpeed ) );
	}
	else
	{
		Quat firstHalf = Quat( EulerAngles( m_rootRotation[leftFrame] * frameRemainTime * m_frameRate * m_playbackSpeed ) );
		float deltaSecondsRemain = deltaSeconds - frameRemainTime;
		if (rightFrame == (int)m_rootTranslation.size() - 1)
		{
			return firstHalf * Quat( EulerAngles( m_rootRotation[0] * deltaSecondsRemain * m_frameRate * m_playbackSpeed ) );
		}
		else
		{
			return firstHalf * Quat( EulerAngles( m_rootRotation[rightFrame + 1] * deltaSecondsRemain * m_frameRate * m_playbackSpeed ) );
		}
	}
}

std::vector<AnimationEvent> const& AnimationSequence::GetEvents() const
{
	return m_events;
}

void AnimationSequence::ExportToXML( const std::string& filePath ) const
{
	using namespace tinyxml2;

	XmlDocument doc;
	XmlElement* root = doc.NewElement( "AnimationSequence" );
	root->SetAttribute( "Name", m_name.c_str() );
	root->SetAttribute( "FrameRate", m_frameRate );
	root->SetAttribute( "Duration", m_duration );
	root->SetAttribute( "PlaybackSpeed", m_playbackSpeed );
	root->SetAttribute( "Looping", m_looping );
	doc.InsertFirstChild( root );

	for (const auto& [jointName, keyFrame] : m_keyFrames)
	{
		XmlElement* jointElement = doc.NewElement( "Joint" );
		jointElement->SetAttribute( "Name", jointName.c_str() );
		root->InsertEndChild( jointElement );

		KeyFrame* currentFrame = keyFrame;
		while (currentFrame) 
		{
			XmlElement* keyFrameElement = doc.NewElement( "KeyFrame" );
			keyFrameElement->SetAttribute( "FrameNumber", currentFrame->m_frameNum );

			keyFrameElement->SetAttribute( "Transform", currentFrame->m_globalTransform.ToString().c_str() );
			jointElement->InsertEndChild( keyFrameElement );

			currentFrame = currentFrame->m_next;
		}
	}

	XmlElement* rootMotionElement = doc.NewElement( "RootMotion" );
	root->InsertEndChild( rootMotionElement );
	for (int i = 0; i < m_rootTranslation.size(); i++)
	{
		XmlElement* deltaTransformElement = doc.NewElement( "DeltaTransform" );
		deltaTransformElement->SetAttribute( "FrameNumber", i );
		deltaTransformElement->SetAttribute( "Translation", m_rootTranslation[i].ToString().c_str() );
		deltaTransformElement->SetAttribute( "Rotation", m_rootRotation[i].ToString().c_str() );
		rootMotionElement->InsertEndChild( deltaTransformElement );
	}

	XMLError eResult = doc.SaveFile( filePath.c_str() );
	if (eResult != XML_SUCCESS) {
		ERROR_AND_DIE( "Fail to save animation sequence to xml" );
	}
}

AnimationSequence* AnimationSequence::ImportFromXML( const std::string& filePath )
{
	using namespace tinyxml2;

	XMLDocument doc;
	XMLError eResult = doc.LoadFile( filePath.c_str() );
	if (eResult != XML_SUCCESS) 
	{
		ERROR_AND_DIE( "Error loading XML file: " + filePath );
		return nullptr;
	}

	XmlElement* root = doc.FirstChildElement( "AnimationSequence" );
	if (!root) 
	{
		ERROR_AND_DIE( "Invalid XML format: missing AnimationSequence element." );
		return nullptr;
	}

	std::string name = root->Attribute( "Name" );
	float frameRate = root->FloatAttribute( "FrameRate" );
	float duration = root->FloatAttribute( "Duration" );
	float playbackSpeed = root->FloatAttribute( "PlaybackSpeed" );
	bool looping = root->BoolAttribute( "Looping" );

	AnimationSequence* sequence = new AnimationSequence( name, frameRate, duration, playbackSpeed, looping );

	if (root->FirstChildElement( "Joint" ))
	{
		for (XmlElement* jointElement = root->FirstChildElement( "Joint" ); jointElement != nullptr; jointElement = jointElement->NextSiblingElement( "Joint" ))
		{
			std::string jointName = jointElement->Attribute( "Name" );

			KeyFrame* head = nullptr;
			KeyFrame* prev = nullptr;

			for (XmlElement* keyFrameElement = jointElement->FirstChildElement( "KeyFrame" ); keyFrameElement != nullptr; keyFrameElement = keyFrameElement->NextSiblingElement( "KeyFrame" ))
			{
				int64_t frameNum = keyFrameElement->Int64Attribute( "FrameNumber" );

				Mat44 globalTransform;
				globalTransform.FromString( keyFrameElement->Attribute( "Transform" ) );

				KeyFrame* newFrame = new KeyFrame();
				newFrame->m_frameNum = frameNum;
				static const Mat44 XRotation90 = Mat44::CreateXRotationDegrees( 90.f );
				static const Mat44 ZRotation90 = Mat44::CreateZRotationDegrees( 90.f );
				globalTransform = XRotation90 * globalTransform;
				globalTransform = ZRotation90 * globalTransform;
				newFrame->m_globalTransform = globalTransform;
				

				if (prev)
				{
					prev->m_next = newFrame;
				}
				else
				{
					head = newFrame;
				}

				prev = newFrame;
			}
			sequence->m_keyFrames[jointName] = head;
		}
	}

	XmlElement* rootMotionElement = root->FirstChildElement( "RootMotion" );
	if (rootMotionElement)
	{
		for (XmlElement* deltaTransformElement = rootMotionElement->FirstChildElement( "DeltaTransform" ); deltaTransformElement != nullptr; deltaTransformElement = deltaTransformElement->NextSiblingElement( "DeltaTransform" ))
		{
			//int64_t frameNum = deltaTransformElement->Int64Attribute( "FrameNumber" );
			Vec3 deltaTranslation;
			if (deltaTransformElement->Attribute( "Translation" ))
				deltaTranslation.SetFromText( deltaTransformElement->Attribute( "Translation" ) );
			Vec3 deltaRotation;
			if (deltaTransformElement->Attribute( "Rotation" ))
				deltaRotation.SetFromText( deltaTransformElement->Attribute( "Rotation" ) );

			sequence->m_rootTranslation.push_back( deltaTranslation );
			sequence->m_rootRotation.push_back( deltaRotation );
		}
	}

	XmlElement* eventElement = root->FirstChildElement( "Event" );
	if (eventElement)
	{
		for (XmlElement* collisionElement = eventElement->FirstChildElement( "ToggleCollision" ); collisionElement != nullptr; collisionElement = collisionElement->NextSiblingElement( "ToggleCollision" ))
		{
			AnimationEvent animEvent;
			animEvent.name = "ToggleCollision";
			animEvent.time = ParseXmlAttribute( *collisionElement, "Time", 0.f );
			animEvent.collisionIndex = ParseXmlAttribute( *collisionElement, "CollisionIndex", -1 );
			animEvent.flag = ParseXmlAttribute( *collisionElement, "Flag", true );
			animEvent.persisting = ParseXmlAttribute( *collisionElement, "Persisting", true );

			for (XmlElement* damageElement = collisionElement->FirstChildElement( "Damage" ); damageElement != nullptr; damageElement = damageElement->NextSiblingElement( "Damage" ))
			{
				int damageTypeIndex = ParseXmlAttribute( *damageElement, "Type", 0 );
				float damageValue = ParseXmlAttribute( *damageElement, "Value", 0.f );
				animEvent.damageTypeIndex.push_back( damageTypeIndex );
				animEvent.damageValue.push_back( damageValue );
			}

			sequence->m_events.push_back( animEvent );
		}
	}

	return sequence;
}
