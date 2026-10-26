/*! \file
	\brief Base library macros, types, and definitions header file.
*/
#ifndef TL_DEFS_H
#define TL_DEFS_H

#include "mercury_to_tl.h"

#if 0 + defined(PLATFORM_PC) + TL_PLAT_PS2 + TL_PLAT_PS2IOP + defined(PLATFORM_PS3) + defined(PLATFORM_SPU) + TL_PLAT_GC + TL_PLAT_REV != 1
 #error TL_PLAT_XXX platform specification is missing. It should be set via the compiler command line.
#endif

#if defined(PLATFORM_PC)
 #pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif


#if defined(PLATFORM_PS3)
 #include <sdk_version.h>
 #include <ppu_intrinsics.h>
#endif

#if defined(PLATFORM_SPU)
 #include <sdk_version.h>
 #include <spu_mfcio.h>
 #include <spu_intrinsics.h>
 #include <spu_printf.h>
#endif

//! Evaluates the offset of member m within structure or class s.
#ifdef COMPILER_GNU
#define tl_offsetof(s,m) __builtin_offsetof(s,m)
#else
#define tl_offsetof(s,m) ((size_t)&(((s*)1)->m)-1)
#endif

//! Evaluates the number of elements in static array x.
#define tl_elementsof(x) (sizeof(x) / sizeof(x[0]))

//! Returns p aligned to the next multiple of a.  A must be a power of 2.
template <typename T>
inline T tl_align(T p, size_t a) { return (T)(((size_t)(p)+((a)-1))&(~((a)-1))); }

//! Returns the smaller of a and b.
template <typename A, typename B>
inline A tl_min(const A& a, const B& b)
{
	return a<A(b) ? a : A(b);
}

//! Returns the larger of a and b.
template <typename A, typename B>
inline A tl_max(const A& a, const B& b)
{
	return a>A(b) ? a : A(b);
}

//! Returns true if x is an even power of 2.
inline bool IsPow2( unsigned int x )
{
	return x && !(x & (x - 1));
}


// ----------------------------- REQUIRED STUFF


inline U32 tlFloatAsInt(float v)
{
	union { float fv; U32 iv; } u;
	u.fv = v;
	return u.iv;
}

inline float tlIntAsFloat(U32 v)
{
	union { float fv; U32 iv; } u;
	u.iv = v;
	return u.fv;
}

//! Declares that a type must have an address that is a multiple of a.
#ifdef COMPILER_GNU
#define TL_ALIGNED(a) __attribute__ ((aligned(a)))
#elif defined(COMPILER_VC)
#define TL_ALIGNED(a) __declspec(align(a))
#endif

//! Returns the number of bits set in the value
inline int tlCountOnes(U32 v)
{
	int c = 0;
	while( v )
	{
		v &= v - 1;
		c++;
	}
	return c;
}

//! Evaluates the alignment requirement of a type.
/*!
	This is useful when allocating memory, to inform the allocator of the correct alignment requirement for any type.
*/
#if defined(COMPILER_VC)
#define tl_alignof(x) __alignof(x)
#elif defined(COMPILER_GNU)
#define tl_alignof(x) __alignof__(x)
#endif

#endif
