#include <math.h>
#include <string>
#include <algorithm>

#include "XmlUtils.hpp"

int ParseXmlAttribute( XmlElement const& element, char const* attributeName, int defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return stoi( (std::string)data );
}

unsigned int ParseXmlAttribute( XmlElement const& element, char const* attributeName, unsigned int defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return (unsigned int)stoul( (std::string)data );
}


char ParseXmlAttribute( XmlElement const& element, char const* attributeName, char defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return data[0];
}

bool ParseXmlAttribute( XmlElement const& element, char const* attributeName, bool defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;

	if (strcmp(data, "true") == 0 || strcmp(data, "1") == 0)
		return true;
	else if (strcmp(data, "false") == 0 || strcmp(data, "0") == 0)
		return false;
	return defaultValue;
}

float ParseXmlAttribute( XmlElement const& element, char const* attributeName, float defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return stof( (std::string)data );
}

Rgba8 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return Rgba8( (std::string)data );
}

Vec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec2 const& defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return Vec2( (std::string)data );
}

Vec3 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec3 const& defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return Vec3( (std::string)data );
}

EulerAngles ParseXmlAttribute( XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return EulerAngles( (std::string)data );
}

// Quat ParseXmlAttribute( XmlElement const& element, char const* attributeName, Quat const& defaultValue )
// {
// 	const char* data = element.Attribute( attributeName );
// 	if (data == nullptr)
// 		return defaultValue;
// 	if ((std::string)data == "")
// 		return defaultValue;
// 	return Quat( (std::string)data );
// }

IntVec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return IntVec2( (std::string)data );
}

std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, std::string const& defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return data;
}

Strings ParseXmlAttribute( XmlElement const& element, char const* attributeName, Strings const& defaultValues )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValues;
	if ((std::string)data == "")
		return defaultValues;
	return Split( data, ',' );
}

std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, char const* defaultValue )
{
	const char* data = element.Attribute( attributeName );
	if (data == nullptr)
		return defaultValue;
	if ((std::string)data == "")
		return defaultValue;
	return data;
}
