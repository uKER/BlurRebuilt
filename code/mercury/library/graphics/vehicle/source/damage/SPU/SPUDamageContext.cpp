#include "VehiclePCH.h"
#include "Core/threads/Atomic.h"
#include "Vehicle/Damage/SPU/SPUDamageContext.h"
#include "Vehicle/Damage/SPU/SPUDamageJob.h"
#include "System/Scheduler/JobQueue.h"


using namespace SpuDamage;

Debug::Bool g_noSpuDeformation("Vehicle.Damage.NoDeform", false);
const U32 C_ContextManager::m_MaxPerFrame = 1;
U32 C_ContextManager::m_RunThisFrame = 0;
int C_ContextManager::m_PolicyHandle = -1;

// -----------------------------------------------------------------

C_ContextManager::C_ContextManager() :
	m_frameID(0),
	m_bDoneInit(false)
{
	m_freeContexts.Resize(20);
	for(U32 iCount=0; iCount<20; ++iCount)
	{
		m_jobLabels[iCount].m_value		= 0;		// Initialise all the labels.
		m_freeContexts[iCount]			= iCount;	// Initialise the free context allocator.
	}

	return;
}

// -----------------------------------------------------------------

C_ContextManager::~C_ContextManager()
{
	// Wait until all tasks are done before we kill this object.
	Spin();
	return;
}

// -----------------------------------------------------------------

// Finalise the context -- kick the job.
const bool C_ContextManager::FinaliseContext(C_DamageContext* __restrict pContext)
{
	BIZ_ASSERT_MSG(m_bDoneInit, "Need to startup the context manager first!");

	{
		PERF_EVENT("Spu stall test");

		while(pContext->contextLabelValue < MT::AtomicGet(&m_jobLabels[pContext->contextIndex].m_value))
		{
			;
		}
	}
	
	if(m_RunThisFrame < m_MaxPerFrame || pContext->forceRunThisFrame)
	{
		// run task with deformation.
		pContext->noDeformation = g_noSpuDeformation.Value();
		pContext->contextLabelValue = m_jobLabels[pContext->contextIndex].m_value + 1;	// set to the next update's value.
		Scheduler::AddTask(CarDamageJob, (void*)pContext, NULL, m_taskGroup, m_PolicyHandle);
		pContext->forceRun = false;		// we've run it now! wOOt
		pContext->forceRunThisFrame = false;
		m_RunThisFrame++;
	}
	return true;
}

// -----------------------------------------------------------------

// Free context.
void C_ContextManager::Free(C_DamageContext* __restrict pContext)
{
	BIZ_ASSERT_MSG(m_bDoneInit, "Need to startup the context manager first!");
	while(pContext->contextLabelValue < MT::AtomicGet(&m_jobLabels[pContext->contextIndex].m_value))
	{
		sys_timer_usleep(30);	// spinning for this particular damage job.
	}
	m_jobLabels[pContext->contextIndex].m_value = 0;
	m_freeContexts.PushBack(pContext->contextIndex);		// add it back onto the stack.
	
	BIZ_ASSERT_MSG(pContext, "NULL context pointer!");
	if(pContext)
		m_damageContexts.Free(pContext);

	return;
}
	
// -----------------------------------------------------------------

// Shutdown the context manager.
void C_ContextManager::Shutdown()
{
	BIZ_ASSERT_MSG(m_bDoneInit, "Need to startup the context manager first!");
	Scheduler::DestroyGroup(m_taskGroup);
	return;
}

// -----------------------------------------------------------------

// Startup.
void C_ContextManager::Startup()
{
	if(!m_bDoneInit)
	{
		m_taskGroup = Scheduler::CreateGroup();
		m_bDoneInit = true;
	}
	return;
}

// -----------------------------------------------------------------

// Wait on the Spu task of a specific context.
void C_ContextManager::WaitOnContextTask(C_DamageContext* __restrict pContext)
{
	BIZ_ASSERT_MSG(m_bDoneInit, "Need to startup the context manager first!");
	// TODO: [Stove] this needs to be more granular, for now spin on all tasks.
	Spin();
	return;
}

// -----------------------------------------------------------------

// Zero the run this frame counter.
void C_ContextManager::ZeroRunJobsCount()
{
	m_RunThisFrame = 0;
	return;
}

// -----------------------------------------------------------------

// Set the scheduler policy for the damage context manager.
void C_ContextManager::SetSchedulerPolicy(const S32 x)
{
	m_PolicyHandle = x;
	return;
}

