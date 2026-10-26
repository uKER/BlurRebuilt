#include "VehiclePCH.h"
#include "Vehicle/Utils/DebugLog.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

Debug::Logger& g_Log = Debug::GetLogger("mercury.vehicle");

} // namespace Vehicle
