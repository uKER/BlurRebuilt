// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <core/core.h>
//#include "tools_common/SystemDefines.h"

const F32 PI = 3.1415926535897932384626433832795f;

#include "cc++/Persist.h"
typedef ost::BaseObject BaseObject;

#include <string>
#include <cmath>
#include <float.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#ifdef __GNUC__
#define min(A,B) ((A<B)?A:B)
#define max(A,B) ((A>B)?A:B)
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif

#include <core/maths/Constants.h>
#include <core/maths/Storage.h>
#include <old_nightmare_maths/maths.h>
#include <core/UtMaths.h>

#include <lux_common/LpvsCommonHeaders.h>

#include <Lux/Typedefs.h>

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <Lux/Definitions.h>

#include <tools_debug/DebugManager.h>

#include <old_nightmare_maths/serializevecmat.h>

#include <core/containers/vector.h>
#include <core/containers/bitvector.h>
#include <spatial_hierarchy/spatialDividerHeaders.h>
#include <Lux/SpatialHierarchy.h>

#include <Lux/PolyFlags.h>
#include <Lux/Common.h>
#include "Lux/Typedefs.h"
#include "Lux/Lux.h"

// person-specific typedefs (personal.h is not included in source-control, please create your own)


#define LMAP_UNUSED_SPACE_COLOUR 0x00ff00ff

inline Tu::DebugManager& operator <<(Tu::DebugManager &mgr, const float3 &value)
{
	mgr << value.x << " " << value.y << " " << value.z;

	return mgr;
}
