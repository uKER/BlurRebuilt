// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline 
bdInt64 BD_CALL bdStrtoi64(const bdNChar8 *const s,
										   bdNChar8** endptr, 
										   const bdUInt32 radix)
{
	// ideally we'd use ::strtoll but the changes are it's not defined.
	// return ::strtoll(s, endptr, radix);

	if (radix == 10)
	{
		return ::atoll(s);
	}
	else
	{
		return bdStrtoi32(s, endptr, radix);
	}
}

inline 
bdUInt64 BD_CALL bdStrtoui64(const bdNChar8 *const s,
											 bdNChar8** endptr, 
											 const bdUInt32 radix)
{
	// ideally we'd use ::strtoull but the changes are it's not defined.
	// return ::strtoull(s, endptr, radix);

	if (radix == 10)
	{
		return static_cast<bdUInt64>(::atoll(s));
	}
	else
	{
		return bdStrtoui32(s, endptr, radix);
	}
}

inline 
bdUWord BD_CALL bdStrnlen(const bdNChar8 *const s, 
						  const bdUWord maxLen)
{
	const bdNChar8 *const pos = reinterpret_cast<const bdNChar8 *>(::memchr(s, '\0', maxLen));
	const bdUWord slen = pos ? (pos - s) : maxLen;
	return slen;
}

inline 
bdInt32 BD_CALL bdStrcasecmp(const bdNChar8 *const s1,
											 const bdNChar8 *const s2)
{
	return ::strcasecmp(s1, s2);
}

inline 
bdInt32 BD_CALL bdStrncasecmp(const bdNChar8 *const s1,
											  const bdNChar8 *s2,
											  const bdInt32 len)
{
	return ::strncasecmp(s1, s2, len);
}

inline 
bdInt BD_CALL bdVsnprintf(bdNChar8 *buf, 
						  const bdUWord maxlen, 
						  const bdNChar8 *format, 
						  bdVaList argPtr)
{
	const bdInt ret = ::vsnprintf(buf, maxlen, format, argPtr);
	return ret;
}

