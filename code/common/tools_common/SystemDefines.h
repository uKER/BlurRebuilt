// this header file defines some common stuff that varies per platform

#ifndef __SYSTEM_DEFINES__
#define __SYSTEM_DEFINES__

#if defined ( __GNUC__ )
// GCC
typedef signed long long	S64;
typedef unsigned long long	U64;

typedef signed int			S32;
typedef unsigned int		U32;

typedef signed short		S16;
typedef unsigned short		U16;

typedef signed char			S8;
typedef unsigned char		U8;

typedef double				F64;
typedef float				F32;

#define FORCE_INLINE inline
#define FRIEND_CLASS friend class

// these functions are names differently on WIN32
#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strncmpi
#define strncmpi strncasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#elif defined( WIN32 ) || defined( AMD64 ) || defined( WIN64 ) || defined( _XENON ) || defined(PLATFORM_PC)

// windows
typedef signed __int64		S64;
typedef unsigned __int64	U64;

typedef signed __int32		S32;
typedef unsigned __int32	U32;

typedef signed __int16		S16;
typedef unsigned __int16	U16;

typedef signed __int8		S8;
typedef unsigned __int8		U8;

typedef double				F64;
typedef float				F32;

#define FORCE_INLINE __forceinline
#define FRIEND_CLASS friend

#else

#pragma message("*** ERROR: Unknown target platform.");

#endif

#if defined(_WIN64) || defined(__LP64__)
#define __64BIT__
#endif

#endif
