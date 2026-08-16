#include <iostream>
#include <fstream>

#include "Engine/Binary/BinaryUtil.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

void BufferWriter::AppendChar( char value )
{
	m_buffer.push_back( static_cast<char>(value) );
}

void BufferWriter::AppendByte( unsigned char value )
{
	m_buffer.push_back( value );
}

void BufferWriter::AppendBool( bool value )
{
	AppendByte( value ? 1 : 0 );
}

void BufferWriter::AppendShort( short value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( short ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( short )), sizeof( short ) );
	}
}

void BufferWriter::AppendUShort( unsigned short value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( unsigned short ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( unsigned short )), sizeof( unsigned short ) );
	}
}

void BufferWriter::AppendInt( int value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( int ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( int )), sizeof( int ) );
	}
}

void BufferWriter::AppendUInt( unsigned int value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( unsigned int ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( unsigned int )), sizeof( unsigned int ) );
	}
}

void BufferWriter::AppendInt32( long int value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( long int ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( long int )), sizeof( long int ) );
	}
}

void BufferWriter::AppendUInt32( unsigned long int value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( unsigned long int ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( unsigned long int )), sizeof( unsigned long int ) );
	}
}

void BufferWriter::AppendInt64( long long int value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( long long int ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( long long int )), sizeof( long long int ) );
	}
}

void BufferWriter::AppendUInt64( unsigned long long int value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( unsigned long long int ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( unsigned long long int )), sizeof( unsigned long long int ) );
	}
}

void BufferWriter::AppendFloat( float value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( float ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( float )), sizeof( float ) );
	}
}

void BufferWriter::AppendDouble( double value )
{
	unsigned char* bytes = reinterpret_cast<unsigned char*>(&value);
	m_buffer.insert( m_buffer.end(), bytes, bytes + sizeof( double ) );
	if (m_endianness != Endianness::LITTLE)
	{
		ReverseByte( m_buffer.data() + (m_buffer.size() - sizeof( double )), sizeof( double ) );
	}
}

void BufferWriter::AppendStringZeroTerminated( std::string const& value )
{
	for (int i = 0; i < value.length(); i++)
		AppendChar( value[i] );
	AppendChar( 0 );
}

void BufferWriter::AppendStringAfter32BitLength( std::string const& value )
{
	AppendUInt32( (unsigned long int)value.length() );
	for (int i = 0; i < value.length(); i++)
		AppendChar( value[i] );
}

void BufferWriter::AppendRgba( Rgba8 const& value )
{
	AppendByte( value.r );
	AppendByte( value.g );
	AppendByte( value.b );
	AppendByte( value.a );
}

void BufferWriter::AppendRgb( Rgba8 const& value )
{
	AppendByte( value.r );
	AppendByte( value.g );
	AppendByte( value.b );
	AppendByte( (unsigned char)255 );
}

void BufferWriter::AppendIntVec2( IntVec2 const& value )
{
	AppendInt( value.x );
	AppendInt( value.y );
}

void BufferWriter::AppendVec2( Vec2 const& value )
{
	AppendFloat( value.x );
	AppendFloat( value.y );
}

void BufferWriter::AppendAABB2( AABB2 const& value )
{
	AppendVec2( value.m_mins );
	AppendVec2( value.m_maxs );
}

void BufferWriter::AppendVertexPCU( Vertex_PCU const& value )
{
	AppendFloat( value.m_position.x );
	AppendFloat( value.m_position.y );
	AppendFloat( value.m_position.z );
	AppendRgba( value.m_color );
	AppendVec2( value.m_uvTexCoords );
}

void BufferWriter::SetEndianMode( Endianness mode )
{
	m_endianness = mode;
}

void BufferWriter::ReverseByte( unsigned char* startPos, unsigned long int length )
{
	for (size_t i = 0; i < length / 2; ++i) 
	{
		std::swap( startPos[i], startPos[length - 1 - i] );
	}
}

BufferParser::BufferParser( std::vector<unsigned char>& buffer )
{
	m_bufferStart = buffer.data();
	m_currentPos = 0;
}

char BufferParser::ParseChar()
{
	char character = static_cast<char>(m_bufferStart[m_currentPos]);
	m_currentPos += 1;
	return character;
}

unsigned char BufferParser::ParseByte()
{
	unsigned char character = static_cast<unsigned char>(m_bufferStart[m_currentPos]);
	m_currentPos += 1;
	return character;
}

bool BufferParser::ParseBool()
{
	unsigned char chara = ParseByte();
	if (chara == 1)
		return true;
	else if (chara == 0)
		return false;
	ERROR_AND_DIE( "NOT A BOOLEAN" );
}

short BufferParser::ParseShort()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 2 );
	}
	short value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 2 );
	m_currentPos += 2;
	return value;
}

unsigned short BufferParser::ParseUShort()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 2 );
	}
	unsigned short value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 2 );
	m_currentPos += 2;
	return value;
}

int BufferParser::ParseInt()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 4 );
	}
	int value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 4 );
	m_currentPos += 4;
	return value;
}

unsigned int BufferParser::ParseUInt()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 4 );
	}
	unsigned int value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 4 );
	m_currentPos += 4;
	return value;
}

long int BufferParser::ParseInt32()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 4 );
	}
	long int value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 4 );
	m_currentPos += 4;
	return value;
}

unsigned long int BufferParser::ParseUInt32()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 4 );
	}
	unsigned long int value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 4 );
	m_currentPos += 4;
	return value;
}

long long int BufferParser::ParseInt64()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 8 );
	}
	long long int value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 8 );
	m_currentPos += 8;
	return value;
}

unsigned long long int BufferParser::ParseUInt64()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 8 );
	}
	unsigned long long int value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 8 );
	m_currentPos += 8;
	return value;
}

float BufferParser::ParseFloat()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 4 );
	}
	float value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 4 );
	m_currentPos += 4;
	return value;
}

double BufferParser::ParseDouble()
{
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, 8 );
	}
	double value;
	std::memcpy( &value, m_bufferStart + m_currentPos, 8 );
	m_currentPos += 8;
	return value;
}

std::string BufferParser::ParseStringZeroTerminated()
{
	std::string value;
	while (true)
	{
		char chara = ParseChar();
		if (chara == 0)
			break;
		value += chara;
	}
	if (m_endianness != m_nativeEndianness)
	{
		std::reverse( value.begin(), value.end() );
	}
	return value;
}

std::string BufferParser::ParseStringAfter32BitLength()
{
	unsigned long int length = ParseUInt32();
	if (m_endianness != m_nativeEndianness)
	{
		ReverseByte( m_bufferStart + m_currentPos, length );
	}
	const char* value;
	std::memcpy( &value, m_bufferStart + m_currentPos, length );
	return value;
}

Rgba8 BufferParser::ParseRgba()
{
	unsigned char r = ParseByte();
	unsigned char g = ParseByte();
	unsigned char b = ParseByte();
	unsigned char a = ParseByte();
	return Rgba8( r, g, b, a );
}

Rgba8 BufferParser::ParseRgb()
{
	unsigned char r = ParseByte();
	unsigned char g = ParseByte();
	unsigned char b = ParseByte();
	return Rgba8( r, g, b, 255 );
}

IntVec2 BufferParser::ParseIntVec2()
{
	int a = ParseInt();
	int b = ParseInt();
	return IntVec2( a, b );
}

Vec2 BufferParser::ParseVec2()
{
	float a = ParseFloat();
	float b = ParseFloat();
	return Vec2( a, b );
}

AABB2 BufferParser::ParseAABB2()
{
	Vec2 a = ParseVec2();
	Vec2 b = ParseVec2();
	return AABB2( a, b );
}

Vertex_PCU BufferParser::ParseVertexPCU()
{
	float a = ParseFloat();
	float b = ParseFloat();
	float c = ParseFloat();
	Rgba8 rgba = ParseRgba();
	Vec2 vec2 = ParseVec2();
	return Vertex_PCU( Vec3( a, b, c ), rgba, vec2 );
}

void BufferParser::SetEndianMode( Endianness mode )
{
	m_endianness = mode;
}

void BufferParser::ReverseByte( unsigned char* startPos, unsigned long int length )
{
	for (size_t i = 0; i < length / 2; ++i) 
	{
		std::swap( startPos[i], startPos[length - 1 - i] );
	}
}

Endianness BufferParser::GetEndianMode() const
{
	return m_endianness;
}

bool LoadBinaryFileToExistingBuffer( std::string const& filePath, std::vector<unsigned char>& buffer ) 
{
	std::ifstream file( filePath, std::ios::binary | std::ios::ate );
	if (!file.is_open()) 
	{
		ERROR_AND_DIE( Stringf( "Can not open file %s", filePath.c_str() ).c_str() );
		return false;
	}

	std::streamsize fileSize = file.tellg();
	file.seekg( 0, std::ios::beg );
	buffer.resize( fileSize );

	if (!file.read( reinterpret_cast<char*>(buffer.data()), fileSize ))
	{
		ERROR_AND_DIE( Stringf( "Can not read file %s", filePath.c_str() ).c_str() );
		return false;
	}

	file.close();
	return true;
}

bool SaveBinaryFileFromBuffer( std::string const& filePath, std::vector<unsigned char>& buffer )
{
	std::ofstream file( filePath, std::ios::binary );
	if (!file.is_open())
	{
		ERROR_AND_DIE( Stringf( "Cannot open file %s for writing", filePath.c_str() ).c_str() );
		return false;
	}

	file.write( reinterpret_cast<const char*>(buffer.data()), buffer.size() );

	if (!file)
	{
		ERROR_AND_DIE( Stringf( "Error writing to file %s", filePath.c_str() ).c_str() );
		return false;
	}

	file.close();
	return true;
}
