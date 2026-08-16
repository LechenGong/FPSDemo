#pragma once

#include <vector>
#include <string>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

enum class Endianness
{
	NONE,
	LITTLE,
	BIG,
	TOTAL
};

class BufferWriter
{
public:
	void AppendChar( char value );
	void AppendByte( unsigned char value );
	void AppendBool( bool value );
	void AppendShort( short value );
	void AppendUShort( unsigned short value );
	void AppendInt( int value );
	void AppendUInt( unsigned int value );
	void AppendInt32( long int value );
	void AppendUInt32( unsigned long int value );
	void AppendInt64( long long int value );
	void AppendUInt64( unsigned long long int value );
	void AppendFloat( float value );
	void AppendDouble( double value );

	void AppendStringZeroTerminated( std::string const& value );
	void AppendStringAfter32BitLength( std::string const& value );
	void AppendRgba( Rgba8 const& value );
	void AppendRgb( Rgba8 const& value );
	void AppendIntVec2( IntVec2 const& value );
	void AppendVec2( Vec2 const& value );
	void AppendAABB2( AABB2 const& value );
	void AppendVertexPCU( Vertex_PCU const& value );

	void SetEndianMode( Endianness mode );
	void ReverseByte( unsigned char* startPos, unsigned long int length );
public:
	std::vector<unsigned char> m_buffer;
	Endianness m_endianness;
};

class BufferParser
{
public:
	BufferParser( std::vector<unsigned char>& buffer );

	char ParseChar();
	unsigned char ParseByte();
	bool ParseBool();
	short ParseShort();
	unsigned short ParseUShort();
	int ParseInt();
	unsigned int ParseUInt();
	long int ParseInt32();
	unsigned long int ParseUInt32();
	long long int ParseInt64();
	unsigned long long int ParseUInt64();
	float ParseFloat();
	double ParseDouble();

	std::string ParseStringZeroTerminated();
	std::string ParseStringAfter32BitLength();
	Rgba8 ParseRgba();
	Rgba8 ParseRgb();
	IntVec2 ParseIntVec2();
	Vec2 ParseVec2();
	AABB2 ParseAABB2();
	Vertex_PCU ParseVertexPCU();

	void SetEndianMode( Endianness mode );
	void ReverseByte( unsigned char* startPos, unsigned long int length );

	Endianness GetEndianMode() const;

private:
	//std::vector<unsigned char> m_data;
	Endianness m_endianness = Endianness::NONE;
	Endianness const m_nativeEndianness = Endianness::LITTLE;
	unsigned char* m_bufferStart = nullptr;
	unsigned long int m_currentPos = 0;
};
bool LoadBinaryFileToExistingBuffer( std::string const& filePath, std::vector<unsigned char>& buffer );

bool SaveBinaryFileFromBuffer( std::string const& filePath, std::vector<unsigned char>& buffer );