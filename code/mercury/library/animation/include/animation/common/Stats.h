#ifndef ANIMATION_COMMON_STATS_H
#define ANIMATION_COMMON_STATS_H

#if !defined(CFG_GOLD) && !defined(PLATFORM_SPU)
#define ANIMATION_STATS_ENABLED
#define ANIMATION_STATS_UPDATE(x)	::Animation::gStats.x
#else
#define ANIMATION_STATS_UPDATE(x)
#endif

namespace Animation
{
#ifdef ANIMATION_STATS_ENABLED

	class C_Stats
	{
	public:
		C_Stats();

		// C_Pose stats.
		U32		numPoses;
		U32		memoryUsedByPoseData;

		// C_Cycle stats.
		U32		numCycles;
		U32		memoryUsedByCycles;

		// Evaluation task stats.
		U32 	numImmediateTasks;
		U32 	numThreadedTasks;
		U32 	numFlushes;
		U32 	memoryUsedByTaskManager;
		U32 	maxThreadedTasks;
		U32		numBatches;

		// C_CycleMapping stats.
		U32		numCachedCycleMappings;			// Total with zero refcounts.
		U32		numActiveCycleMappings;			// Total with non-zero ref counts.
		U32		numInvalidCycleMappings;		// Total without valid cycle or pose layout pointers.
		U32		memoryUsedByCycleMappings;		// Total memory used by all allocated cycle mappings.
		U32		cycleMappingCacheMemoryUsed;	// Memory used by cache.
		U32		cycleMappingCacheMemoryLimit;	// Max. memory that can be used by cache.
	};

	extern C_Stats		gStats;

#endif
}

#endif
