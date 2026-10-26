#pragma once

#define _CRT_SECURE_NO_WARNINGS

//C and C++ includes
#include <stdio.h>
#include <string>

//Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

//Mercury Core Lib
#include <core/core.h>
#include <core/containers/string.h>
#include <core/containers/list.h>

//Mercury System Lib
#include <system/debuglog.h>

//Mercury Tools Shared Lib
#include <tools_shared/ToolsShared.h>

//Include TCLAP
#include <CmdLine.h> 

#include "source/Options.h"
