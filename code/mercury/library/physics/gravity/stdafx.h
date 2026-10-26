
#ifdef _MSC_VER
#pragma once
#endif

#if defined (USER_KSTOREY) || defined(USER_FLU)
#define TEAM_PHYSICS
#endif

#if defined(CFG_ALLOW_OPTIMIZE_OFF) && defined(TEAM_PHYSICS) && !defined(PLATFORM_PS3) && !defined(PLATFORM_SPU)
#define DEBUG_PHYSICS
#endif

#define NOMINMAX

#include <core/core.h>
#include <core/maths.h>
#include <Core/Maths/HVec/New/IncludePhysics.h>
#include <Core/Maths/HVec/New/IncludePhysics.inl>
#include <System/Debug/Debug.h>
//#include <Render/Debug/DebugRender.h>
#include "gravity/sharedphysics/constants.h"
#include <System/Debug/I_DebugRender.h>
#include <System/Scheduler/Jobqueue.h>
#include <System/Perf.h>
//#include "gravity/sharedPhysics/SolverSetting.h"

