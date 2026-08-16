#pragma once

#include <vector>
#include <string>

bool FileReadToBuffer( std::vector<uint8_t>& outBuffer, std::string const& fileName );
bool FileReadToString( std::string& outString, std::string const& fileName );
bool FileWriteToBuffer( std::vector<uint8_t> const& buffer, std::string const& filePath );
bool FileWriteToBuffer_S( std::vector<uint8_t> const& buffer, std::string const& filePath );