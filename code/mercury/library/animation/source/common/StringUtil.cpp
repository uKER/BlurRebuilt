#include "stdafx.h"
#include <animation/common/StringUtil.h>

namespace Animation
{
namespace StringUtil
{

inline bool IsWildcard(CHAR8 ch)
{
	return ch == '?' || ch == '*';
}

bool MatchWildcard(const CHAR8*& str, const CHAR8*& pattern)
{
	// Skip duplicate * characters.
	while (IsWildcard(*pattern))
	{
		++pattern;
	}

	bool searching = true;
	do
	{
		// Skip over characters in string until we reach a potential match for the remaining pattern.
		while (*str != *pattern && *str != 0)
		{
			++str;
		}
		
		if (*str == *pattern)	// *str == *pattern i.e. 
		{
			if (*pattern != 0)
			{
				if (Match(str, pattern))
				{
					// Found a match!
					return true;
				}
			}
			else
			{
				// Reached end of pattern - string has matched.
				return true;
			}
			++str;
		}
		else
		{
			// Reached end of string without finding next part of pattern - no match.
			searching = false;
		}
	} while (searching);
	
	return false;
}

bool Match(const CHAR8* str, const CHAR8* pattern)
{
	bool match = true;
	
	while (*str != 0 && *pattern != 0 && match)
	{
		if (IsWildcard(*pattern))
		{
			match = MatchWildcard(str, pattern);
			
			if (match)
			{
				return true;
			}
		}
		else
		{
			if (*str != *pattern)
			{
				match = false;
			}
			++str;
			++pattern;
		}
	}
	
	if (*str == 0)
	{
		while (IsWildcard(*pattern))
		{
			++pattern;
		}
	}
	
	return match && *pattern == 0 && *str == 0;
}

bool MatchWildcardAnyCase(const CHAR8*& str, const CHAR8*& pattern)
{
	// Skip duplicate * characters.
	while (IsWildcard(*pattern))
	{
		++pattern;
	}

	bool searching = true;
	do
	{
		// Skip over characters in string until we reach a potential match for the remaining pattern.
		while (ToLower(*str) != ToLower(*pattern) && *str != 0)
		{
			++str;
		}
		
		if (ToLower(*str) == ToLower(*pattern))	// *str == *pattern i.e. 
		{
			if (*pattern != 0)
			{
				if (MatchAnyCase(str, pattern))
				{
					// Found a match!
					return true;
				}
			}
			else
			{
				// Reached end of pattern - string has matched.
				return true;
			}
			++str;
		}
		else
		{
			// Reached end of string without finding next part of pattern - no match.
			searching = false;
		}
	} while (searching);
	
	return false;
}

bool MatchAnyCase(const CHAR8* str, const CHAR8* pattern)
{
	bool match = true;
	
	while (*str != 0 && *pattern != 0 && match)
	{
		if (IsWildcard(*pattern))
		{
			match = MatchWildcardAnyCase(str, pattern);
			
			if (match)
			{
				return true;
			}
		}
		else
		{
			if (ToLower(*str) != ToLower(*pattern))
			{
				match = false;
			}
			++str;
			++pattern;
		}
	}
	
	if (*str == 0)
	{
		while (IsWildcard(*pattern))
		{
			++pattern;
		}
	}
	
	return match && *pattern == 0 && *str == 0;
}


} // StringUtil
} // Animation

