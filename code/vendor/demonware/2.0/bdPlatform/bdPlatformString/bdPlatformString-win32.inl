// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline 
bdInt64 BD_CALL bdStrtoi64(const bdNChar8 *const s,
						   bdNChar8** endptr, 
						   const bdUInt radix)
{	
	// return ::_strtoi64(s, endptr, radix);

	// ::_atoi64 should really be replaced with ::_strtoi64 and ::_strtoui64. 
	// As mentioned in the microsoft docs:
	// <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt_strtod.2c_.strtol.2c_.strtoul_functions.asp>
	// However they don't appear to be defined in <stdlib.h> for MSVC6.
	if(radix == 10)
	{
		return ::_atoi64(s);
	}
	else
	{
		return bdStrtoi32(s, endptr, radix);
	}
}

inline 
bdUInt64 BD_CALL bdStrtoui64(const bdNChar8 *const s,
							 bdNChar8** endptr, 
							 const bdUInt radix)
{
	// return ::_strtoiu64(s, endptr, radix);

	// ::_atoi64 should really be replaced with ::_strtoi64 and ::_strtoui64. 
	// As mentioned in the microsoft docs:
	// <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt_strtod.2c_.strtol.2c_.strtoul_functions.asp>
	// However they don't appear to be defined in <stdlib.h> for MSVC6.
	// As there is no unsigned equivalent to ::_atoi64 we will have problems with unsigned
	// 64 bit integers that are greater than 9223372036854775807.
	if(radix == 10)
	{
		return ::_atoi64(s);
	}
	else
	{
		return bdStrtoui32(s, endptr, radix);
	}
}

inline 
bdInt BD_CALL bdStrcasecmp(const bdNChar8 *const s1,
						   const bdNChar8 *const s2)
{
	return ::_stricmp(s1, s2);
}

inline 
bdInt BD_CALL bdStrncasecmp(const bdNChar8 *const s1,
							const bdNChar8 *s2,
							const bdInt len)
{
	return ::_strnicmp(s1, s2, len);
}

inline 
bdUWord BD_CALL bdStrnlen(const bdNChar8 *const s, 
						  const bdUWord maxLen)
{
	const bdNChar8 *const pos = reinterpret_cast<const bdNChar8 *const>(::memchr(s, '\0', maxLen));
	const bdUWord slen = pos ? (pos - s) : maxLen;
	return slen;
}


inline 
bdInt BD_CALL bdVsnprintf(bdNChar8 *buf, 
						  const bdUWord maxlen, 
						  const bdNChar8 *format, 
						  bdVaList argPtr)
{
#if defined BD_COMPILER_MSVC8 || defined BD_COMPILER_MSVC_XENON
	const bdInt ret = ::_vscprintf(format, argPtr);	
	::vsnprintf_s(buf, maxlen, _TRUNCATE, format, argPtr);

#elif defined BD_COMPILER_MSVC7 || defined BD_COMPILER_MSVC_XBOX
	const bdInt ret = ::_vscprintf(format, argPtr);	
	::_vsnprintf(buf, maxlen, format, argPtr);
	buf[maxlen-1] = '\0';	
#elif defined BD_COMPILER_MSVC6
	bdInt ret = ::_vsnprintf(buf, maxlen, format, argPtr);
	if(ret == -1)
	{
		buf[maxlen-1] = '\0';		
		// indicate an overflow occurred, we have no way to tell how big it was though
		ret = maxlen+1;
	};
#else
	error!!
#endif

	return ret;
}
