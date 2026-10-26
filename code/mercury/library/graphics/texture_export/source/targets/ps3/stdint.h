/* SCE CONFIDENTIAL
PLAYSTATION(R)3 Programmer Tool Runtime Library 091.006
* Copyright (C) 2006 Sony Computer Entertainment Inc. 
* All Rights Reserved.
*/
/* stdint.h standard header */
#ifndef _STDINT
#define _STDINT
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

/* NB: also assumes
	long is 32 bits
	pointer can convert to and from long long
	long long is longest type
 */

/* Get the int types from Lv-2's sys/integertypes.h */
#include <sys/integertypes.h>

_C_STD_BEGIN

		/* TYPE DEFINITIONS */
 #ifndef _SYS_INT_TYPES_H
  #define _STD_USING_INT_TYPES

typedef signed char int_least8_t;
typedef short int_least16_t;
//typedef _Int32t int_least32_t;
typedef _Longlong int_least64_t;
typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
//typedef _Uint32t uint_least32_t;
typedef _ULonglong uint_least64_t;
 #endif /* _SYS_INT_TYPES_H */

typedef int32_t int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;

 #ifndef _INTPTR
  #define _INTPTR	0	/* 0 => int, 1 => long, 2 => long long */
 #endif /* _INTPTR */

 #ifndef _SYS_INT_TYPES_H


 #endif /* _SYS_INT_TYPES_H */

typedef int64_t int_fast64_t;
typedef uint64_t uint_fast64_t;

 #if !defined(_SYS_INT_TYPES_H) && !defined(_INTMAXT)
  #define _INTMAXT
typedef _Longlong intmax_t;
typedef _ULonglong uintmax_t;
 #endif /* !defined(_SYS_INT_TYPES_H) etc. */

		/* LIMIT MACROS */
 #if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)
#define INT8_MIN	(-0x7f - _C2)
#define INT16_MIN	(-0x7fff - _C2)
#define INT32_MIN	(-0x7fffffff - _C2)

#define INT8_MAX	0x7f
#define INT16_MAX	0x7fff
#define INT32_MAX	0x7fffffff
#define UINT8_MAX	0xff
#define UINT16_MAX	0xffff
#define UINT32_MAX	0xffffffff

#define INT_LEAST8_MIN		(-0x7f - _C2)
#define INT_LEAST16_MIN		(-0x7fff - _C2)
#define INT_LEAST32_MIN		(-0x7fffffff - _C2)

#define INT_LEAST8_MAX		0x7f
#define INT_LEAST16_MAX		0x7fff
#define INT_LEAST32_MAX		0x7fffffff
#define UINT_LEAST8_MAX		0xff
#define UINT_LEAST16_MAX	0xffff
#define UINT_LEAST32_MAX	0xffffffff

#define INT_FAST8_MIN		(-0x7fffffff - _C2)
#define INT_FAST16_MIN		(-0x7fffffff - _C2)
#define INT_FAST32_MIN		(-0x7fffffff - _C2)

#define INT_FAST8_MAX		0x7fffffff
#define INT_FAST16_MAX		0x7fffffff
#define INT_FAST32_MAX		0x7fffffff
#define UINT_FAST8_MAX		0xffffffff
#define UINT_FAST16_MAX		0xffffffff
#define UINT_FAST32_MAX		0xffffffff

 #if _INTPTR == 0 && !_ILONG
#define INTPTR_MAX			0x7fff
#define INTPTR_MIN			(-INTPTR_MAX - _C2)
#define UINTPTR_MAX			0xffff

 #elif _INTPTR == 0 && _ILONG
#define INTPTR_MAX			0x7fffffff
#define INTPTR_MIN			(-INTPTR_MAX - _C2)
#define UINTPTR_MAX			0xffffffff

 #elif _INTPTR == 1
#define INTPTR_MAX			0x7fffffffffffffffL
#define INTPTR_MIN			(-INTPTR_MAX - _C2)
#define UINTPTR_MAX			0xffffffffffffffffUL

 #else /* _INTPTR == 2 */
#define INTPTR_MIN			(-_LLONG_MAX - _C2)
#define INTPTR_MAX			_LLONG_MAX
#define UINTPTR_MAX			_ULLONG_MAX
 #endif /* _INTPTR */
 #endif /* !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS) */

 #if !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS)
#define INT8_C(x)	x
#define INT16_C(x)	x
 #if _ILONG
  #define INT32_C(x)	x
 #else
  #define INT32_C(x)	x ## L
 #endif

#define UINT8_C(x)	x ## U
#define UINT16_C(x)	x ## U
 #if _ILONG
  #define UINT32_C(x)	x ## U
 #else
  #define UINT32_C(x)	x ## UL
 #endif
 #endif /* !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS) */

 #if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)
 #if _ILONG
  #define SIG_ATOMIC_MIN	INT32_MIN
  #define SIG_ATOMIC_MAX	INT32_MAX

  #if _INTPTR == 0
  #define PTRDIFF_MIN		INT32_MIN
  #define PTRDIFF_MAX		INT32_MAX

  #define SIZE_MAX			UINT32_MAX
  #elif _INTPTR == 1
  #define PTRDIFF_MIN		INT64_MIN
  #define PTRDIFF_MAX		INT64_MAX

  #define SIZE_MAX			UINT64_MAX
  #endif

 #else /* _ILONG */
  #define PTRDIFF_MIN		INT16_MIN
  #define PTRDIFF_MAX		INT16_MAX

  #define SIG_ATOMIC_MIN	INT16_MIN
  #define SIG_ATOMIC_MAX	INT16_MAX

  #define SIZE_MAX			UINT16_MAX
 #endif /* _ILONG */

#define WCHAR_MIN	_WCMIN
#define WCHAR_MAX	_WCMAX

#define WINT_MIN	_WIMIN
#define WINT_MAX	_WIMAX

#define INT64_MIN	(-0x7fffffffffffffffLL - _C2)
#define INT64_MAX	0x7fffffffffffffffLL
#define UINT64_MAX	0xffffffffffffffffULL

#define INT_LEAST64_MIN		(-0x7fffffffffffffffLL - _C2)
#define INT_LEAST64_MAX		0x7fffffffffffffffLL
#define UINT_LEAST64_MAX	0xffffffffffffffffULL

#define INT_FAST64_MIN		(-0x7fffffffffffffffLL - _C2)
#define INT_FAST64_MAX		0x7fffffffffffffffLL
#define UINT_FAST64_MAX		0xffffffffffffffffULL

#define INTMAX_MIN		(-0x7fffffffffffffffLL - _C2)
#define INTMAX_MAX		0x7fffffffffffffffLL
#define UINTMAX_MAX		0xffffffffffffffffULL
 #endif /* !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS) */

 #if !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS)
#define INT64_C(x)		x ## LL
#define UINT64_C(x)		x ## ULL
#define INTMAX_C(x)		INT64_C(x)
#define UINTMAX_C(x)		UINT64_C(x)
 #endif /* !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS) */
_C_STD_END

#endif /* _STDINT */

 #if defined(_STD_USING)

  #ifdef _STD_USING_INT_TYPES

using _CSTD int_least8_t; using _CSTD int_least16_t;
using _CSTD int_least32_t;  using _CSTD int_least64_t;
using _CSTD uint_least8_t; using _CSTD uint_least16_t;
using _CSTD uint_least32_t; using _CSTD uint_least64_t;

using _CSTD intmax_t; using _CSTD uintmax_t;
  #endif /* _STD_USING_INT_TYPES */

using _CSTD int_fast8_t; using _CSTD int_fast16_t;
using _CSTD int_fast32_t; using _CSTD int_fast64_t;
using _CSTD uint_fast8_t; using _CSTD uint_fast16_t;
using _CSTD uint_fast32_t; using _CSTD uint_fast64_t;
 #endif /* defined(_STD_USING) */

/*
 * Copyright (c) 1992-2003 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V4.02:0216 */
