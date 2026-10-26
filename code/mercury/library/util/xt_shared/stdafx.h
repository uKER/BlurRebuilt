#ifdef _MSC_VER_
#pragma once
#endif

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
#define GDIPVER			0x0110

#define _USE_INLINING // For tchar.h

#include <windows.h>
#include <string.h>


// shared container classes
#include <core/core.h>
#include <Btl/Containers/openhashset.h>

#pragma warning(push)
#pragma warning(disable:4355)	// This used in initialiser list
#pragma warning(disable:4706)	// Assignment within conditional expression
#pragma warning(disable:4200)	// Zero sized array in struct
#include <Xt/XtTool.h>
#pragma warning(pop)
