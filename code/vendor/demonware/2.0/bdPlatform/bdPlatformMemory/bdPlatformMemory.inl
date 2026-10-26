// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <stdlib.h>
#include <string.h>

inline 
void* BD_CALL bdMalloc(bdUWord size)
{
	void *const ptr = ::malloc(size);
	return ptr;
}

inline 
void BD_CALL bdFree(void *p)
{
	::free(p);
}

inline 
void* BD_CALL bdRealloc(void *p,
						bdUWord size)
{
	void *const ptr = ::realloc(p, size);
	return ptr;
}

inline 
void* BD_CALL bdMemchr(void *const s, 
									   const bdInt c, 
									   const bdUWord len)
{
	return ::memchr(s, c, len);
}

inline 
void* BD_CALL bdMemcpy(void *const dest,
									   const void *const src,
									   const bdUWord len)
{
	return ::memcpy(dest, src, len);
}

inline 
bdInt BD_CALL bdMemcmp(const void *const s1, 
										 const void *const s2,
										 const bdUWord len)
{
	return ::memcmp(s1, s2, len);
}

inline 
void* BD_CALL bdMemmove(void *const dest,
										const void *const src,
										const bdUWord len)
{
	return ::memmove(dest, src, len);
}

inline 
void* BD_CALL bdMemset(void *const s,
										const bdInt c,
										const bdUWord len)
{
	return ::memset(s, c, len);
}
