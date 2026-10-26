#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif


#ifdef BUILD_CONFIG_SPEED
#define assert(x)
#endif

#define MATHS_DONT_INITIALISE_TO_ZERO

#include <string.h>
#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>

#endif // STDAFX_H
