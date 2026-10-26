// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

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

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <core/core.h>

const float PI = 3.1415926535897932384626433832795f;
#define NM_MATHS_PROJECT_NAMESPACE VecMat

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <string>
#include <vector>
#include <io.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <float.h>

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>


#include <D3DX9.h>
#include <dxerr.h>

typedef IDirect3D9 D3D;
typedef IDirect3DDevice9 D3DDevice;
typedef IDirect3DSurface9 D3DSurface;
typedef IDirect3DVertexBuffer9 D3DVertexBuffer;

#include <Nightmare_Maths/maths.h>

using VecMat::RGBA;

using VecMat::HVecX;
using VecMat::HVecY;
using VecMat::HVecZ;
using VecMat::HVecW;

using VecMat::HVec2;
using VecMat::HVec3;
using VecMat::HVec4;

using VecMat::RotX;
using VecMat::RotY;
using VecMat::RotZ;

using VecMat::AxisX;
using VecMat::AxisY;
using VecMat::AxisZ;

using VecMat::HMat33;
using VecMat::HMat43;
using VecMat::HMat44;

using VecMat::Quaternion;

#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/maths/utmaths/mmatrix.h>
#include <core/maths/utmaths/colour.h>

#include <Aftk/AftkHeaders.h>

#include <Tools_Common/Bounds.h>
#include <Tools_Common/MathsConversion.h>

#include <Prof-UIS.h> // Prof-UIS library

//Include additional useful MFC components
#include <mfc_extras/HistoryEdit.h>
#include <mfc_extras/ColorEdit.h>

#include <nexus_render/RenderLib.h>
//#include <Spatial Divider/SpatialDivider.h>

#include <Tools_Common/Status.h>
#include <Tools_Debug/DebugManager.h>

#include <vis_app/Definitions.h>

extern Tu::DebugManager g_nexusDebugManager;

class MainFrame;
class NexusView;

extern MainFrame* g_pMainFrame;
extern NexusView* g_pView;

namespace Nexus
{
	class NexusSystem;
	class NexusRender;
	extern NexusSystem* g_pNexusSystem;
	extern NexusRender* g_pNexusRender;
}

const int UPDATE_RENDER_TIME = WM_APP + 1;

#define DO_BEGIN { g_nexusDebugManager 
#define DO_ERROR_END << " " << __FILE__ << " " << __LINE__; g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear();}
#define DO_ERROR_RETURN_END << " " __FILE__ << " " << __LINE__; GuiDisplayError(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear(); return MS::kFailure; }
#define DO_ERROR_CLEAN_UP_RETURN_END << " " __FILE__ << " " << __LINE__; GuiDisplayError(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear(); CleanUpAfterFailure(); return MS::kFailure; }

#define DO_WARNING_END << " " __FILE__ << " " << __LINE__; GuiDisplayWarning(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear();}

#define DO_MESSAGE_END ; GuiDisplayMessage(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n";  g_nexusDebugManager.Clear();}

#define CHECK_BEGIN if (!status) { g_nexusDebugManager 
#define CHECK_ERROR_CLEAN_UP_RETURN_END << " " << __FILE__ << " " << __LINE__; GuiDisplayError(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear(); CleanUpAfterFailure(); return status; }
#define CHECK_ERROR_RETURN_END << " " << __FILE__ << " " << __LINE__; GuiDisplayError(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear(); return status; }

#define CHECK_WARNING_RETURN_END << " " << __FILE__ << " " << __LINE__; GuiDisplayWarning(g_nexusDebugManager.GetText()); g_nexusDebugManager << "\n"; g_nexusDebugManager.Clear(); return status; }

#define NEXUS_IDLE_PROCESS


#define Assert(X) { if (!(X)) assert(X); }
#define Mem_Alloc(X) malloc(X)
#define Mem_Free(X) free(X)
#define Mem_Realloc(X,Y) realloc(X,Y)

#undef Assert

#include "CServ/ClusterServHeaders.h"
