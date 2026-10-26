// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <string.h>

#include <core/Core.h>
#include <core/containers.h>
#include <core/Maths.h>
#include <core/utility/HashOps.h>

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <string>
#include <vector>
#include <io.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <float.h>

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include <Aftk/Aftk.h>

namespace Nexus
{
	class Core;
	class Progress;
	extern Core* g_pCore;
	extern Progress* g_pProgressCallback;

	extern const char* g_aftkAttrTable[Aftk::E_GenericAttributeType_Count];
}

#include <tools_common/Status.h>
#include <tools_common/Bounds.h>
#include <tools_common/MathsConversion.h>
#include <tools_common/AlignedAlloc.h>
#include <tools_debug/DebugManager.h>

#include <nexus_core/StringTable.h>
#include <nexus_core/Definitions.h>
#include <nexus_core/Progress.h>

#include <nexus_core/CSGenTasks.h>

#include <nexus_core/GlobalTasks.h>
#include <nexus_core/AftkTasks.h>
#include <nexus_core/AftkUtTasks.h>
#ifdef NEXUS_MAYA
#include <nexus_core/MayaTasks.h>
#endif
#include <lua.h>
#include <lauxlib.h>
#include <tex_atlas/TexAtlas.h>
#include <nexus_core/LuaUtility.h>
#include <nexus_core/PickResult.h>
#include <nexus_core/SelectedNodeInfo.h>
