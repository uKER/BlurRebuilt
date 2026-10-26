#ifndef _NETWORK_STDAFX_H
#define _NETWORK_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <System/DebugLog.h>
#include <core/containers/string.h>
#include <System/Debug/Debug.h>
#include <Network/Network.h>
#include <Network/SystemHeaders.h>
#include <wchar.h>
#include <bdNet/bdNet.h>

#if defined (PLATFORM_PS3)
// PS3 system header files
#include <stdio.h>
#include <stdlib.h>
#include <netex/net.h>
#include <netex/libnetctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <np.h>
#include <sysutil\sysutil_common.h>
#include <sysutil\sysutil_sysparam.h>
#include <cell\l10n.h>

#elif defined (PLATFORM_PC)
// PC system header files
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <WinSock2.h>
#include <windows.h>
#pragma comment (lib, "Iphlpapi.lib")
#else
#endif

#include <btl/containers/vector.h>
#include <btl/containers/list.h>
#include <btl/Allocators/pool.h>

#endif  //_NETWORK_STDAFX_H
