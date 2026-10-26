#ifdef _MSC_VER_
#pragma once
#endif

#ifndef EDITOR_PROPS_STDAFX_H
#define EDITOR_PROPS_STDAFX_H

//MFC, used only for serialisation helpers.
#define WINVER			0x0501
#define _WIN32_WINNT	0x0501

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// SYSTEM INCLUDES
#include <wx/wxprec.h>
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <windows.h>

#include <string>

// shared container classes
#include <core/core.h>
#include <core/containers.h>
#include <core/maths.h>
#include <core/utmaths.h>
#include <core/utility/utility.h>
#include <core/utility/singleton.h>
#include <core/utility/smartptr.h>
#include <core/utility/Delegate.h>
#include <btl/containers/vector.h>
#include <btl/containers/openhashmap.h>
#include <btl/containers/openhashset.h>
#include <btl/containers/list.h>
#include <btl/utility/string.h>
#include <Btl/Utility/Hashops.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Debug/I_DebugRender.h>
#include <System/Serialise.h>
#include <System/Misc/stringutil.h>

// display library
#include <Display/Core/Includes.h>
#include <Display.h>

#include <Misc_Shared/Factory/Factory.h>

#pragma warning(disable:4355)
#include <xt/xttool.h>

#include <EditorLib/Misc/NodeId.h>
#include <EditorLib/Misc/Definitions.h>
#include <EditorApp/Misc/Definitions.h>
#include <UserNode/Props/Misc/Definitions.h>

#endif
