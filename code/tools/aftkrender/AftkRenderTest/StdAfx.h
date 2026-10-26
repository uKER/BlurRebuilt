#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <gl/wglext.h>

#include <d3d9.h>
#include <d3dx9.h>

#include <d3d10.h>
#include <d3dx10.h>
#include <dxerr.h>

#include <cg/cg.h>
#include <cg/cgd3d9.h>
#include <cg/cggl.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <Display.h>

//#include "Core/Utility/AssertPC.h" // Ensures asserts are thrown
#include <Samples/System/Memory.h> // Overload new and delete
#include <Samples/System/Debug.h> // For g_Log

#include "Aftk/Aftk.h"
#include "AftkRender.h"


#endif // STDAFX_H
