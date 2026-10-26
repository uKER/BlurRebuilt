#ifndef __NETWORK_SYSTEM_HEADERS_H__
#define __NETWORK_SYSTEM_HEADERS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>

#if defined (PLATFORM_PS3)
	#define MAX_FRIENDS (100)

#elif defined (PLATFORM_PC)
	#define MAX_FRIENDS (100) 

#endif

/// maximum number of characters including the terminator that can be used in per user storage file names.
const U32 MAX_PER_USER_FILE_NAME	=	(32);

/// defines how many players we can add / remove in 1 api call within the C_Session class
const U32 NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION	 = (20);

#endif

