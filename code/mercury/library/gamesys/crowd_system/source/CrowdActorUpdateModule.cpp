#include "CrowdSystemPch.h"

#include <crowd_system/CrowdActorUpdateModule.h>
#include <crowd_system/CrowdActorUpdateTask.h>

#include <system/Scheduler.h>
#include <system/Perf.h>

namespace AnimatedCrowd
{

// Module entry point.
void UpdateCachedActors(Scheduler::Task* task)
{
	PERF_EVENT("CrowdActorUpdateTask");
	Spu::MemBaseMarker memBaseMarker;

	C_CrowdActorUpdateTask* pTask = (C_CrowdActorUpdateTask*)Spu::FetchAlloc(*(C_CrowdActorUpdateTask**)task->ParamData, sizeof(C_CrowdActorUpdateTask));
	C_CrowdActorUpdateTask::Run(pTask);
}

}; // namespace AnimatedCrowd

JQ_DECLARE_MODULE(CrowdActorUpdateModule, AnimatedCrowd::UpdateCachedActors, crowd_actor_update_task)
