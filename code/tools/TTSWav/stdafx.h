#pragma once

//C and C++ includes
#include <stdio.h>

#include <atlbase.h>
#include <sapi.h>
#include <sphelper.h>
#include <spuihelp.h>
#include <string>

//Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

//Mercury Core Lib
#include <core/core.h>
#include <core/containers/string.h>
#include <core/containers/list.h>
#include <core/utility/singleton.h>

#include <btl/utility/string.h>

//Mercury System Lib
#include <system/debuglog.h>

//Mercury Tools Shared Lib
#include <tools_shared/ToolsShared.h>

//Include TCLAP
#include <CmdLine.h> 