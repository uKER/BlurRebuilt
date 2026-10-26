// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <iostream>
#include <tchar.h>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
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

#include <cassert>
#include <string>
#include <sstream>
#include <math.h>
