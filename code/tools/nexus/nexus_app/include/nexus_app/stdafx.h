#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _AFXDLL 

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

//Include additional useful MFC components
#include <mfc_extras/HistoryEdit.h>
#include <mfc_extras/ColorEdit.h>
#include <mfc_extras/TreeCtrlEx.h>
#include <mfc_extras/NumEdit.h>
#include <mfc_extras/NumSpinCtrl.h>

#include <D3DX9.h>
#include <dxerr.h>

typedef IDirect3D9 D3D;
typedef IDirect3DDevice9 D3DDevice;
typedef IDirect3DSurface9 D3DSurface;
typedef IDirect3DVertexBuffer9 D3DVertexBuffer;

#include <core/Core.h>
#include <core/containers.h>
#include <core/containers/OpenHashTable.h>
#include <core/Maths.h>
#include <core/utility/HashOps.h>
#include <tools_common/AlignedAlloc.h>

#include <minmax.h>
#include <Prof-UIS.h> // Prof-UIS library
#undef min
#undef max

#include <Aftk/Aftk.h>

#include <tools_common/Status.h>
#include <tools_common/Bounds.h>
#include <tools_common/MathsConversion.h>

#include "resource.h"

#include <nexus_core/Headers.h>


#include <nexus_render/RenderLib.h>

#include <tools_common/Status.h>
#include <tools_debug/DebugManager.h>
#include <core/utility/StringProc.h>
#include <tools_common/FileUtilities.h>

#include <nexus_app/Definitions.h>
#include <nexus_app/ApplicationState.h>

namespace Nexus
{
	extern Tu::DebugManager g_nexusDebugManager;
	class MainFrame;
	class C_ViewportParent;
	class NexusDoc;

	extern MainFrame*			g_pMainFrame;
	extern C_ViewportParent*	g_pViewportParent;
	extern NexusDoc*			g_pDocument;

	class NexusSystem;
	extern NexusSystem*			g_pNexusSystem;

	#include <tools_common/List.h>
}

#include <nexus_app/Overlay2D.h>
#include <nexus_app/ViewportDisplay.h>
#include <nexus_app/Viewport.h>

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

