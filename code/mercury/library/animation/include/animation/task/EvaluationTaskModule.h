#ifndef ANIMATION_EVALUATIONTASKMODULE_H
#define ANIMATION_EVALUATIONTASKMODULE_H

namespace Scheduler
{
	struct jqModule;
}

extern Scheduler::jqModule AnimationEvaluationTaskModule;

namespace Animation
{
	class C_EvaluationTask;

	//
	// Evaluation task module parameters.
	//
	struct C_EvaluationTaskParams
	{
		const C_EvaluationTask*		tasks;
		U32							numTasks;
	};
	
}

#endif
