#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <core/core.h>
#include <core/maths.h>

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
#include <float.h>

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include <Aftk/Aftk.h>

#include <tools_common/Bounds.h>
#include <tools_common/MathsConversion.h>

#include <tools_common/Status.h>
#include <tools_debug/DebugManager.h>

#include <nexus_cmd/Definitions.h>
#include <nexus_cmd/CommandLineOptions.h>

#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.h>

#include <nexus_core/Headers.h>

//Disable warning C4005: 'M_PI' : macro redefinition
#ifdef NEXUS_MAYA
#include <maya/MStatus.h>
#include <maya/MFileIO.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#pragma warning(push)
	#pragma warning(disable : 4005)
	#include <maya/MLibrary.h>
	#include <maya/MString.h>
#pragma warning(pop)
#endif

#include <winsock2.h>
#include <nexus_cmd/ConsoleProgress.h>

namespace NexusCmd
{
	extern Tu::DebugManager* g_pDebugManager;
	extern Tu::DebugManager* g_pOutputManager;
}

