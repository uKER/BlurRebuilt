#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define WIN32_LEAN_AND_MEAN

#include <core/core.h>

#include <stdio.h>
#include <tchar.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#include <atlbase.h>
#include <atlstr.h>

//#include <win32/vs2005/d3d9.h>
//#include <win32/vs2005/xgraphics.h>
//#include <win32/vs2005/xcompress.h>

