#pragma once

#include <fstream>
#include <cctype>
#include <cassert>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <algorithm>
#include <windows.h>

#include <core/core.h>
#include <Core/Maths.h>
#include <aftk_translator_plugin/StringTable.h>
#include <aftk_translator_plugin/Definitions.h>
#include <aftk/aftk.h>

// disable warning C4995: '_OLD_IOSTREAMS_ARE_DEPRECATED': name was marked as #pragma deprecated
#pragma warning(disable : 4995)

#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MDagPath.h>
#include <maya/MStringArray.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnCompoundAttribute.h>

#include <aftk_translator_plugin/EmailReport.h>

#include <string>

#include <Btl/Utility/string.h>
#include <System/Serialise.h>

extern char g_versionString[512];
#define AFTK_MAYA_SUPPORT_EMAIL ""

//Plug-in headers
#include <maya/MProgressWindow.h>

#include <system/debug/log/debuglog.h>

namespace AftkMaya
{
	extern Debug::Logger& g_mainLogger;
}

#include <aftk_translator_plugin/Gui.h>
#include <aftk_translator_plugin/GuiProgressCallback.h>
#include <aftk_translator_plugin/AftkTranslatorPlugin.h>

