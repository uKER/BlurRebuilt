#include "stdafx.h"
#include <animation/task/EvaluationTaskModule.h>
#include <animation/task/EvaluationTask.h>
#include <system/Scheduler.h>
#include <system/Perf.h>

namespace Animation
{
	// Module entry point.
	void RunEvaluationTasks(Scheduler::Task* task)
	{
		PERF_EVENT("AnimationEvaluationTask");
		Spu::MemBaseMarker memBaseMarker;
		const C_EvaluationTaskParams* params = (const C_EvaluationTaskParams*)&task->ParamData[0];
		const C_EvaluationTask* tasks = (const C_EvaluationTask*) Spu::FetchAlloc(params->tasks, params->numTasks * sizeof(C_EvaluationTask));
		C_EvaluationTask::RunBatch(tasks, params->numTasks);
	}

}


JQ_DECLARE_MODULE(AnimationEvaluationTaskModule, Animation::RunEvaluationTasks, anim_evaluation_task)

