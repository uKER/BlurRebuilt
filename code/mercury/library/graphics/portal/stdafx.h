#ifndef STDAFX_H
#define STDAFX_H

#include <string.h>

#ifdef _MSC_VER
#pragma once
#endif

#if defined (PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <btl/Containers/Vector.h>

#ifndef PLATFORM_SPU
#include <btl/Utility/String.h>	// Just say "NO" to strings!
#endif

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <system\Debug\I_DebugRender.h>
#include <system/serialise.h>

#endif // STDAFX_H
