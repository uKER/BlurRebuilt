#ifndef NEWCLIB_H
#define NEWCLIB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <core/bizassert.h>

#ifdef COMPILER_VC_7
#define USE_STR_S
#include <string.h>
#include <stdio.h>
#endif

#ifdef COMPILER_GNU
#define USE_STR_S
#include <string.h>
#include <ctype.h>
#endif



#ifdef USE_STR_S
#include <stdio.h>
#include <stdlib.h>

#define sscanf_s    sscanf
#define _access_s   _access

#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))

#ifdef PLATFORM_PS3
#define snPause() __asm__ volatile ( "tw 31,1,1" )
#endif

inline U32 strcpy_s( char* dest, U32 bufSize, const char* src )
{
	int lenSrc = strlen(src);

	if (lenSrc < bufSize)
	{
		strcpy(dest, src);
#ifdef CFG_DEBUG
		memset(dest+lenSrc+1, -2, bufSize-lenSrc-1);
#endif
		return 0;
	}
	else
	{
		abort();
		return -1;
	}
}

inline U32 strncpy_s( char* dest, U32 bufSize, const char* src, std::size_t length )
{
	// Allowed - nothing to do
	if (length == 0 && dest == NULL && bufSize == 0)
		return 0;

	//BIZ_ASSERT(dest != NULL && bufSize > 0);
	if (dest == NULL || bufSize == 0)
	{
		return -1;
	}

	// Allowed - source string can be NULL too
	if (length == 0)
	{
		dest[0] = '\0';
#ifdef CFG_DEBUG
		memset(dest+1, -2, bufSize-1);
#endif
		return 0;
	}

	if ((int)length == -1) // Truncate
	{
		int len = strlen(src);
		int copyLen = MIN(bufSize-1, len);
		strncpy(dest, src, copyLen);
		dest[copyLen] = '\0';
#ifdef CFG_DEBUG
		memset(dest+copyLen+1, -2, bufSize-copyLen-1);
#endif
	}
	else
	{
		int len = strlen(src);
		int copyLen = MIN(length, len);

		if (copyLen < bufSize)
		{
			strncpy(dest, src, copyLen);
			dest[copyLen] = '\0';
#ifdef CFG_DEBUG
		memset(dest+copyLen+1, -2, bufSize-copyLen-1);
#endif
		}
		else
		{
			// Copy won't fit - we fail to stop anything bad happening
			// To avoid this, you could always use _TRUNCATE to cap the string
			abort();
		}
	}
    return 0;
}

inline U32 sprintf_s( char* buffer, U32 bufSize, const char* format, ... )
{
	va_list arglist;
	va_start( arglist, format );
    vsprintf( buffer, format, arglist );
	va_end( arglist );
    return true;
}

inline U32 vsprintf_s( char* buffer, U32 bufSize, const char* format, va_list arglist )
{
#if defined(PLATFORM_PC) && (_MSC_VER < 1400)
    if (_vsnprintf( buffer, bufSize-1, format, arglist ) == -1)
	{
		// Cap the buffer if we hit the limit
		buffer[bufSize-1] = '\0';
	}
#else
    vsnprintf( buffer, bufSize, format, arglist );
#endif
    return true;
}

inline U32 strcat_s( char* dest, U32 bufSize, const char* toCat )
{
	int lenDst = strlen(dest);
	int lenSrc = strlen(toCat);

	if (lenDst + lenSrc < bufSize)
	{
		strcat( dest, toCat );
		return 0;
	}
	else
	{
		abort();
		return -1;
	}
}

inline U32 strncat_s( char* dest, U32 bufSize, const char* toCat, std::size_t length )
{
	int lenDst = strlen(dest);
	int lenSrc = strlen(toCat);

	if ((int)length == -1) // Truncate
	{
		int copyLen = MIN(bufSize-1-lenDst, lenSrc);

		strncat( dest, toCat, copyLen );
		return 0;
	}
	else
	{
		int copyLen = MIN(length, lenSrc);

		if (lenDst+copyLen < bufSize)
		{
			strncat( dest, toCat, copyLen );
			return 0;
		}
		else
		{
			// String won't fit - we fail to stop anything bad happening
			// To avoid this, you could always use _TRUNCATE to cap the string
			abort();
		}
	}
}

inline U32 _strupr_s( char* dest, U32 bufSize )
{
#ifdef COMPILER_GNU
	int len = strlen( dest );
	for (int i = 0 ; i < len ; i++)
	{
		dest[i] = toupper(dest[i]);
	}
#else
    _strupr( dest );
#endif
    return true;
}

inline U32 _strlwr_s( char* dest, U32 bufSize )
{
#ifdef COMPILER_GNU
	int len = strlen( dest );
	for (int i = 0 ; i < len ; i++)
	{
		dest[i] = tolower(dest[i]);
	}
#else
    _strlwr( dest );
#endif
    return true;
}

inline U32 _ltoa_s( U32 value, char* dest, U32 bufSize, U32 base )
{
#ifdef COMPILER_GNU
	int i = 0;
	while ( value > 0 )
	{
		char c = value % base;
		if (c < 10) c += '0';
		else c += '7';
		memmove(&dest[1], &dest[0], i);
		dest[0] = c; 
		value /= base;
		i++;
	}
	dest[i] = '\0';
	return true;
#else
    _ltoa( value, dest, base );
    return true;
#endif
}

inline U32 _gcvt_s( char* dest, U32 bufSize, double value, U32 decPlaces )
{
#ifdef COMPILER_GNU
	char fmt[256];
	const int integer = 16;
	sprintf( fmt, "%%%d.%df", integer, decPlaces );
	sprintf( dest, fmt, value );
#else	
    _gcvt( value, decPlaces, dest );
#endif
    return true;
}

#endif


#endif
