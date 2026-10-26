#ifndef GAMESYS_ACTOR_STUDIO_MSG_GAME_PCH_H
#define GAMESYS_ACTOR_STUDIO_MSG_GAME_PCH_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <core/maths.h>

#if defined(PLATFORM_PC) && defined(COMPILER_VC)
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#if defined(PLATFORM_PC)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <winsock2.h>
#include <windows.h>

#include <XInput.h>

#elif defined(PLATFORM_PS3)
#include <cell/atomic.h>
#include <string.h>
#endif

#if !defined( COMPILER_VC )
#include <sys/socket.h>
#endif

// core includes
#include <core/containers.h>

// system includes
#include <system/debuglog.h>
#include <system/serialise.h>

#endif // GAMESYS_ACTOR_STUDIO_MSG_GAME_PCH_H
