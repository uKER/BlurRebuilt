#pragma once

//C and C++ includes
#include <stdio.h>
#include <tchar.h>
#include <string>

//Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

//Mercury Core Lib
#include <core/core.h>
#include <core/containers/list.h>
#include <core/containers/string.h>
#include <core/utility/StringProc.h>
#include <core/utility/endian.h>
#include <core/utility/singleton.h>
#include <core/maths.h>

//Mercury System Lib
#include <system/xml/xml.h>
#include <system/serialise.h>
#include <system/debug/log/FileSystem.h>
#include <system/debuglog.h>
#include <system/shell/ShellExec.h>
#include <system/misc/StringUtil.h>

#include <system/serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

//Mercury Tools Shared Lib
#include <tools_shared/ToolsShared.h>
#include <tools_shared/file_copy/FileCopy.h>
#include <tools_shared/Utility/MacroExpander.h>
#include <tools_shared/executer/Executer.h>
#include <tools_shared/shim/Shim.h>

//Include TCLAP
#include <CmdLine.h> 

#include "source/Options.h"

