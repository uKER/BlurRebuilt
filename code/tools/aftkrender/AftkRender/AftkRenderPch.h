#ifndef AFTK_RENDER_PCH_INCLUDE
#define AFTK_RENDER_PCH_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#include <d3d10.h>
#include <d3dx10.h>
#include <dxerr.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <core/containers/vector.h>
#include <core/containers/bitvector.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <system/Debug/Log/DebugLog.h>
#include <System/Memory/MemoryTracker.h>

#include <Display.h>

#include "Misc/Definitions.h"

#include "Aftk/Aftk.h"

#ifdef _DEBUG
#define ASSERT(x) if (!(x)) _asm int 3
#else
#define ASSERT(x) assert(x)
#endif

#endif // STDAFX_H
