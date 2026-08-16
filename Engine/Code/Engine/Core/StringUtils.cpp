#include "StringUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings Split( std::string const& str, std::string const& delimiter, bool skip_empty/* = false*/, int maxOperations/* = 999999*/ )
{
	Strings elems;
	std::string::size_type pos = 0, prev = 0;
	while ((pos = str.find_first_of( delimiter, prev )) != std::string::npos && maxOperations)
	{
		if (pos > prev)
		{
			elems.emplace_back( str, prev, pos - prev );
		}
		else if (!skip_empty && pos == prev)
		{
			elems.emplace_back( "" );
		}
		prev = pos + 1;
		maxOperations--;
	}

	if (prev < str.size())
	{
		elems.emplace_back( str, prev, str.size() - prev );
	}
	return elems;
}

Strings Split( std::string const& str, char delimiter, bool skip_empty/* = false*/, int maxOperations/* = 999999*/ )
{
	return Split( str, std::string( 1, delimiter ), skip_empty, maxOperations );
}

Strings SplitByLength( std::string const& str, int maxLength )
{
	Strings result;
	for (std::size_t i = 0; i < str.length(); i += maxLength) 
	{
		result.push_back( str.substr( i, maxLength ) );
	}
	return result;
}

Strings SplitWithQuotation( std::string const& str, char delimiter, bool skip_empty/* = false*/, bool remove_quotation/* = false*/, int maxOperations/* = 999999*/ )
{
	Strings elems;
	std::string token;

	bool inQuote = false;
	bool escapeNext = false;
	int splitCounter = 0;

	for (int i = 0; i < str.length(); i++)
	{
		char chara = str[i];

		if (escapeNext)
		{
			if (remove_quotation)
			{
				if (chara == '"' || chara == '\\')
				{
					token += chara;
				}
				else
				{
					token += '\\';
					token += chara;
				}
			}
			else
			{
				token += '\\';
				token += chara;
			}
			escapeNext = false;
			continue;
		}

		if (chara == '\\')
		{
			escapeNext = true;
			continue;
		}

		if (chara == '"')
		{
			inQuote = !inQuote;
			if (!remove_quotation)
			{
				token += '"';
			}
			continue;
		}

		if (chara == delimiter && !inQuote && splitCounter < maxOperations)
		{
			if (!(token.empty() && skip_empty))
			{
				elems.push_back( token );
				token.clear();
				++splitCounter;
			}
		}
		else
		{
			token += chara;
		}
	}

	if (!(token.empty() && skip_empty))
	{
		elems.push_back( token );
	}

	return elems;
}


Strings SplitWithQuotation( std::string const& str, std::string const& delimiter, bool skip_empty/* = false*/, bool remove_quotation/* = false*/, int maxOperations/* = 999999*/ )
{
	Strings elems;
	std::string token;

	bool inQuote = false;
	bool escapeNext = false;
	int splitCounter = 0;

	size_t i = 0;

	while (i < str.length())
	{
		char chara = str[i];

		if (escapeNext)
		{
			if (remove_quotation)
			{
				if (chara == '"' || chara == '\\')
				{
					token += chara;
				}
				else
				{
					token += '\\'; // Preserve unknown escape
					token += chara;
				}
			}
			else
			{
				token += '\\';
				token += chara;
			}
			escapeNext = false;
			i++;
			continue;
		}

		if (chara == '\\')
		{
			escapeNext = true;
			i++;
			continue;
		}

		if (chara == '"')
		{
			inQuote = !inQuote;
			if (!remove_quotation)
				token += '"';
			i++;
			continue;
		}

		if (!inQuote && delimiter.length() > 0 &&
			str.compare( i, delimiter.length(), delimiter ) == 0 &&
			splitCounter < maxOperations)
		{
			if (!(token.empty() && skip_empty))
			{
				elems.push_back( token );
				token.clear();
				splitCounter++;
			}
			i += delimiter.length();
			continue;
		}

		token += chara;
		i++;
	}

	if (!(token.empty() && skip_empty))
	{
		elems.push_back( token );
	}

	return elems;
}

std::string ToLower( std::string const& str )
{
	std::string lowerStr;
	for (char c : str)
	{
		lowerStr += (char)std::tolower( static_cast<unsigned char>(c) );
	}
	return lowerStr;
}

std::string ToUpper( std::string const& str )
{
	std::string upperStr;
	for (char c : str)
	{
		upperStr += (char)std::toupper( static_cast<unsigned char>(c) );
	}
	return upperStr;
}

std::string RemoveEndingSpace( std::string const& str )
{
	std::size_t endPos = str.find_last_not_of( ' ' );

	if (endPos != std::string::npos) 
	{
		return str.substr( 0, endPos + 1 );
	}
	return "";
}

std::string RemoveChar( std::string const& str, char const& charToRemove )
{
	std::string tempString;
	std::string newString;
	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] == charToRemove)
		{
			newString += tempString;
			tempString.clear();
		}
		else
		{
			tempString.push_back( str[i] );
		}
	}
	newString += tempString;
	return newString;
}

Value TryConvert( std::string const& str )
{
	try {
		size_t pos;
		int intValue = std::stoi( str, &pos );
		if (pos == str.length()) 
		{
			return intValue;
		}
	}
	catch (const std::invalid_argument&) {}
	catch (const std::out_of_range&) {}

	try {
		size_t pos;
		float floatValue = std::stof( str, &pos );
		if (pos == str.length()) 
		{
			return floatValue;
		}
	}
	catch (const std::invalid_argument&) {}
	catch (const std::out_of_range&) {}

	return str.c_str();
}
