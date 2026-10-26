// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>

typedef unsigned __int64 U64;
typedef __int64 S64;

typedef unsigned __int32 U32;
typedef __int32 S32;

typedef unsigned __int16 U16;
typedef __int16 S16;

typedef unsigned __int8 U8;
typedef signed __int8 S8;

typedef char		CHAR8;

typedef double F64;
typedef float F32;

#include <list>
#include <vector>

// TODO: reference additional headers your program requires here
