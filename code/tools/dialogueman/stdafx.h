#ifndef DMAN_STDAFX_H
#define DMAN_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#define _CRTDBG_MAP_ALLOC

// dont include winsock since artery needs ws2
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

//Windows stuff
#include <windows.h>
#include <gdiplus.h>

//TTS
#include <sapi.h>
#include <sphelper.h>
#include <spuihelp.h>


#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/treectrl.h"
#include "wx/grid.h"
#include "wx/wxhtml.h"
#include "wx/wizard.h"
#include "wx/choice.h"
#include "wx/listbox.h"
#include "wx/statline.h"
#include "wx/filepicker.h"
#include "wx/aui/aui.h"
#include "wx/evtloop.h"
#include "wx/file.h"

#pragma warning(disable:4100)  

// shared container classes
#include <Core/core.h>
#include <Core/maths.h>
#include <Core/Utility/Utility.h>
#include <Core/Utility/Singleton.h>
#include <Core/Utility/Delegate.h>
#include <Btl/Containers/vector.h>
#include <Btl/Containers/openhashmap.h>
#include <Btl/Containers/chainedhashmap.h>
#include <Btl/Containers/openhashset.h>
#include <Btl/Containers/list.h>
#include <Btl/Containers/map.h>
#include <Btl/Utility/string.h>
#include <Btl/Utility/hashops.h>

// logging and file access
#include <System/DebugLog.h>
#include <System/Debug/Log/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Debug/I_DebugRender.h>
#include <System/Serialise.h>

#include <Tools_Shared/ToolsApplication.h>

#include <xt/xttool.h>

#include <Xt_Shared/XtObject.h>
#include <Xt_Shared/XtObjectImpl.h>

// std
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>

#include <sigslot/sigslot.h>

//Excel
#include <BasicExcel/BasicExcel.h>

// APPLICATION INCLUDES
#include <DialogueMan/Misc/Definitions.h>
#include <DialogueMan/Misc/TTSStructures.h>
#include <DialogueMan/Misc/MutantValue.h>
#include <DialogueMan/Misc/State.h>
#include <DialogueMan/ImportExport/Reader.h>
#include <DialogueMan/Config.h>
#include <DialogueMan/App.h>
//#include <svninterface.h>

namespace DMan
{
	extern Debug::Logger& g_Log;
}

#endif //DMAN_STDAFX_H
