#ifndef STDAFX_H
#define STDAFX_H

//#define UNICODE
//#define WIN32_LEAN_AND_MEAN

#ifndef PLATFORM_PC
#define PLATFORM_PC
#endif

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
#define GDIPVER			0x0110

#define _USE_INLINING // For tchar.h

#define NO_MERCURY_ASSERT

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <gdiplus.h>
#include <math.h>

#undef small

#pragma warning(disable:4355)	// This used in initialiser list
#pragma warning(disable:4706)	// Assignment within conditional expression
#pragma warning(disable:4200)	// Zero sized array in struct
#pragma warning(disable:4345)	// An object of POD type constructed with an initializer of the form () will be default-initialized
#pragma warning(disable:4512)	// Assignment operator could not be generated
#pragma warning(disable:4100)	// Unreferenced formal parameter
#pragma warning(disable:4238)	// Nonstandard extension used : Class rvalue used as lvalue

#include <System/Debug/Debug.h>
#include <System/Artery/ArterySync.h>

#endif // STDAFX_H