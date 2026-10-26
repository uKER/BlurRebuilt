#ifndef LOGICALFS_PCH
#define LOGICALFS_PCH

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <core/containers/vector.h>
#include <btl/containers/vector.h>
#include <core/containers/bitvector.h>
#include <core/utility/singleton.h>


#ifdef PLATFORM_PC
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#endif

#include <system/serialise.h>
#include <system/misc/StringUtil.h>
#include <system/serialise/TextSerialise.h>
#include <system/DebugLog.h>

namespace LogicalFs
{
	extern Debug::Logger &g_log;
}

#endif
