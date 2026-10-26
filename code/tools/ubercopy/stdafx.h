#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

//Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500 
#include <windows.h>
#include <winsock2.h>

#include <core/core.h>

#if defined(PLATFORM_PC) && defined(COMPILER_VC)
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#if !defined( COMPILER_VC )
#include <sys/socket.h>
#endif

// core includes
#include <core/containers.h>

// system includes
#include <system/debuglog.h>
#include <system/misc/StringUtil.h>

// tools_shared includes
#include <tools_shared/ToolsShared.h>
#include <tools_shared/file_copy/FileCopy.h>

#include <tools_shared/file_copy/FileCopyUtils.h>
#include <tools_shared/file_copy/FileCopyList.h>
#include <tools_shared/file_copy/FileCopyListXml.h>

extern Debug::Logger& g_Log;

#endif // STDAFX_H
