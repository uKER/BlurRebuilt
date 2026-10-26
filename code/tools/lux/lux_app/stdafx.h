// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define _CRT_SECURE_NO_DEPRECATE

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxcview.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <core/core.h>

const F32 PI = 3.1415926535897932384626433832795f;

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <stdio.h>
#include <assert.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

#include <windows.h>
#include <mmsystem.h>

#define DIRECTINPUT_VERSION 0x0800
#include <D3DX9.h>
#include <dxerr.h>
#include <dinput.h>

#include <cc++/persist.h>

#include <system\debuglog.h>

#include <core/maths/Constants.h>
#include <core/maths/Storage.h>
#include <old_nightmare_maths/maths.h>
#include <core/maths/MathsUtil.h>
#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/containers/bitvector.h>

#include <lux_Common/LpvsCommonHeaders.h>

#include <Aftk/Aftk.h>

#include <scene_renderer/SceneRendererHeaders.h>
#include <spatial_hierarchy/spatialDividerHeaders.h>
#include <Lux/SpatialHierarchy.h>
#include <lux/LuxHeaders.h>
#include <lux_common/MultiProgressNotification.h>
#include <Lux/Core.h>
#include <lux_App/LpvsCore.h>

#include <lux_App/Definitions.h>

#include <Prof-UIS.h> // Prof-UIS library

typedef ost::BaseObject BaseObject;

#pragma warning(disable:4244)

#include <winsock2.h>
#include "winsock2.h"
#include "Ws2tcpip.h"

#define SIN_ADDR sin_addr.S_un.S_addr
