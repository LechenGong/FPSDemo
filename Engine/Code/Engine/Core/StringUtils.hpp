#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>

using Value = std::variant<int, float, const char*>;

typedef std::vector<std::string> Strings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

Strings Split( std::string const& str, std::string const& delimiter, bool skip_empty = false, int maxOperations = 999999 );
Strings Split( std::string const& str, char delimiter, bool skip_empty = false, int maxOperations = 999999 );

Strings SplitByLength( std::string const& str, int maxLength );

Strings SplitWithQuotation( std::string const& str, char delimiter, bool skip_empty = false, bool remove_quotation = false, int maxOperations = 999999 );
Strings SplitWithQuotation( std::string const& str, std::string const& delimiter, bool skip_empty= false, bool remove_quotation = false, int maxOperations = 999999 );

std::string ToLower( std::string const& str );
std::string ToUpper( std::string const& str );

std::string RemoveEndingSpace( std::string const& str );

std::string RemoveChar( std::string const& str, char const& charToRemove );

Value TryConvert( std::string const& str );