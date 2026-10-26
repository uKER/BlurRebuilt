#ifndef RENDER_STDAFX_H
#define RENDER_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma warning(disable:4146)

#if PLATFORM_SPU

#elif PLATFORM_PS3

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#define _stricmp strcasecmp

#else

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#ifndef PLATFORM_SPU
#include <Display/Core/Includes.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>
#include <Core/Utility/Utility.h>
#include <Core/Containers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

#include <Display.h>

#include "Render/Utils/DebugLog.h"

#endif // RENDER_STDAFX_H
