#include <stdafx.h>
#include <Portal/DeferredTracer.h>
#include <Portal/Ray.h>
#include <Portal/TraceResults.h>
#include <Portal/DeferredTracerFunctorBase.h>
#include <system/Perf/PerfEvent.h>

using namespace Portal;

Debug::Bool				C_DeferredTracer::m_bUseDeferredTracerJobQueue("Graphics.Portal.UseDeferredTracer", true);


// C'tor.
C_DeferredTracer::C_DeferredTracer(C_System* pSystem) :
	m_bJobsStarted(false),
	m_pSystem(pSystem)
{
	m_GroupHandle = Scheduler::CreateGroup();
	return;
}

// -----------------------------------------------------------------

// Default c'tor.
C_DeferredTracer::C_DeferredTracer() :
	m_bJobsStarted(false),
	m_pSystem(NULL),
	m_GroupHandle(Scheduler::JQ_GROUP_NONE)
{
	BIZ_WARNING_MSG(m_pSystem, "Deferred tracer has NULL portal system parent pointer.");
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_DeferredTracer::~C_DeferredTracer()
{
	// Do we need to discard last lot of deferred rays?
	if(!m_bJobsHandled)
	{
		Scheduler::Flush(m_GroupHandle, true);
		while(static_cast<U32>(m_vLiveSpuJobs.Size()) > 0)
		{
			C_DeferredTracerJob* pQueueFront = m_vLiveSpuJobs.At(0);
			BIZ_WARNING_MSG(pQueueFront, "Sanity check failed, trying to process NULL tracer job I/O from the live Spu job queue.");
			m_vLiveSpuJobs.EraseByIndexNonOrdered(0);
			m_DeferredTracerJobPool.Free(pQueueFront);
		}
		m_bJobsHandled = true;
	}

	Scheduler::DestroyGroup(m_GroupHandle);
	return;
}

// -----------------------------------------------------------------

// Scheduler a ray for deferred tracing.
void C_DeferredTracer::AddRay(const C_Ray& rkRay, I_DeferredTracerFunctorBase* pFunctor)
{
	if(m_bUseDeferredTracerJobQueue.Value())
		m_vTraceQueue.PushBack(Btl::MakePair(&rkRay, pFunctor));
	else
	{
		// TODO: [Stove] Trace the ray immediately and call the functor.
		/*C_TraceResults results = rkRay.Trace(m_pSystem->GetDeferredTracer());
		pFunctor->operator ()(results);*/
	}

	return;
}

// -----------------------------------------------------------------

// Debug render the traced rays.
void C_DeferredTracer::DebugRenderTracedRays(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43 &rkCameraMat)
{

#ifndef CFG_GOLD
	BIZ_WARNING_MSG(pDebugRender, "NULL debug renderer pointer.");
	if(!pDebugRender)
		return;

	const Maths::RGBA red		(1.0f, 0.0f, 0.0f, 1.0f);
	const Maths::RGBA yellow	(1.0f, 1.0f, 0.0f, 1.0f);

	// Draw all the rays we traced last update.
	for(U32 iCurrentRay=0; iCurrentRay<static_cast<U32>(m_vDebugTraceResults.Size()); ++iCurrentRay)
	{
		pDebugRender->Draw3dCross(m_vDebugTraceResults[iCurrentRay].GetOrigin(), 1.0f, yellow);
		Maths::HVec3 dir = m_vDebugTraceResults[iCurrentRay].GetEndPoint() - m_vDebugTraceResults[iCurrentRay].GetOrigin();
		const F32 fLength = dir.Length();
		dir.Normalise();
		pDebugRender->Draw3dArrow(m_vDebugTraceResults[iCurrentRay].GetOrigin(), dir, fLength, 3.0f, yellow);
		//pDebugRender->Draw3dLine(m_vDebugTraceResults[iCurrentRay].GetOrigin(), m_vDebugTraceResults[iCurrentRay].GetEndPoint(), blue);
		if(m_vDebugTraceResults[iCurrentRay].HasHit()) // Intersection point.
		{
			pDebugRender->Draw3dCross(m_vDebugTraceResults[iCurrentRay].GetWorldPosition(), 3.0f, red);
			pDebugRender->Draw3dArrow(m_vDebugTraceResults[iCurrentRay].GetWorldPosition(), m_vDebugTraceResults[iCurrentRay].GetWorldNormal(), 3.0f, 1.0f, red);
		}
	}
	m_vDebugTraceResults.Clear();	// Clear the vector out.
#endif // #ifndef CFG_GOLD

	return;
}

// -----------------------------------------------------------------

// End deferred trace of rays.
void C_DeferredTracer::EndJobs()
{
 	PERF_EVENT("C_DeferredTracer::EndJobs");
	
	Scheduler::Flush(m_GroupHandle, true);
	if(m_bJobsHandled)
		return;	// Early out on job handling.

	while(static_cast<U32>(m_vLiveSpuJobs.Size()) > 0)
	{
		C_DeferredTracerJob* pQueueFront = m_vLiveSpuJobs.At(0);
		BIZ_WARNING_MSG(pQueueFront, "Sanity check failed, trying to process NULL tracer job I/O from the live Spu job queue.");

		// Call the functors for on all the traced rays.
		for(U32 iCurrentRay=0, iCount=pQueueFront->m_Input.m_iRayCount; iCurrentRay<iCount; ++iCurrentRay)
		{
			pQueueFront->m_Output.m_vFunctors[iCurrentRay]->operator ()(pQueueFront->m_Output.m_vResults[iCurrentRay]);
#ifndef CFG_GOLD
			// Grab this for debugging purposes.
			m_vDebugTraceResults.PushBack(pQueueFront->m_Output.m_vResults[iCurrentRay]);
#endif // #ifndef CFG_GOLD
		}

		m_vLiveSpuJobs.EraseByIndexNonOrdered(0);
		m_DeferredTracerJobPool.Free(pQueueFront);
	}
	m_bJobsHandled = true;
	BIZ_WARNING_MSG(!static_cast<U32>(m_vLiveSpuJobs.Size()), "Sanity check failed, should have completely emptied the live Spu job pointer queue.");
	BIZ_WARNING_MSG(!static_cast<U32>(m_DeferredTracerJobPool.Allocated()), "Sanity check failed, should have completely deallocated memory for Spu job inputs.");
	return;
}

// -----------------------------------------------------------------

// Return the BSP container.
C_BSPContainer& C_DeferredTracer::GetBSPTree()
{
	return m_BSPContainer;
}

// -----------------------------------------------------------------

// Return the BSP container.
const C_BSPContainer& C_DeferredTracer::GetBSPTree() const __restrict
{
	return m_BSPContainer;
}

// -----------------------------------------------------------------

// Start tracing the rays.
void C_DeferredTracer::StartJobs()
{
 	PERF_EVENT("C_DeferredTracer::StartJobs");

	// Do we need to discard last lot of deferred rays?
	if(!m_bJobsHandled)
	{
		Scheduler::Flush(m_GroupHandle, true);
		while(static_cast<U32>(m_vLiveSpuJobs.Size()) > 0)
		{
			C_DeferredTracerJob* pQueueFront = m_vLiveSpuJobs.At(0);
			BIZ_WARNING_MSG(pQueueFront, "Sanity check failed, trying to process NULL tracer job I/O from the live Spu job queue.");
			m_vLiveSpuJobs.EraseByIndexNonOrdered(0);
			m_DeferredTracerJobPool.Free(pQueueFront);
		}
		m_bJobsHandled = true;
	}

	// Kick jobs batches for rays scheduled for tracing.
	while(static_cast<U32>(m_vTraceQueue.Size()) > 0)
	{
		C_DeferredTracerJob* pCurrentJobData = m_DeferredTracerJobPool.Allocate();
		m_vLiveSpuJobs.PushBack(pCurrentJobData);
		BIZ_WARNING_MSG(pCurrentJobData, "Failed to allocate memory from dynamic pool.");
		U32 iCurrentRay = 0;
		for(; iCurrentRay<C_DeferredTracerJob::MaxBatchSize && static_cast<U32>(m_vTraceQueue.Size())>0; ++iCurrentRay)
		{
			pCurrentJobData->m_Input.m_vRays[iCurrentRay] = *(m_vTraceQueue.At(0).first);
			pCurrentJobData->m_Output.m_vFunctors[iCurrentRay] = m_vTraceQueue.At(0).second;
			m_vTraceQueue.EraseByIndexNonOrdered(0);
		}
		pCurrentJobData->m_Input.m_pTreeEA = &m_BSPContainer;
		pCurrentJobData->m_Input.m_iRayCount = iCurrentRay;
		BIZ_WARNING_MSG(pCurrentJobData->m_Input.m_pTreeEA, "Tree effective address is NULL.");
		BIZ_WARNING_MSG(iCurrentRay, "Trying to add deferred raytracing batch with no rays.");

		Scheduler::AddTask(DeferredTracerJobModule, 
						   (void*)&pCurrentJobData->m_Input, 
						   (void*)&pCurrentJobData->m_Output, 
						   m_GroupHandle, 
						   0,			/* TODO: [Stove] Need to set the policy for the scheduler tasks. */
						   (void*)&pCurrentJobData, sizeof(C_DeferredTracerJob*));
		m_bJobsHandled = false;
	}

	return;
}
