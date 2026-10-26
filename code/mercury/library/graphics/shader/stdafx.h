#ifndef SHADER_STDAFX_H
#define SHADER_STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_SPU

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

#include <Core/Core.h>
#include <Btl/Containers/Vector.h>
#include <Core/Containers.h>
#include <Core/Maths.h>
#include <Display.h>

#endif // SHADER_STDAFX_H
