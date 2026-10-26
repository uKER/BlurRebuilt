#ifndef XT_IMPL_XTCORETYPES_H
#define XT_IMPL_XTCORETYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PC)

	typedef signed __int8		XtI8;
	typedef signed __int16		XtI16;
	typedef signed __int32		XtI32;
	typedef signed __int64		XtI64;

	typedef unsigned __int8		XtU8;
	typedef unsigned __int16	XtU16;
	typedef unsigned __int32	XtU32;
	typedef unsigned __int64	XtU64;

	typedef float				XtF32;
	typedef double				XtF64;

	typedef intptr_t			XtIntPtr;
	typedef XtU32				XtId;

#elif defined(PLATFORM_PS3) || defined(PLATFORM_LINUX) || defined(PLATFORM_SPU)

	typedef signed char			XtI8;
	typedef signed short		XtI16;
	typedef signed int			XtI32;
	typedef signed long long	XtI64;

	typedef unsigned char		XtU8;
	typedef unsigned short		XtU16;
	typedef unsigned int		XtU32;
	typedef unsigned long long	XtU64;

	typedef float				XtF32;
	typedef double				XtF64;

	typedef intptr_t			XtIntPtr;
	typedef XtU32				XtId;

#else
#error "No platform defined for Xt library"
#endif 

#endif // XT_IMPL_XTCORETYPES_H
