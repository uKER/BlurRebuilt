#ifndef SAMPLES_STDAFX_H
#define SAMPLES_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma warning(disable:4146)
#pragma warning(disable:4530)

#if defined (PLATFORM_PS3)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#include <cell/pad.h>

#define _stricmp strcasecmp

#else

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#endif

#include <Display/Core/Includes.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <Display.h>

#include "Samples/System/Debug.h"

#endif // DISPLAY_STDAFX_H
