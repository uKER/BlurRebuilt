
#ifndef UI_WIDESTRING_INL
#error WideString.inl should only be included from WideString.h
#endif // UI_WIDESTRING_INL

namespace UI
{
/*---------------------------------------------------------------------------------------
	Constructors
---------------------------------------------------------------------------------------*/
template < U32 m_Size >
C_WideString<m_Size>::C_WideString()
{
    memset( m_Chars, 0, m_Size );
}


template < U32 m_Size >
C_WideString<m_Size>::C_WideString( const WCHAR* str )
{
	U32 len = (U32)wcslen( str );
	if ( len > m_Size - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	wcsncpy( m_Chars, str, m_Size );	// Doesnt copy or add the end of string marker so
	m_Chars[ ( len > m_Size - 1 ) ? m_Size - 1 : len ] = '\0';	// need to add it ourselfs.
#else // Xbox and PC are the same.
    wcsncpy_s( m_Chars, m_Size, str, _TRUNCATE );
#endif
}


template < U32 m_Size >
C_WideString<m_Size>::C_WideString( const char* str )
{
	U32 len = (U32)strlen( str );
	if ( len > m_Size - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	mbsrtowcs( m_Chars, &str, m_Size, NULL );	// Doesnt copy or add the end of string marker so
	m_Chars[ ( len > m_Size - 1 ) ? m_Size - 1 : len ] = '\0';			// need to add it ourselfs.
#else // Xbox and PC are the same.
	size_t convertedChars = 0;	// [Blur2PC][x64]
	mbstowcs_s( &convertedChars, m_Chars, m_Size, str, _TRUNCATE );
#endif
}


template < U32 m_Size >
C_WideString<m_Size>::C_WideString( C_TextStringID str )
{
	U32 len = (U32)wcslen( str.String() );
	if ( len > m_Size - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	wcsncpy( m_Chars, str.String(), m_Size );	// Doesnt copy or add the end of string marker so
	m_Chars[ ( len > m_Size - 1 ) ? m_Size - 1 : len ] = '\0';			// need to add it ourselfs.
#else // Xbox and PC are the same.
    wcsncpy_s( m_Chars, m_Size, str.String(), _TRUNCATE );
#endif
}


template < U32 m_Size >
C_WideString<m_Size>::C_WideString( const WCHAR c )
{
    memset( m_Chars, 0, m_Size );
	Add( c );
}


/*---------------------------------------------------------------------------------------
	Copy constructor
---------------------------------------------------------------------------------------*/
template < U32 m_Size >
C_WideString<m_Size>::C_WideString( const I_WideString& str )
{
	U32 len = str.Length();
	if ( len > m_Size - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	wcsncpy( m_Chars, str.Chars(), m_Size );	// Doesnt copy or add the end of string marker so
	m_Chars[ ( len > m_Size - 1 ) ? m_Size - 1 : len ] = '\0';			// need to add it ourselfs.
#else // Xbox and PC are the same.
    wcsncpy_s( m_Chars, m_Size, str.Chars(), _TRUNCATE );
#endif
}


/*---------------------------------------------------------------------------------------
	Operator overloads.
---------------------------------------------------------------------------------------*/
template < U32 m_Size >
C_WideString<m_Size>::operator const WCHAR* () const
{
    return m_Chars;
}

template < U32 m_Size >
const C_WideString<m_Size>& C_WideString<m_Size>::operator += ( const I_WideString& str )
{
	Add( str );

	return *this;
}

template < U32 m_Size >
const C_WideString<m_Size>& C_WideString<m_Size>::operator += ( const WCHAR* str )
{
	Add( str );

	return *this;
}

template < U32 m_Size >
const C_WideString<m_Size>& C_WideString<m_Size>::operator += ( const WCHAR c )
{
	Add( c );

	return *this;
}

template < U32 m_Size >
const C_WideString<m_Size> C_WideString<m_Size>::operator + ( const I_WideString& str )
{
	// Make tmp copy.
	C_WideString<m_Size> tmp( m_Chars );

	U32 len = Length();
	U32 sLen = str.Length();

	// Combine.
	if ( len + sLen > tmp.Size() - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	U32 count = ( sLen > ( m_Size - 1 ) - len ) ? ( m_Size - 1 ) - len : sLen;
	wcsncat( tmp.NonConstChars(), str.Chars(), count );
	tmp.NonConstChars()[ len + count ] = '\0';

#else // Xbox and PC are the same.
	wcsncat_s( tmp.NonConstChars(), tmp.Size(), str.Chars(), _TRUNCATE );
#endif	
	return tmp;
}


template < U32 m_Size >
const C_WideString<m_Size> C_WideString<m_Size>::operator + ( const WCHAR* str )
{
	// Make tmp copy.
	C_WideString<m_Size> tmp( m_Chars );

	U32 len = Length();
	U32 sLen = (U32)wcslen(str);

	// Combine.
	if ( len + sLen > tmp.Size() - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	U32 count = ( sLen > ( m_Size - 1 ) - len ) ? ( m_Size - 1 ) - len : sLen;
	wcsncat( tmp.NonConstChars(), str, count );
	tmp.NonConstChars()[ len + count ] = '\0';

#else // Xbox and PC are the same.
	wcsncat_s( tmp.NonConstChars(), tmp.Size(), str, _TRUNCATE );
#endif
	return tmp;
}


template < U32 m_Size >
bool C_WideString<m_Size>::operator == ( const I_WideString& str ) const
{
	return Compare( str );
}


template < U32 m_Size >
bool C_WideString<m_Size>::operator != ( const I_WideString& str ) const
{
	return !Compare( str );
}


template < U32 m_Size >
const WCHAR C_WideString<m_Size>::operator[] ( const S32 index ) const
{
	assert( index >= 0 && index < m_Size );
    return m_Chars[index];
}


template < U32 m_Size >
WCHAR C_WideString<m_Size>::operator[] ( const S32 index )
{
	assert( index >= 0 && index < m_Size );
    return m_Chars[index];
}


/*---------------------------------------------------------------------------------------
	Functions
---------------------------------------------------------------------------------------*/
template < U32 m_Size >
void C_WideString<m_Size>::Serialise( Sys::BinarySerialiserAutoEndian& serial )
{
	serial.Write( m_Chars, m_Size * sizeof( WCHAR ) );
}

template < U32 m_Size >
void C_WideString<m_Size>::Deserialise( Sys::BinaryDeserialiserAutoEndian& serial )
{
	serial.Read( m_Chars, m_Size * sizeof( WCHAR ) );
}


template < U32 m_Size >
Util::C_String C_WideString<m_Size>::AsString() const
{
	char buff[m_Size];
	wcstombs( buff, m_Chars, Length() );
	buff[ Length() ] = '\0';
	return buff;
}

template < U32 m_Size >
void C_WideString<m_Size>::Clear()
{
    memset( m_Chars, 0, m_Size );
}


template < U32 m_Size >
void C_WideString<m_Size>::Copy( const WCHAR* str )
{
#ifdef PLATFORM_PS3
	wcsncpy( m_Chars, str, m_Size );
	m_Chars[ m_Size - 1 ] = '\0';
#else // Xbox and PC are the same.
	wcsncpy_s( m_Chars, m_Size, str, _TRUNCATE );
#endif
}


template < U32 m_Size >
void C_WideString<m_Size>::Format( const WCHAR* format, ... )
{
	va_list va_args;
	va_start( va_args, format );

#ifdef PLATFORM_PS3
	static WCHAR formatted_string[1024];
	vswprintf( m_Chars, m_Size, Ps3_Reformat( formatted_string, lengthof(formatted_string), format ), va_args );
#else
	vswprintf( m_Chars, m_Size, format, va_args );
#endif

	va_end( va_args );
}


template < U32 m_Size >
bool C_WideString<m_Size>::Compare( const I_WideString& str ) const
{
	U32 len = Length();

	// Same length?
	if ( str.Length() != len )
		return false;

	// Check all characters match.
	for ( U32 i = 0; i < len; i++ )
	{
		if ( str[i] != m_Chars[i] )
			return false;
	}
	return true;
}

template < U32 m_Size >
bool C_WideString<m_Size>::Compare( const WCHAR* str ) const
{
	U32 len = Length();

	// Same length?
	if ( wcslen( str ) != len )
		return false;

	// Check all characters match.
	for ( U32 i = 0; i < len; i++ )
	{
		if ( str[i] != m_Chars[i] )
			return false;
	}
	return true;
}


template < U32 m_Size >
void C_WideString<m_Size>::Add( const I_WideString& str )
{
	U32 len = Length();
	U32 sLen = str.Length();

	if ( len + sLen > m_Size - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	U32 count = ( sLen > ( m_Size - 1 ) - len ) ? ( m_Size - 1 ) - len : sLen;
	wcsncat( m_Chars, str.Chars(), count );
	m_Chars[ len + count ] = '\0';

#else // Xbox and PC are the same.
	wcsncat_s( m_Chars, m_Size, str.Chars(), _TRUNCATE );
#endif
}


template < U32 m_Size >
void C_WideString<m_Size>::Add( const WCHAR* str )
{
	U32 len = Length();
	U32 sLen = (U32)wcslen( str );

	if ( len + sLen > m_Size - 1 )
		LOGWARN( g_Log, DBSTR "Buffer too small. Truncating string\n" );

#ifdef PLATFORM_PS3
	U32 count = ( sLen > ( m_Size - 1 ) - len ) ? ( m_Size - 1 ) - len : sLen;
	wcsncat( m_Chars, str, count );
	m_Chars[ len + count ] = '\0';

#else // Xbox and PC are the same.
	wcsncat_s( m_Chars, m_Size, str, _TRUNCATE );
#endif
}


template < U32 m_Size >
void C_WideString<m_Size>::Add( const WCHAR c )
{
	U32 len = Length();

	if ( len < m_Size - 1 ) // room for '\0'
	{
		m_Chars[len] = c;
		m_Chars[len+1] = '\0';
	}
}


template < U32 m_Size >
C_WideString<m_Size> C_WideString<m_Size>::SubString( const S32 start, const S32 count ) const
{
	// Tmp buffer
	C_WideString<m_Size> buff;
	I_WideString::SubString( start, count, buff );

	return buff;
}

template < U32 m_Size >
void C_WideString<m_Size>::Insert( const WCHAR* str, const S32 pos )
{
	assert( pos >= 0 && pos < m_Size );

	// Make a tempory copy.
	C_WideString<m_Size> buff; 
	I_WideString::SubString( pos, Length(), buff );

	m_Chars[ pos ] = '\0';

	Add( str );
	Add( buff );// Add the rest of the string.
}

		
template < U32 m_Size >
void C_WideString<m_Size>::Insert( const WCHAR c, const S32 pos )
{
	assert( pos >= 0 && pos < m_Size );

	// Make a tempory copy.
	C_WideString<m_Size> buff; 
	I_WideString::SubString( pos, Length(), buff );

	m_Chars[ pos ] = '\0';

	Add( c );
	Add( buff );// Add the rest of the string.
}


template < U32 m_Size >
void C_WideString<m_Size>::Replace( U32 index, const WCHAR c  )
{
	assert( index >= 0 && index < m_Size );

	m_Chars[index] = c;
}

template < U32 m_Size >
void C_WideString<m_Size>::ReplaceCharacter( const WCHAR toReplace, const WCHAR newC )
{
	for ( S32 i = 0; i < m_Size; i++ )
		if ( m_Chars[ i ] == toReplace )
			m_Chars[ i ] = newC;
}

template < U32 m_Size >
const WCHAR* C_WideString<m_Size>::Chars() const 
{
	return m_Chars;
}


template < U32 m_Size >
WCHAR* C_WideString<m_Size>::NonConstChars()
{
	return m_Chars;
}


template < U32 m_Size >
U32 C_WideString<m_Size>::Length() const
{
	return wcslen( m_Chars );
}

template < U32 m_Size >
U32 C_WideString<m_Size>::Size() const
{
	return m_Size;
}


// Removed this functions as you shouldnt really be adding text that isnt localised.
/*
/// Will overide the contents starting from the given point with the give string.
void Replace( S32 start, const WCHAR* pStr  )
{
	if ( !pStr || start > Length() || ( start + wcslen( pStr ) > Length() ) )
		return; // Safety checks.

	S32 size = wcslen( pStr );
	for ( S32 i = 0; i < size; i++ )
	{
		m_Chars[i + start] = pStr[i];
	}
	m_Chars[ Length() ] = '\0';
}

void Replace( const WCHAR* pSrc, const WCHAR* pDest  )
{
	// Bail early.
	if ( !pSrc  || !pDest || wcslen( pSrc ) > Length() )
		return; // Never going to find it.

	for ( U32 i = 0, count = 0; i < Length(); i++ )
	{
		if ( m_Chars[i] == pSrc[count] )
			count++;
		else
			count = 0;

		// Check to see if we found it.
		if ( count == wcslen( pSrc ) )
		{
			if ( wcslen( pSrc ) == wcslen( pSrc ) )
			{
				// Are the same lengths, write over it.
				Replace( i - ( count - 1 ), pDest );
				return;
			}
			else
			{
				// Need to make some room for the extra chars.
				return;
			}
		}
	}
}
*/

}; // namespace
