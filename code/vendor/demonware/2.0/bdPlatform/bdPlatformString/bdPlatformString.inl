// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

inline 
bdInt BD_CALL bdToLower(const bdInt c)
{
	return tolower(c);
}

inline 
bdInt BD_CALL bdToUpper(const bdInt c)
{
	return toupper(c);
}

inline 
bdNChar8* BD_CALL bdStrlwr(bdNChar8 *const s)
{
	bdInt i=0;
	while(s[i])
	{
		s[i] = static_cast<bdNChar8>(bdToLower(s[i]));
		i++;
	}

	return s;
}

inline 
bdNChar8* BD_CALL bdStrupr(bdNChar8 *const s)
{
	bdInt i=0;
	while(s[i])
	{
		s[i] = static_cast<bdNChar8>(bdToUpper(s[i]));
		i++;
	}

	return s;
}

inline 
bdInt BD_CALL bdStrtoi32(const bdNChar8 *const s,
						 bdNChar8** endptr, 
						 const bdUInt radix)
{
	return ::strtol(s, endptr, radix);
}

inline 
bdUInt BD_CALL bdStrtoui32(const bdNChar8 *const s,
						   bdNChar8** endptr, 
						   const bdUInt radix)
{
	return ::strtoul(s, endptr, radix);
}

inline 
bdFloat32 BD_CALL bdStrtof32(const bdNChar8 *const s,
							 bdNChar8** endptr)
{
	return static_cast<bdFloat32>(bdStrtof64(s, endptr));
}

inline 
bdFloat64 BD_CALL bdStrtof64(const bdNChar8 *const s,
							 bdNChar8** endptr)
{
	return ::strtod(s, endptr);
}

inline bdUWord BD_CALL bdStrlcat(bdNChar8 * const dst, 
								 const bdNChar8 * const src, 
								 const bdUWord size)
{
	bdUWord dlen = bdStrlen(dst);
	
	if (dlen < size - 1)
	{
		return(dlen + bdStrlcpy(dst + dlen,src, size - dlen));
	}
	else
	{
		return(dlen + bdStrlen(src)); 
	}
}

inline 
const bdNChar8* BD_CALL bdStrchr(const bdNChar8 *const s, 
								const bdInt c)
{
	return ::strchr(s, c);
}

inline 
bdNChar8* BD_CALL bdStrchr(bdNChar8 *const s, 
						  const bdInt c)
{
	return ::strchr(s, c);
}

inline 
const bdNChar8* BD_CALL bdStrrchr (const bdNChar8 *const s, 
								  const bdInt c)
{
	return ::strrchr(s, c);
}

inline 
bdNChar8* BD_CALL bdStrrchr (bdNChar8 *const s, 
							const bdInt c)
{
	return ::strrchr(s, c);
}

inline 
bdInt BD_CALL bdStrcmp(const bdNChar8 *const s1, 
					   const bdNChar8 *const s2)
{
	return ::strcmp(s1, s2);
}

inline 
bdInt BD_CALL bdStrncmp(const bdNChar8 *const s1, 
						const bdNChar8 *s2, 
						const bdInt len)

{
	return ::strncmp(s1, s2, len);
}

inline 
bdUWord BD_CALL bdStrlcpy(bdNChar8 * const dst, 
						  const bdNChar8 * const src, 
						  const bdUWord size)
{	
	const bdUWord slen = bdStrlen(src);
	
	if ((size != 0) && (dst != BD_NULL)) 
	{
		const bdUWord n = BD_MIN(slen, size - 1);
		bdMemcpy(dst, src, n);
		dst[n] = '\0';
	} 
	
	return slen; 
}

inline 
bdUWord BD_CALL bdStrlen(const bdNChar8 *const s)
{
	return ::strlen(s);
}


inline 
const bdNChar8* BD_CALL bdStrstr(const bdNChar8 *const str, 
								const bdNChar8 *const searchStr)
{
	return ::strstr(str, searchStr);
}

inline 
bdNChar8* BD_CALL bdStrstr(bdNChar8 *const str, 
						  const bdNChar8 *const searchStr)
{
	return ::strstr(str, searchStr);
}

inline 
bdInt BD_CALL bdSnprintf(bdNChar8 *buf, 
						 const bdUWord maxlen, 
						 const bdNChar8 *format,
						 ...)
{
	bdVaList argPtr;	
	bdVaStart(argPtr, format);

	const bdInt ret = bdVsnprintf(buf, maxlen, format, argPtr);

	bdVaEnd(argPtr);

	return ret;
}

inline
bdInt BD_CALL bdFprintf(bdFileHandle stream, 
						const bdNChar8 *format,
						...)
{
	va_list ap;
	va_start(ap, format);

	const bdInt ret = ::vfprintf(static_cast<FILE*>(stream), 
								 format,
								 ap);

	va_end(ap);
	return ret;
}

inline 
bdInt BD_CALL bdPrintf(const bdNChar8 *format,
					   ...)
{
	va_list ap;
	va_start(ap, format);

	const bdInt ret = ::vfprintf(static_cast<FILE*>(bdGetStdout()),
								 format,
								 ap);

	va_end(ap);
	return ret;
}

inline 
bdFileHandle BD_CALL bdGetStdout()
{
	return static_cast<bdFileHandle>(stdout);
}

inline 
bdFileHandle BD_CALL bdGetStderr()
{
	return static_cast<bdFileHandle>(stderr);
}

inline 
bdFileHandle BD_CALL bdGetStdin()
{
	return static_cast<bdFileHandle>(stdin);
}
