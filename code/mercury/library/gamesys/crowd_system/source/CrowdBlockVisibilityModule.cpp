#include "CrowdSystemPch.h"

#include <crowd_system/CrowdBlockVisibilityModule.h>
#include <crowd_system/CrowdBlockVisibilityTask.h>

#include <system/Scheduler.h>
#include <system/Perf.h>

namespace AnimatedCrowd
{

// Module entry point.
void CalculateVisibleBlocks(Scheduler::Task* task)
{
	PERF_EVENT("CrowdBlockVisibilityTask");
	Spu::MemBaseMarker memBaseMarker;

	C_CrowdBlockVisibilityTask* pTask = (C_CrowdBlockVisibilityTask*)Spu::FetchAlloc(*(C_CrowdBlockVisibilityTask**)task->ParamData, sizeof(C_CrowdBlockVisibilityTask));
	C_CrowdBlockVisibilityTask::Run(pTask);
}

}; // namespace AnimatedCrowd

JQ_DECLARE_MODULE(CrowdBlockVisibilityModule, AnimatedCrowd::CalculateVisibleBlocks, crowd_block_visibility_task)
