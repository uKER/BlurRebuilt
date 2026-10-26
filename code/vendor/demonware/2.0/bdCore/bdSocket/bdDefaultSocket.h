// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DEFAULT_SOCKET_H
#define BD_DEFAULT_SOCKET_H

#include <bdPlatform/bdPlatformConfig/bdPlatformConfig.h>

#ifdef BD_MULTITHREADED_SOCKET

#if defined BD_PLATFORM_UNIX || defined BD_PLATFORM_PS3
// cf RT#6980 & trac#24
BD_COMPILE_ASSERT(false, The_BD_MULTITHREADED_SOCKET_define_is_not_available_on_this_platform);
#endif

#include <bdCore/bdSocket/bdThreadedSocket.h>
typedef bdThreadedSocket bdDefaultSocket;

#else 

#include <bdCore/bdSocket/bdSocket.h>
typedef bdSocket bdDefaultSocket;

#endif // BD_MULTITHREADED_SOCKET

#endif // BD_DEFAULT_SOCKET_H
