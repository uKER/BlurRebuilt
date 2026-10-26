// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

// TODO: reference additional headers your program requires here

#define MATHS_DONT_INITIALISE_TO_ZERO

#include "Core/Core.h"
#include "Core/Maths.h"
//#include "Core/Utility/Utility.h"

#include "System\DebugLog.h"
#include "system\perf\perfevent.h"

#include "System/DebugLog.h"
#include "System/Debug/Debug.h"
#include "system/scheduler/jobqueue.h"

#ifndef PLATFORM_SPU 

#include "System/Serialise.h"
#include "Display/Core/Includes.h"
#include "Display/Device/DeviceDefines.h"
#include "Display/Utils/DebugLog.h"
#include "Display/Utils/Globals.h"
#include "Display/Utils/Perf.h"
#include "Display/Device/Device.h"
#include "Display/Manager/Manager.h"

#endif

//#define DEBUG_RACING_PHYSICS
//#define MULTI_THREAD_VEHICLE_PHYSICS

#include "racingphysics/physics/geometry.h"

using Maths::Min;
using Maths::Max;
using Maths::Clamped;
using Maths::ClampedAbs;
using Maths::Lerp;
using Maths::Abs;
using Maths::Sqrt;
using Maths::Sign;
using Maths::AddSmooth;
using Maths::Constants;

namespace RacingPhysics
{
	void PhysicsError();

#ifndef PLATFORM_SPU 
	extern Debug::Logger& g_RacingPhysicsLog;
#endif
}


