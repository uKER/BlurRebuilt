#ifndef LUASHARED_MISCSHAREDPCH_H
#define LUASHARED_MISCSHAREDPCH_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PC)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 

#include <windows.h>

#elif defined(PLATFORM_PS3)

#endif

#if !defined( COMPILER_VC )
#endif

// core includes
#include <core/core.h>
#include <core/maths.h>
#include <core/containers.h>

// lua includes
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#endif // MISCSHARED_MISCSHAREDPCH_H
