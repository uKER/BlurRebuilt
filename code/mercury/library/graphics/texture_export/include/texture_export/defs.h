/*! \file defs.h
	\brief ActToolsLib library macros, types, and definitions header file.
*/
#ifndef DEFS_H__INCLUDED
#define DEFS_H__INCLUDED

#if defined(_MSC_VER) && (_MSC_VER > 1000)
	#pragma once
#endif

/** \defgroup ACTTOOLSLIB_MACROS Macros/Pragmas
	@{
*/
/// Converts macro parameters (after expansion) to string constants.
#define __X(x) #x
/// Converts macro parameters (after expansion) to string constants.
#define __QUOTE(x) __X(x)
/// The line number in the current source file and the name of the current source file.
#define __FILE_LINE__ __FILE__ "(" __QUOTE(__LINE__) ") : "
/// Pragma that sends a "TODO: ..." formated string literal to the standard output without terminating the compilation.
#define TODO(x)   message(__FILE_LINE__ " TODO :   " #x "\n")
/// Pragma that sends a "FIXME: ..." formated string literal to the standard output without terminating the compilation.
#define FIXME(x)  message(__FILE_LINE__ " FIXME:   " #x "\n") 
/// Pragma that sends a "WARNING: ..." formated string literal to the standard output without terminating the compilation.
#define WARN(x)   message(__FILE_LINE__ " WARNING:   " #x "\n")

#ifndef NULL
/// NULL is the null-pointer value used with many pointer operations and functions.
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif

/// NULL is the null-pointer value used with many pointer operations and functions.
#ifndef FALSE
	#define FALSE               0
#endif

#ifndef TRUE
	#define TRUE                1
#endif

#if !defined(ACT_TOOLS_NOOP)
/// ACT_TOOLS_NOOP specifies that a function should be ignored and the argument list unevaluated.
	#if defined(_MSC_VER) && ( 1300 <= _MSC_VER )
		#define ACT_TOOLS_NOOP    __noop
	#else
		#define ACT_TOOLS_NOOP    ((void)0)
	#endif
#endif // ACT_TOOLS_NOOP

#if !defined(ACTT_UNUSED)
	#define ACTT_UNUSED(identifier) /* identifier */
#endif // ACTT_UNUSED

#define ACT_TOOLS_DECLARE_TYPE( mydecl, mytype ) \
	typedef mydecl mytype;                       \
	typedef mytype * mytype ## Ptr;              \
	typedef const mytype * mytype ## Cptr;       \
	typedef mytype & mytype ## Ref;              \
	typedef const mytype & mytype ## Cref
#define ACT_TOOLS_DECLARE_CLASS( tag ) \
	class tag;                         \
	typedef tag * tag ## Ptr;          \
	typedef const tag * tag ## Cptr;   \
	typedef tag & tag ## Ref;          \
	typedef const tag & tag ## Cref
//@}

/** \defgroup ACTTOOLSLIB_TYPE Types
	Basic types are defined as follows:

	int, unsigned, uint, float  Platform word sized variables typically used for counters, guaranteed to be 32bits
	u8, s8, i8                  8bit integers
	u16, s16, i16               16bit integers
	u32, s32, i32               32bit integers
	u64, s64, i64               64bit integers
	f32, f64                    Real numbers

	Usage convention:
		For values whose size is not important (loop variables, return values, etc.) use float or int.
		For strings or text, use char*.
    @{
*/
typedef unsigned int        uint;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef signed char         s8;
typedef signed short        s16;
typedef signed char         i8;
typedef signed short        i16;
typedef float               f32;
typedef double              f64;

typedef signed int          i32;
typedef signed int          s32;
typedef unsigned int        u32;
typedef unsigned __int64    u64;
typedef signed __int64      s64;
typedef signed __int64      i64;

struct __ActToolsIterator { };
typedef __ActToolsIterator* ActToolsIterator;
//@}

/// Returns the number of elements in array
template <typename T>
size_t elementsof( const T& array ) { return sizeof(array) / sizeof(array[0]); }

/// Returns p aligned to the next multiple of a. a must be a power of 2.
inline size_t align( size_t p, size_t a ) { return ((uint)p+(a-1))&(~(a-1)); }

/// Returns p aligned to the next multiple of a.  'a' does not need to be a power of 2.
template <typename T>
inline T align_nonpow2( T p, size_t a ) { return (T)((((size_t)(p) + a + 1) / a) * a); }

/// Returns x with reversed bits.
template <typename T>
inline T ReverseBits( T x )
{
	T y = 0;
	for(int i = 0; i < sizeof(T)*8; i++)
	{
		y <<= 1;
		y |= (x & 1);
		x >>= 1;
	}
	return y;
}
/** \defgroup ACTTOOLSLIB_ENDIANSWAP Endian Swapping
    @{
*/
#ifdef __cplusplus
/// Endian swap 32bit unsigned values.
inline u32 EndianSwap32( u32 x ) { return (x>>24) | ((x>>8)&0xff00) | ((x&0xff00)<<8) | (x<<24); }
/// Endian swap 32bit signed values.
inline i32 EndianSwap32( i32 x ) { return (x>>24) | ((x>>8)&0xff00) | ((x&0xff00)<<8) | (x<<24); }

/// Endian swap 16bit unsigned values.
inline u16 EndianSwap16( u16 x ) { return (x>>8) | (x<<8); }
/// Endian swap 16bit signed values.
inline i16 EndianSwap16( i16 x ) { return (x>>8) | (x<<8); }

/// Endian swap 32bit float values.
inline float EndianSwap32( float x ) { u32 y = (u32&)x; y = EndianSwap32(y); return (float&)y; }

/// Endian swap an array of 32bit values (can be float, signed or unsigned).
inline void EndianSwapMemory32( void* aData, size_t aBytes )
{
#ifdef assert
	assert((aBytes&3) == 0);
#endif

	u32* ptr = static_cast<u32*>(aData);
	for ( size_t i = 0; i < aBytes/4; ++i )
		*ptr++ = EndianSwap32(*ptr);
}
/// Endian swap an array of 16bit values (signed or unsigned).
inline void EndianSwapMemory16( void* aData, size_t aBytes )
{
#ifdef assert
	assert((aBytes&1) == 0);
#endif

	u16* ptr = static_cast<u16*>(aData);
	for ( size_t i = 0; i < aBytes/2; ++i )
		*ptr++ = EndianSwap16(*ptr);
}
#endif // __cplusplus
//@}

#ifdef ACT_TOOLS_DLL_EXPORT
  #define ACT_TOOLS_DLL_API extern "C" __declspec(dllexport)
#else
  #define ACT_TOOLS_DLL_API extern "C" __declspec(dllimport)
#endif

/** \defgroup ACTTOOLSLIB_ERRORS Error Handling
    @{
*/
/// Error type.
#define ACTRESULT int
/// Error code for success
#define ACT_OK -1
/// Error code for failure
#define ACT_ERROR 0

/// Macor to check success code
#define ACT_SUCCEEDED(x) ( (x))
/// Macor to check failure code
#define ACT_FAILED(x)    (!(x))
//@}

#endif // DEFS_H__INCLUDED

