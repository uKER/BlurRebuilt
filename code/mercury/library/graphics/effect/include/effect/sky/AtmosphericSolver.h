#ifndef EFFECT_SKY_ATMOSPHERICSOLVER_H
#define EFFECT_SKY_ATMOSPHERICSOLVER_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Scheduler
{
	struct Task;
};

namespace Effect
{
namespace Sky
{

struct C_AtmosphericTask;

void SolveAtmosphere(const C_AtmosphericTask* const pTask);
void SolveAtmosphereTask(Scheduler::Task* pTask);

}; // namespace Sky
}; // namespace Effect

#endif // EFFECT_SKY_ATMOSPHERICSOLVER_H
