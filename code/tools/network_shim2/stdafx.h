#pragma once

//C and C++ includes
#include <stdio.h>
#include <tchar.h>
#include <string>

#define WINVER		 0x0501
#define _WIN32_WINNT 0x0501 

#include <winsock2.h>
#include <windows.h>

//Mercury Core lib
#include <core/core.h>
#include <core/containers/list.h>
#include <core/containers/string.h>
#include <core/containers/openhashtable.h>
#include <core/utility/StringProc.h>
#include <core/utility/Singleton.h>

//Mercury System lib
#include <system/xml/xml.h>
#include <system/serialise.h>
#include <system/debug/log/FileSystem.h>
#include <system/debug/log/DebugLog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <system/shell/ShellExec.h>
#include <system/misc/StringUtil.h>

//BTL
#include <Btl/Utility/string.h>
#include "btl/containers/map.h"
#include <btl/containers/openhashmap.h>
#include <Btl/Containers/MultiMap.h>

//Include TCLAP
#include <CmdLine.h> 

#include <tools_shared/executer/executer.h>
#include <tools_shared/shim/shim.h>
#include <tools_shared/utility/MacroExpander.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include <Definitions.h>
#include <CommandLineOptions.h>
#include <Options.h>
#include <Utility.h>


