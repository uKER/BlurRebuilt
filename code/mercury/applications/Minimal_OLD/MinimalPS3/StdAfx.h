#ifndef STDAFX_H
#define STDAFX_H

#pragma once

#include <Core/Core.h>
#include <core/containers/string.h>

#ifndef PLATFORM_SPU

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>

#include <cell/pad.h>

#include <Display/Core/Includes.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <Display.h>
#include <Render.h>

#include "Samples/System/Debug.h"

#endif

#endif // STDAFX_H
