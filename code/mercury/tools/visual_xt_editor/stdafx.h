#ifndef _stdwx_h_
#define _stdwx_h_

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <Core/Core.h>
#include <Core/Maths.h>

// SYSTEM INCLUDES
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

// shared container classes
#include <Display/Core/Includes.h>
#include <Core/Utility/Utility.h>
#include <Core/Containers/Vector.h>
#include <Core/Containers/List.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

// display library
#include <Display.h>

// string
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

// APPLICATION INCLUDES
#endif
