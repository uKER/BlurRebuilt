#ifndef MERCURY_CORE_H
#define MERCURY_CORE_H

#ifdef _MSC_VER
#pragma once
#endif

// [Rog] Enable memory tracking on debug builds
#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
#endif

// [Rog] This is needed to define _XBOX_VER

/*****************************************************************************/
// Name: Core.h - Mandatory include for Core Tech 
//
// Author: Ian Wilson
//
// Description: This file is where all platform, compiler and type defines are 
// made, as well as any constants. They are in named sections below, search for
// the following:
//
// Section: Build Configurations
// Section: Compiler Defines 
// Section: Platform Defines 
// Section: Type Defines 
// Section: Compiler specific type Defines 
// Section: Platform specific type Defines 
/*****************************************************************************/

#define CORE_TECH_MERCURY

/*****************************************************************************/
// Section: Build Configurations
/*****************************************************************************/

//Supported Build Configurations. Must be defined by project

// BUILD_CONFIG_DEBUG, BUILD_CONFIG_RELEASE, BUILD_CONFIG_PROFILE, 
// BUILD_CONFIG_SPEED, BUILD_CONFIG_MASTER

// The above configs should not be used directly for specifying conditional 
// code compilation. The following defines will provide the necessary 
// information such as "does this need to be removed for a Speed build" or
// "this should only be included for paranoid validation of structures".
// Some of these will be defined depending on the selected build config.

// Supported build related defines, which can be enabled as needed.

// CFG_VALIDATE, CFG_PROFILE, CFG_SPEED, CFG_GOLD, CFG_DEBUG

// #define CFG_DEBUG
// #define CFG_VALIDATE
// #define CFG_SPEED
// #define CFG_PROFILE
// #define CFG_GOLD

// The following sets up defines depending on build config. 

#if defined(BUILD_CONFIG_DEBUG)
	#define CFG_DEBUG 1
#elif defined(BUILD_CONFIG_RELEASE)
	#define CFG_RELEASE 1	
	//[MarkC] was in to test 
	//#define CFG_GOLD
	#ifdef _MSC_VER
		//Use the following to wrap #pragma optimize( "", off ) with.
		#define CFG_ALLOW_OPTIMIZE_OFF 1
	#endif
#elif defined(BUILD_CONFIG_PROFILE)
	#define CFG_PROFILE 1
	#define CFG_SPEED 1
#elif defined(BUILD_CONFIG_SPEED)
	#define CFG_SPEED 1
#elif defined(BUILD_CONFIG_MASTER)
	#define CFG_SPEED 1
	#define CFG_GOLD 1
	#ifndef GLOBAL_DEBUG_MESSAGE_LEVEL
	#define GLOBAL_DEBUG_MESSAGE_LEVEL 0
	#endif
#else
#error ("*** ERROR: Unknown build configuration")
#endif

//#define CFG_HACKED_MASTER 1

#if defined(CFG_HACKED_MASTER)
	#ifndef GLOBAL_DEBUG_MESSAGE_LEVEL
		#define GLOBAL_DEBUG_MESSAGE_LEVEL 0
	#endif
#endif

/*****************************************************************************/
// Section: Compiler Defines 
/*****************************************************************************/

//Supported Compilers

// COMPILER_VC7, COMPILER_VC8, COMPILER_GCC, COMPILER_XENON, COMPILER_XBOX

#ifdef _MSC_VER
	//MS compiler
	//#pragma message( "COMPILER_VC" )
	#define COMPILER_VC
	//Is it an Xbox, Xenon or PC?
	#if ( _MSC_VER < 1400 )
		#define COMPILER_VC_7
		//#pragma message( "COMPILER_VC_7" )
	#else
		#define COMPILER_VC_8
		//#pragma message( "COMPILER_VC_8" )
	#endif
#elif defined ( __GNUC__ )
    #if !defined( COMPILER_GNU )
	#define COMPILER_GNU
	#endif
	//#warning "COMPILER_GNU"
#else
	#error("*** ERROR: Unknown compiler ***");
#endif

#if defined( COMPILER_VC )

// JASOND: MSVC Warning Level 4 [/W4] disabled warnings:

// Disabling warning C4127 because the compiler is whinging pointlessly
// about compile-time constant template parameters being used in conditionals...
#pragma warning( disable: 4127 ) // warning C4127: conditional expression is constant

// Disabling warning C4201 because the PS3 compiler supports nameless structs/unions
#pragma warning( disable: 4201 ) // warning C4201: nonstandard extension used : nameless struct/union

// Disabling warning C4324 because the compiler is whinging pointlessly about
// __declspec(align()) marked structures being padded... Well, yes we know, what else is __declspec(align()) for ?!?
#pragma warning( disable: 4324 ) // warning C4324: structure was padded due to __declspec(align())

#endif

/*****************************************************************************/
// Section: Target Platform Defines 
/*****************************************************************************/

// Supported Target Platforms:

// PLATFORM_PC, PLATFORM_XENON, PLATFORM_PS3, PLATFORM_PSP, 

// Look for specified target e.g. PS3/Xenon/Revolution, otherwise assume PC.
#if defined SN_TARGET_PS3_SPU
	#define WORD_SIZE 32
	#ifndef PLATFORM_SPU
	#define PLATFORM_SPU 1
	#endif
	#if !defined( PLATFORM_PPU_SPU )
    #define PLATFORM_PPU_SPU 1
    #endif
#elif defined PLATFORM_PS3
	#define WORD_SIZE 32
	//#pragma message ("PLATFORM_PS3")
	//#pragma message ("32-BIT WORD SIZE")

#elif ((defined _WIN64) || (defined WIN64))
	#define WORD_SIZE 64
	#define PLATFORM_WINDOWS 1
	#ifndef PLATFORM_PC
	#define PLATFORM_PC 1
	#endif
	#ifndef WIN64
	#define WIN64
	#endif
	//#pragma message ("PLATFORM_WINDOWS")
	//#pragma message ("64-BIT WORD SIZE")
#elif ((defined _WIN32) || (defined WIN32))
	#define WORD_SIZE 32
	#define PLATFORM_WINDOWS 1
	#ifndef PLATFORM_PC
	#define PLATFORM_PC 1
	#endif
	//#pragma message ("PLATFORM_WINDOWS")
	//#pragma message ("32-BIT WORD SIZE")
#else 
	#define PLATFORM_LINUX 1
//	#warning "PLATFORM_LINUX"
	#ifdef __powerpc64__
		#define WORD_SIZE 64
//		#warning "64-BIT WORD SIZE"
	#else
		#define WORD_SIZE 32
//		#warning "32-BIT WORD SIZE"
	#endif
#endif

/*****************************************************************************/
// Section: Function level optimisation control
/*****************************************************************************/
//
// Defines OPTIMISE_OFF and ON to allow function level optimisation
// control in MSVC.
// 
/*****************************************************************************/
#define OPTIMISE_OFF_IMPL2(lineNo) #lineNo
#define OPTIMISE_OFF_IMPL(lineNo) __FILE__ "(" OPTIMISE_OFF_IMPL2(lineNo) "): warning: Optimisations disabled"

#if defined(COMPILER_VC)
	#define OPTIMISE_OFF __pragma(optimize("", off)) __pragma(message(OPTIMISE_OFF_IMPL(__LINE__)))
	#define OPTIMISE_ON __pragma(optimize("", on))
#else
	// Some defines for the future when we eventually move over to SNC.
	// define OPTIMISE_OFF	_Pragma("control %push O=0") _Pragma(message(OPTIMISE_OFF_IMPL(__LINE__)))
	// define OPTIMISE_ON	_Pragma("control %pop O=0")
	#define OPTIMISE_OFF
	#define OPTIMISE_ON
#endif

/*****************************************************************************/
// Section: Compile-time messages
/*****************************************************************************/
//
// COMPILER_WARNING(text) asks the compiler to generate a warning and to
// display the message supplied.
// 
/*****************************************************************************/
#if defined(COMPILER_VC)
#define COMPILER_WARNING_STRINGIFY(x) #x
#define COMPILER_WARNING_STRINGIFY2(x) COMPILER_WARNING_STRINGIFY(x)
#define COMPILER_WARNING_MESSAGE(x) __FILE__ "(" COMPILER_WARNING_STRINGIFY2(__LINE__) ") : warning : " x
#define COMPILER_WARNING(text) __pragma(message (COMPILER_WARNING_MESSAGE(text)))
#else
#define COMPILER_WARNING(text) int pleaseCompileInMSVCToSeeCustomWarnings = pleaseCompileInMSVCToSeeCustomWarnings;
#endif

/*****************************************************************************/
// Section: Type Defines 
/*****************************************************************************/
//
// Typedefs for chars, ints, floats, etc for each compiler, then specialised
// as necessary per platform
//
/*****************************************************************************/

/*****************************************************************************/
// Common defines
/*****************************************************************************/

// lengthof - returns the number of elements in an array
#ifndef lengthof
#if defined(COMPILER_VC) && _MSC_VER >= 1400
#define lengthof(x) _countof(x)
#else
#define lengthof(x) (sizeof(x) / sizeof((x)[0]))
#endif
#endif

#define NO_VTABLE __declspec(novtable)

#define INLINE inline

/*****************************************************************************/
// Section: Compiler specific type Defines 
/*****************************************************************************/
// GCC Typedefs
/*****************************************************************************/

#ifdef COMPILER_GNU

//GCC typedefs
typedef signed long long	S64;
typedef unsigned long long	U64;

typedef signed int			S32;
typedef unsigned int		U32;

typedef signed short		S16;
typedef unsigned short		U16;

typedef signed char			S8;
typedef unsigned char		U8;

typedef char				CHAR8;

typedef double				F64;
typedef float				F32;

typedef unsigned long       DWORD;
typedef unsigned int		INTPTR;//TODO: "the int the same size as a pointer" - verify

#define FORCE_INLINE inline
#define ALWAYS_INLINE __attribute__(( always_inline ))
#define FLATTEN __attribute__(( flatten ))
#define FRIEND_CLASS friend class
//#define SELECTANY __attribute__(( selectany )) // GCC for Win32 only apparently...
#define SELECTANY
#define WEAK __attribute__(( weak ))
#define NOINLINE __attribute__(( noinline ))
#define NOVTABLE
#define NORETURN __attribute__(( noreturn ))

// NOTE: GCC __attribute__(( pure | const )) are stricter than MSVC __declspec( noalias )!
#define NOALIAS

//#if !defined( PLATFORM_SPU )
//#define ATTR_PURE  __attribute__(( pure ))
//#define ATTR_CONST __attribute__(( const ))
//#else // These seem to be broken on the SPU compiler!
#define ATTR_PURE
#define ATTR_CONST
//#endif

#define RETURN_RESTRICT __attribute__(( malloc ))

// Override Specifiers
#define SEALED
#define OVERRIDE
#define ABSTRACT
#define sealed
#define override
#define abstract

//#ifndef PLATFORM_PS3
//#define __restrict 
//#endif

// these functions are named differently on WIN32
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
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

#define OutputDebugString printf

#ifdef PLATFORM_SPU
#define DebugBreak() { __asm volatile ("stopd 0,1,1"); }
#else
#ifndef CFG_GOLD
#define DebugBreak() { printf("DebugBreak() %s %d\n",__FILE__,__LINE__); snPause(); }
#else
#define DebugBreak() { snPause(); }
#endif
#endif
#define _CVTBUFSIZE (309+40)
//#define __forceinline inline __attribute__ ((always_inline))
#define __forceinline inline 
#define __assume(X)
#define _finite isfinite

#define _snprintf snprintf
#define _alloca alloca
#define INFINITE 0xffffffff
#define DEPRECATED __attribute__ ((deprecated))
#define THREAD_LOCAL_STORAGE __thread

/*****************************************************************************/
// VC Typedefs
/*****************************************************************************/

#elif (defined COMPILER_VC)

//VC typedefs

typedef unsigned __int64 U64;
typedef __int64 S64;

typedef unsigned __int32 U32;
typedef int S32;

typedef unsigned __int16 U16;
typedef __int16 S16;

typedef unsigned __int8 U8;
typedef signed __int8 S8;

typedef char		CHAR8;

typedef double F64;
typedef float F32;

typedef unsigned long       DWORD;
#if defined(_WIN64) // [Blur2PC][x64] INTPTR holds pointers for AlignUp/AlignDown and address arithmetic.
typedef unsigned __int64 INTPTR;
#else
typedef	unsigned int	INTPTR;		//TODO:"the integer the same size as a pointer"- check defines for 32 v 64bit
#endif

#define FORCE_INLINE __forceinline
#define ALWAYS_INLINE
#define FLATTEN
#define FRIEND_CLASS friend
#define SELECTANY __declspec( selectany )
#define WEAK
#define NOINLINE __declspec( noinline )
#define NOVTABLE __declspec( novtable )
#define NORETURN __declspec( noreturn )
#define __attribute__(X)
#define THREAD_LOCAL_STORAGE __declspec( thread )

#ifdef COMPILER_VC_7

#define NOALIAS
#define ATTR_PURE
#define ATTR_CONST

#define RETURN_RESTRICT 

// Override Specifiers
#define SEALED
#define OVERRIDE
#define ABSTRACT
#define sealed
#define override
#define abstract

#else

// Override Specifiers
#define SEALED   sealed
#define OVERRIDE override
#define ABSTRACT abstract

#define NOALIAS __declspec(noalias)
#define ATTR_PURE __declspec(noalias)
#define ATTR_CONST __declspec(noalias)

#define RETURN_RESTRICT __declspec(restrict)

#endif

#ifndef DEPRECATED
	#define DEPRECATED __declspec(deprecated)
#endif 

#if defined(COMPILER_VC_8) //restrict only supported in later MS compilers
//#define RESTRICT __restrict
#else
#define __restrict
#endif

#else
	#error("*** ERROR: Cannot identify correct compiler typedefs ***");
#endif

// Cache control - Prefetch(DCBT), Flush(DCBF), Allocate&Clear(DCBZ128)
#define	STANDARD_PREFETCH_AHEAD_DIST 1024


#define CACHELINE_SIZE		128
#ifdef PLATFORM_PS3
#include <ppu_intrinsics.h>
#define DCBT(offset, base)	__dcbt( ((U8*)(offset)) + (U32)(base) )
#define DCBF(offset, base)  __dcbf( ((U8*)(offset)) + (U32)(base) )
#define DCBZ128(offset, base)	__dcbz( ((U8*)(offset)) + (U32)(base) )
#else
#define DCBT(offset, base)	do {} while(0)
#define DCBF(offset, base)	do {} while(0)
#define DCBZ128(offset, base) memset( (void*)(((offset+(INTPTR)(base)))&(~(128-1))), 0, 128);
#endif
#define DCBT_AHEAD(offset, base)		DCBT(offset+STANDARD_PREFETCH_AHEAD_DIST, base) 
// Does not use cacheline define since the expected behaviour is to clear 128 aligned bytes.



// TODO - WHATS THE POINTER-INT TYPE ?



/*****************************************************************************/
// Section: Platform specific type Defines 
// 
// These should be used to override where necessary the defines for the 
// compiler related to the platform. e.g. Specific defines for PS3 which 
// are different to GCC
/*****************************************************************************/



/*****************************************************************************/
// PS3 typedefs
/*****************************************************************************/

#if PLATFORM_PS3 || PLATFORM_SPU

#define __w64	long
#define BOOL	bool
#define FALSE 0
#define TRUE  1
#define VOID void
#define ULONG_PTR DWORD
#define _wtoi(x) wcstol( x, NULL, 10 )
#define _wtof(x) wcstof( x, NULL )
#define _TRUNCATE ((size_t)-1)

#endif

#ifdef PLATFORM_PS3

#define PLATFORM_PPU
#define PLATFORM_PPU_SPU 1

#include <altivec.h>
#include <stdint.h>
#include <sdk_version.h>

//#define PLATFORM_PS3_LOAD_FROM_BLURAY

#ifndef _WINDOWS_TYPES_DEFINED_
#define _WINDOWS_TYPES_DEFINED_
// TODO - move to windows typedefs emulator .h
typedef ::std::uint64_t		U64;
typedef ::std::int64_t		S64;
typedef ::std::uint32_t		U32;
typedef ::std::int32_t		S32;
typedef unsigned	short	U16;
typedef				short	S16;
typedef unsigned	char	U8;
//typedef				char	S8;
typedef				char	CHAR8;
typedef				double	F64;
typedef				float	F32;
typedef void* HANDLE;
typedef HANDLE HWND;
typedef float FLOAT;
typedef unsigned int UINT;
typedef int64_t __int64;
typedef int32_t LONG;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef U32 IN_ADDR;
typedef void* LPVOID;
typedef DWORD *LPDWORD;
typedef void *PVOID;
typedef const void* LPCVOID;
typedef const char* LPCTSTR;
typedef long LONG_PTR;
typedef unsigned long _fsize_t;
typedef long* PLONG;
typedef char CHAR;
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
typedef short SHORT;
typedef const WCHAR *LPCWSTR;
typedef const CHAR *LPCSTR;
typedef unsigned char BYTE;

typedef union _LARGE_INTEGER
{
    struct
    {
        LONG HighPart;
        DWORD LowPart;
    };
    struct 
    {
        LONG HighPart;
        DWORD LowPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER
{
    struct
    {
        DWORD HighPart;
        DWORD LowPart;
    };
    struct 
    {
        DWORD HighPart;
        DWORD LowPart;
    } u;
    LONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;
#endif

#elif defined(PLATFORM_SPU)

typedef void* HANDLE;
typedef char CHAR;
typedef const CHAR *LPCSTR;


/*****************************************************************************/
// PSP Typedefs
/*****************************************************************************/

#elif defined(PLATFORM_PSP)

// PSP TYPE DEFINITIONS HERE

#endif



/*****************************************************************************/
// PS3 PPU & SPU Typedefs
/*****************************************************************************/
#if defined(PLATFORM_PS3) || defined(PLATFORM_SPU)

// TODO - move to xenon intrisics emulator .h
// Emulation of microsoft native vector4 interoperable with GCC VMX intrinsics.
#ifndef __VECTOR4_DEFINED
#define __VECTOR4_DEFINED


struct __constvector4;
struct __attribute__((aligned(16))) __vector4
{
	union
	{
		vector float m_Vector;
		struct
		{
			F32	x;
			F32	y;
			F32	z;
			F32	w;
		};
		F32	v[4];
		
		U32	u[4];
	};	

	__vector4(void) { 
	};
	__vector4(const float _x, const float _y, const float _z, const float _w) { 
		v[0]=_x;
		v[1]=_y;
		v[2]=_z;
		v[3]=_w;
	};
	//operator vector float () const {
		//return m_Vector;
	//}


	// CONVERT FROM
	inline __vector4(vector float src) {
		m_Vector= ( src);
	}
	inline __vector4(vector unsigned int src) {
		m_Vector= ( (vector float) src);
	}
	inline __vector4(vector signed int src) {
		m_Vector= ( (vector float) src);
	}
	inline __vector4(vector unsigned short src) {
		m_Vector= ( (vector float) src);
	}
	inline __vector4(vector signed short src) {
		m_Vector= ( (vector float) src);
	}
	inline __vector4(vector unsigned char src) {
		m_Vector= ( (vector float) src);
	}
	inline __vector4(vector signed char src) {
		m_Vector= ( (vector float) src);
	}

	// CONVERT TO
	inline operator __constvector4 () const;
	inline operator vector float () const{
		return m_Vector;
	}
	inline operator vector unsigned char() const{
		return (vector unsigned char) m_Vector;
	}
	inline operator vector signed char() const{
		return (vector signed char) m_Vector;
	}

	inline operator vector unsigned short() const{
		return (vector unsigned short) m_Vector;
	}
	inline operator vector signed short() const{
		return (vector signed short) m_Vector;
	}
	

	inline operator vector unsigned int() const{
		return (vector unsigned int) m_Vector;
	}
	inline operator vector signed int() const{
		return (vector signed int) m_Vector;
	}
	//operator const vector float () const; //{
		//return (const vector float)  m_Vector;
	//}
};


#endif	// __VECTOR4_DEFINED

#endif




/*****************************************************************************/
// 360 Typedefs
/*****************************************************************************/


/*****************************************************************************/
// PC Typedefs
/*****************************************************************************/

#ifdef PLATFORM_PC
#ifdef PLATFORM_PC_TODO

#define PLATFORM_PS3_TODO

#endif


#define _TRUNCATE ((size_t)-1)

#endif

#if defined(COMPILER_VC)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#define _USE_MATH_DEFINES

#define USE_NEW_ALLOCATOR

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#ifdef PLATFORM_PS3
#include <fastmath.h>
#else
#include <math.h>
#endif

#include <float.h>
//#if defined( PLATFORM_PPU_SPU )
// JASOND: Redefining FLT_MAX & FLT_MIN for the PS3 since the SPU constant for FLT_MAX is getting trashed...
// NEW: Redefining them for all platforms, so that 1/FLT_MAX = FLT_MIN and so that we've got consistent values across all platforms
#undef FLT_MAX // 3.402823466e+38F
#undef FLT_MIN // 1.175494351e-38F
//#define FLT_MAX (1.0e37f)
//#define FLT_MIN (1.0e-37f)
#define FLT_MAX (8.5070591730234615865843651857942e+37f)
#define FLT_MIN (1.1754943508222875079687365372222e-38f)
//#endif

#include <core/allocators/CoreMemoryFunctions.h>

#if (!defined(PLATFORM_PS3)) && (!defined(PLATFORM_SPU))
#include <malloc.h>
#endif

#ifdef PLATFORM_SPU
#include <spu_printf.h>
#endif

#include <core/utility/coredebug.h>

#ifndef SAFE_DELETE_PTR
#define SAFE_DELETE_PTR(_ptr) if (_ptr) { delete _ptr; _ptr = NULL; }
#endif
#ifndef SAFE_DELETE_ARY
#define SAFE_DELETE_ARY(_ary) if (_ary) { delete[] _ary; _ary = NULL; }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(_res) if (_res) { _res->Release(); _res = NULL; }
#endif

// macro to get a symbol generated using objcopy
#define OCSYMBOL(X) X


#ifndef PLATFORM_PS3
#define DBSTR __FUNCTION__ ":"
#else
#define DBSTR 
#endif

#ifdef COMPILER_GNU
#define _copysign copysign
#define snPause() __asm__ volatile ( "tw 31,1,1" )
#endif

#ifdef PLATFORM_SPU
#define PPUDATA __attribute__ ((section ("ppudata")))
#define PPUDATA_STRING(X) ((char*)(X+(int)Scheduler::_jqThisWorker->PPUModuleAddress))
#else
#define PPUDATA
#define PPUDATA_STRING(X) (X)
#endif

/// macro to knock out stray #pragma optimize("", off) in speed builds
#ifdef COMPILER_VC
#ifdef CFG_SPEED
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define optimize(A, B) message(__FILE__ "(" TOSTRING(__LINE__) "): WARNING: Optimize " #B " ignored." )
#endif
#endif

#endif //end of MERCURY_CORE_H

