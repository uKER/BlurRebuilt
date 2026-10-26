#ifndef STDAFX_H
#define STDAFX_H

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <windows.h>
#include <winsock2.h>

#include <core/core.h>
#include <core/containers/list.h>
#include <core/containers/string.h>
#include <core/utility/StringProc.h>
#include <core/maths.h>

#include <system/xml/xml.h>
#include <system/serialise.h>
#include <system/debug/log/FileSystem.h>
#include <system/debuglog.h>
#include <system/shell/ShellExec.h>
#include <system/misc/StringUtil.h>

#include <core/utility/endian.h>

#include <system/serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

extern Debug::Logger& g_Log;

#endif // STDAFX_H
