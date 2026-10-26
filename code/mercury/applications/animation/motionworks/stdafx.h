#ifndef _stdwx_h_
#define _stdwx_h_

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// SYSTEM INCLUDES

// Include math.h to stop it being pulled in later and generating warnings when it redefines M_PI 
#define _USE_MATH_DEFINES
#include <math.h>




#include <wx/wxprec.h>
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#ifdef __VISUALC__
#   pragma warning(disable:4100)  
#endif

// for artery
#include <winsock2.h>

// for xtwin

/*
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
//#include <assert.h>
*/

#define min(x,y) (x < y) ? x : y
#define max(x,y) (x > y) ? x : y

#include <gdiplus.h>


// shared container classes
#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers/Vector.h>
#include <Core/Containers/List.h>
#include <btl/containers/vector.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

// Perf.
#include <system/perf.h>

// display library
#include <Display/Core/Includes.h>
#include <Display.h>

// string
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

// AFTK!
#include <Aftk/Aftk.h>

// Xt
#include <xt/XtTool.h>

// APPLICATION INCLUDES
#endif
