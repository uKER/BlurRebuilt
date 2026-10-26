#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma warning (disable:4996)

#define _WIN32_WINNT 0x501
#define WIN32_LEAN_AND_MEAN

//#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <Core/Core.h>
//#include <Display/Core/Includes.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <Xt/XtTool.h>
//#include <Display.h>
//#include <Render.h>

//#include "Samples/System/Memory.h"
//#include "Samples/System/Debug.h"

#endif // STDAFX_H
