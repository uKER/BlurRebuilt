#ifndef ANIMATION_STRINGID_INL
#define ANIMATION_STRINGID_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <core/containers/String.h>

namespace Animation
{
	inline C_StringId::C_StringId():
		m_stringIndex( 0 )
	{
	}
	
	inline void C_StringId::Clear()
	{
		m_stringIndex = 0;
	}
	
	inline bool C_StringId::IsNull() const
	{
		return m_stringIndex == 0;
	}
	
	inline bool C_StringId::IsValid() const
	{
		return m_stringIndex != 0;
	}
	
	inline C_StringId::operator bool () const
	{
		return m_stringIndex != 0;
	}
	
	/*
	This operator makes it possible to test for null strings as follows:
	@code
	if (!stringid)
	{
		// handle null string case...
	}
	else
	{
		// handle valid string case...
	}
	@endcode
	
	The ! operator has an advantage over "operator bool" as it doesn't
	allow a C_StringId to be implicitly converted to an integer.
	*/
	inline bool C_StringId::operator !() const
	{
		return m_stringIndex == 0;
	}
	
	inline bool C_StringId::operator == ( const C_StringId& other ) const
	{
		return m_stringIndex == other.m_stringIndex;
	}
	
	inline bool C_StringId::operator != ( const C_StringId& other ) const
	{
		return m_stringIndex != other.m_stringIndex;
	}

	inline bool C_StringId::operator < ( const C_StringId& other ) const
	{
		return m_stringIndex < other.m_stringIndex;
	
	}


#ifndef PLATFORM_SPU
	template < typename T >
	inline T& C_StringIdArray::Deserialise( T& in )
	{
		delete[] m_stringIds;
		m_stringIds = 0;
		m_numStrings = 0;

		in >> m_numStrings;
		
		if ( m_numStrings > 0 )
		{
			m_stringIds = new C_StringId[ m_numStrings ];
		
			CHAR8 buffer[256];
		
			for ( U32 i = 0, count = m_numStrings; i < count; ++i )
			{
				U32 numChars;
				in >> numChars;
				if (numChars > 0)
				{
					BIZ_ASSERT(numChars < sizeof(buffer)/sizeof(buffer[0]));
					in.Read(buffer, numChars);
				}
				buffer[numChars] = 0;
				m_stringIds[ i ] = C_StringId( buffer );
			}
		}
		return in;
	}
	

	template < typename T >
	inline T& C_StringIdArray::Serialise( T& out ) const
	{
		out << m_numStrings;
		
		for ( U32 i = 0, count = m_numStrings; i < count; ++i )
		{
			Util::C_String tmp( m_stringIds[ i ].GetString() );
			out << tmp;
		}
		return out;
	}
#endif	
	
}

#endif

