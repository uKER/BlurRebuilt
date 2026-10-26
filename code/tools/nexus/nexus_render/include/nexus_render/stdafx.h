#pragma once

//#define LOAD_SHADERS_FROM_FILE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <string.h>

#include <core/core.h>
#include <core/containers.h>

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <string>
#include <vector>
#include <io.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <float.h>
#include <cctype>
#include <algorithm>
#include <windows.h>
#include <mmsystem.h>

#include <D3DX9.h>

#include <core/Maths.h>
#include <core/utility/HashOps.h>

#include <dxerr.h>

typedef IDirect3D9 D3D;
typedef IDirect3DDevice9 D3DDevice;
typedef IDirect3DSurface9 D3DSurface;
typedef IDirect3DVertexBuffer9 D3DVertexBuffer;

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef _DEBUG
#define ASSERT(x) if (!(x)) _asm int 3
#else
#define ASSERT(x) (assert(x))
#endif


#include <Aftk/Aftk.h>
#include <attk/attk.h>

#include <tools_common/Bounds.h>

#include <tools_common/MathsConversion.h>
#include <tools_common/MathsUtility.h>

#include <tools_common/Status.h>
#include <tools_common/Bounds.h>
#include <tools_common/AlignedAlloc.h>

//Include master header for render library.
#include <nexus_render/RenderLib.h>

#include <tools_debug/DebugManager.h>

namespace NexusRender
{
	extern Tu::DebugManager		g_debugManager;
}



