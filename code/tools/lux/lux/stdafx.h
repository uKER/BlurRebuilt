// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _USE_MATH_DEFINES

#include <core/core.h>

const F32 PI = 3.1415926535897932384626433832795f;

#include <cc++/Persist.h>
typedef ost::BaseObject BaseObject;

void MyAssert(char *text, int iLine);

#include <system\debuglog.h>

#include <string>
#include <cmath>
#include <float.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

#include <math.h>
#include <float.h>

#ifdef __GNUC__
#define min(A,B) ((A<B)?A:B)
#define max(A,B) ((A>B)?A:B)
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif

// for xerces - or it will think we're in windows
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMAttr.hpp>

#ifdef __CYGWIN__
#undef WIN32
#endif

#include "lux/SHMaths.h"

#include <core/maths/Constants.h>
#include <core/maths/Storage.h>
#include <old_nightmare_maths/maths.h>
#include <core/maths/MathsUtil.h>
#include <core/UtMaths.h>
#include <core/containers/vector.h>
#include <core/containers/bitvector.h>
#include <system/memory/MemoryTracker.h>

#include <lux_common/LpvsCommonHeaders.h>

#include <Lux/Typedefs.h>

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <Lux/Definitions.h>

#include <tools_debug/DebugManager.h>

#include <lux_common/XercesUtility.h>

#include <old_nightmare_maths/serializevecmat.h>

#include <spatial_hierarchy/SpatialDividerHeaders.h>
#include <Lux/SpatialHierarchy.h>
#include <lux_common/SerialiseUtVectorToOst.h>

#include <Lux/PolyFlags.h>
#include <Lux/Common.h>
#include "Lux/Typedefs.h"

#include <system\memory.h>

#include <core\maths\MathsUtil.h>

void *AllocAligned(size_t size);
void FreeAligned(void *ptr);

// person-specific typedefs (personal.h is not included in source-control, please create your own)
namespace LUX
{
	extern Debug::Logger &g_Log;
};

#define LMAP_UNUSED_SPACE_COLOUR 0x00ff00ff
