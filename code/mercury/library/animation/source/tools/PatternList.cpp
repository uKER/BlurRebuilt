#include "stdafx.h"
#include <animation/tools/PatternList.h>
#include <animation/common/StringUtil.h>

namespace AnimationTools
{

//
// C_PatternList
//

void C_PatternList::Clear()
{
	m_patterns.Clear();
}

bool C_PatternList::Add( const Util::C_String& pattern )
{
	if ( !IsValidPattern( pattern ) )
	{
		return false;
	}
	
	if ( pattern.Length() == 0 )
	{
		return false;
	}

	S32 index = FindPattern(pattern);
	if (index >= 0)
	{
		// Already have a matching pattern.
		return true;			
	}

	// Add new pattern.
	m_patterns.PushBack( pattern.Lower() );
	
	return true;	
}

void C_PatternList::Remove( const Util::C_String& pattern )
{
	Util::C_String lowerPattern = pattern.Lower();
		
	S32 index = FindPattern(pattern);
	if (index >= 0)
	{
		m_patterns.EraseByIndex(index);
	}
}

bool C_PatternList::Contains( const Util::C_String& pattern ) const
{
	return FindPattern(pattern) >= 0;
}

bool C_PatternList::Match( const Util::C_String& str ) const
{
	for ( S32 i = 0; i < m_patterns.Size(); ++i )
	{
		if ( MatchesPattern( str, m_patterns[ i ] ) )
		{
			return true;
		}
	}
	return false;
}

bool C_PatternList::operator == ( const C_PatternList& rhs ) const
{
	return !(*this != rhs);
}

bool C_PatternList::operator != ( const C_PatternList& rhs ) const
{
	if ( m_patterns.Size() != rhs.m_patterns.Size() )
	{
		return true;
	}
	
	for ( S32 i = 0; i < m_patterns.Size(); ++i )
	{
		if ( m_patterns[i] != rhs.m_patterns[i] )
		{
			return true;
		}
	}
	return false;
}

bool C_PatternList::IsValidPattern( const Util::C_String& pattern ) const
{
	return true;
}

bool C_PatternList::MatchesPattern( const Util::C_String& str, const Util::C_String& pattern ) const
{
	return Animation::StringUtil::MatchAnyCase(str.Chars(), pattern.Chars());
}

S32 C_PatternList::FindPattern( const Util::C_String& pattern ) const
{
	Util::C_String lowerPattern = pattern.Lower();
	S32 index = -1;
	for ( S32 i = 0; i < m_patterns.Size(); ++i )
	{
		if ( lowerPattern == m_patterns[ i ] )
		{
			index = i;
			break;
		}
	}
	return index;
}

} // end AnimationTools
