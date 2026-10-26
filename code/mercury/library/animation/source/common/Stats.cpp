#include "Stdafx.h"
#include <animation/common/Stats.h>

#ifdef ANIMATION_STATS_ENABLED

namespace Animation
{

C_Stats	gStats;

C_Stats::C_Stats():
	numPoses(0),
	memoryUsedByPoseData(0),
	numCycles(0),
	memoryUsedByCycles(0),
	numImmediateTasks(0),
	numThreadedTasks(0),
	numFlushes(0),
	memoryUsedByTaskManager(0),
	maxThreadedTasks(0),
	numBatches(0),
	numCachedCycleMappings(0),
	numActiveCycleMappings(0),
	numInvalidCycleMappings(0),
	memoryUsedByCycleMappings(0),
	cycleMappingCacheMemoryUsed(0),
	cycleMappingCacheMemoryLimit(0)
{
}

} // end Animation

#endif // ANIMATION_STATS_ENABLED