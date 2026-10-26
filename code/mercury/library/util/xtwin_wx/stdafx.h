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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// shared container classes
#include <core/core.h>
#include <btl/containers/vector.h>
#include <btl/containers/map.h>
#include <btl/utility/string.h>

#include <windows.h>
#include <string.h>

#pragma warning(disable:4355)	// This used in initialiser list
#pragma warning(disable:4706)	// Assignment within conditional expression
#pragma warning(disable:4200)	// Zero sized array in struct

#include <Xt/XtTool.h>