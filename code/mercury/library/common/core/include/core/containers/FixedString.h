#ifndef FIXEDSTRING_H
#define FIXEDSTRING_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{

template < int m_Size >
class C_FixedString
{

public:
    
    C_FixedString()
    {
        memset( m_Chars, 0, m_Size );
    }
    
    C_FixedString( const CHAR8* src )
    {
        strncpy_s( m_Chars, m_Size, src, m_Size - 1 );
        m_Chars[ m_Size - 1 ] = 0;
    }
    
    void operator = ( const CHAR8* src )
    {
        strncpy_s( m_Chars, m_Size, src, m_Size - 1 );
        m_Chars[ m_Size - 1 ] = 0;
    }

    /// Concatenation operator
    const C_FixedString& operator += ( const CHAR8* src )
    {
		strncat_s(m_Chars, m_Size, src, m_Size - strlen(m_Chars) - 1);
		return *this;
	}
    
    operator const CHAR8* () const
    {
        return m_Chars;
    }

	FORCE_INLINE CHAR8* NonConstChars() { return m_Chars; }

    template < int Size >
    const bool operator == ( const C_FixedString<Size>& src ) const
    {
        return strcmp( m_Chars, src.m_Chars ) == 0;
    }
    
    const bool operator == ( const CHAR8* src ) const
    {
        return strcmp( m_Chars, src ) == 0;
    }
    
    template < int Size >
    const bool operator != ( const C_FixedString<Size>& src ) const
    {
        return strcmp( m_Chars, src.m_Chars ) != 0;
    }
    
    const bool operator != ( const CHAR8* src ) const
    {
        return strcmp( m_Chars, src ) != 0;
    }
    
    const CHAR8*       Chars() const   { return m_Chars; }
    const S32       Size() const    { return m_Size; }
    const S32       Length() const  { return (S32)strlen( m_Chars ); }


    const CHAR8& operator[] (S32 index) const
    {
		BIZ_ASSERT(index >= 0 && index < m_Size);
        return m_Chars[index];
    }

    CHAR8& operator[] (S32 index)
    {
		BIZ_ASSERT(index >= 0 && index < m_Size);
        return m_Chars[index];
    }

    template < class T_Stream >
    void Deserialise( T_Stream& stream )
    {
        stream.Read( m_Chars, m_Size );
    }

    template < class T_Stream >
    void Serialise( T_Stream& stream )
    {
        stream.Write( m_Chars, m_Size );
    }

	 /// Var-arg initialisation
    const C_FixedString<m_Size>& Printf( char* format, ... )
    {
	    char buffer[ 1024 ];
        
	    va_list arglist;
	    va_start( arglist, format );
	    vsprintf_s( buffer, 1024, format, arglist );
	    va_end( arglist );

		strncpy_s(m_Chars, m_Size, buffer, m_Size-1);
	    
	    return *this;
    }

	S32 RevSearch( const char c ) const
	{
		const char* ptr = strrchr( Chars(), c );
		return ptr-m_Chars;
	}

    /// Sub-string extraction operator.
    /// NOTE: lo = inclusive, hi = exclusive, range ( 0, 0 ) denotes an empty string!
    C_FixedString operator () ( S32 lo, S32 hi ) const
    {
		hi = hi > m_Size ? m_Size : hi;
		lo = lo > hi ? hi : lo; 
        const S32 size = hi - lo;

		C_FixedString ret;

		CHAR8* pDst = &ret.m_Chars[0];
		const CHAR8* pSrc = &m_Chars[lo];
		memcpy(pDst, pSrc, sizeof(CHAR8)*size);
        ret.m_Chars[ size ] = 0;

        return ret;
    }
	
	S32 Search( const char *str )
	{
		const CHAR8* ptr = strstr( m_Chars, str );
		if (!ptr)
		{
			return 0;
		}
		else
		{
			return ptr - m_Chars;
		}
	}

private:
    
    CHAR8              m_Chars[ m_Size ];
};

typedef C_FixedString<4>      C_String4;
typedef C_FixedString<6>      C_String6;
typedef C_FixedString<8>      C_String8;
typedef C_FixedString<12>     C_String12;
typedef C_FixedString<16>     C_String16;
typedef C_FixedString<32>     C_String32;
typedef C_FixedString<64>     C_String64;
typedef C_FixedString<80>     C_String80;
typedef C_FixedString<96>     C_String96;
typedef C_FixedString<128>    C_String128;
typedef C_FixedString<256>    C_String256;



}; // namespace Util






#endif
