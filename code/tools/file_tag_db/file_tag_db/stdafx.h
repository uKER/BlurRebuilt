#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500 

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <windows.h>

//Mercury Core lib
#include <core/core.h>
#include <core/containers.h>
#include <core/utility/Singleton.h>

//Mercury System lib
#include <system/xml/Xml.h>
#include <system/serialise.h>


