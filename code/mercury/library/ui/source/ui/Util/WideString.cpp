
#include "stdafx.h"

#include "ui/util/WideString.h"

namespace UI
{

S32 I_WideString::Find( const WCHAR *pStr ) const
{
	// Bail early.
	if ( !pStr || wcslen( pStr ) > Length() )
		return -1; // Never going to find it.

	// Size of what were looking for.
	U32 size = Length();
	U32 strLen = wcslen( pStr );

	// Get a pointer to the char buffer.
	const WCHAR* chars = Chars();

	for ( U32 i = 0, count = 0; i < size + 1; i++ )
	{
		if ( chars[i] == pStr[count] )
			count++;
		else
			count = 0;

		// Check to see if we found it.
		if ( count == strLen )
			return i - ( count - 1 );
	}
	return -1;
}

void I_WideString::SubString( const S32 start, const S32 count, I_WideString& buff ) const
{
	// Bounds check.
	if ( start >= 0 && start < (S32)Length() && count > 0 )
	{
		// Copy contents and return.
		S32	size = count;

		if ( ( start + size ) > (S32)Length() )
			size = Length() - start;

		const WCHAR* chars = Chars();

		for ( S32 i = 0; i < size; i++ )
		{
			buff.NonConstChars()[i] = chars[i + start];
		}
		buff.NonConstChars()[size] = '\0';
	}
}


/*---------------------------------------------------------------------------------------
	Non-member functions.
---------------------------------------------------------------------------------------*/
#ifdef PLATFORM_PS3

const WCHAR* Ps3_Reformat( WCHAR* formatted_string, int formatted_string_length, const WCHAR* str )
{
	assert(str);
	assert(wcslen(str) < formatted_string_length/2 ); // formatted string can only ever be twice the size of the in_string

	// Copy over string inserting %ls where %s exist
	WCHAR* out = formatted_string;
	for(const WCHAR* in = str; *in; in++, out++)
	{
		if(*in == L'%') // Format seq
		{
			*out++ = *in++; // Copy the %
			if(*in == L'S') // Convert from char
			{
				*out = L's';	// convert to s
			}
			else
			if(*in == L's')	// Convert from WCHAR
			{
				*out++ = L'l';	// Insert l
				*out = *in;		// copy the s
			}
			else
				*out = *in;
		}
		else
		{
			*out = *in; // Just copy
		}
	}
	*out = NULL; // Terminate string

	return formatted_string;
}

#endif // PLATFORM_PS3


int WideStrArgs( WCHAR* str, const U32 size, const WCHAR* format, va_list va_args )
{
#ifdef PLATFORM_PS3
	WCHAR formatted_string[512];
	return vswprintf( str, size, Ps3_Reformat(formatted_string, lengthof(formatted_string), format ), va_args );
#else
	return vswprintf( str, size, format, va_args );
#endif
}

/// Will copy the src into the dest. If the src is bigger, then it will be truncated.
int WideStrFormat( WCHAR* str, const U32 size, const WCHAR* format, ... )
{
	va_list va_args;
	va_start( va_args, format );

	int character_count = WideStrArgs( str, size, format, va_args );

	va_end( va_args );

	return character_count;
}

/// Copy the src into the dest truncating if its too big.
void WideStrCopy( WCHAR* dst, const U32 size, const WCHAR* src )
{
#ifndef CFG_SPEED
	if ( wcslen( src ) > size )
		LOGWARN( UI::g_Log, DBSTR "Buffer too small. Truncating string\n" );
#endif

#ifdef PLATFORM_PS3
	wcsncpy( dst, src, size );
	dst[ size - 1 ] = '\0';

#else // Xbox and PC are the same.
    wcsncpy_s( dst, size, src, _TRUNCATE );
#endif
}


};
