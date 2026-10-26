#ifndef ANIMATION_TOOLS_PATTERNLIST_H
#define ANIMATION_TOOLS_PATTERNLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

namespace AnimationTools
{
	/**
	A list of string patterns that can use "*" wildcards.
	*/
	class C_PatternList
	{
	public:
	
		/// Clear pattern list.
		void Clear();
	
		/// Add a pattern to the pattern list. Returns false if pattern is invalid.
		bool Add( const Util::C_String& pattern );

		void Remove( const Util::C_String& pattern );

		bool Contains( const Util::C_String& pattern ) const;

		/// Returns true if @a str matches any one of the patterns in the list.
		bool Match( const Util::C_String& str ) const;

		
		bool operator == ( const C_PatternList& rhs ) const;
		bool operator != ( const C_PatternList& rhs ) const;
	
	private:
		bool IsValidPattern( const Util::C_String& pattern) const;
		bool MatchesPattern( const Util::C_String& str, const Util::C_String& pattern ) const;
		S32 FindPattern( const Util::C_String& pattern ) const;
		Btl::Vector< Util::C_String > m_patterns;
	};
	
	

}

#endif