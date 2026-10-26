#ifndef PORTAL_DEFERREDTRACERJOB_H
#define PORTAL_DEFERREDTRACERJOB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/scheduler/jobqueue.h>
#include <Btl/Containers/vector.h>
#include <Btl/Containers/list.h>
#include <Portal/Configuration.h>
#include <Portal/Ray.h>
#include <Portal/TraceResults.h>
#include <Portal/BSPContainer.h>

namespace Portal
{
	//! Structure for deferred ray tracing job.
	struct ALIGN16 C_DeferredTracerJob : public Btl::ListElementDouble<C_DeferredTracerJob>
	{
		static const U32	MaxBatchSize = 256;							//!< Maximum size of batch.
		
		//! Input to the deferred tracer batch processing job.
		struct ALIGN16 Input
		{
			C_Ray							m_vRays[MaxBatchSize];		//!< Vector of rays to classify.
			C_BSPContainer*					m_pTreeEA;					//!< Effective address of tree.
			U32								m_iRayCount;				//!< Number of rays in the batch.
		};

		//! Output from the deferred tracer batch processing job.
		struct ALIGN16 Output
		{
			ALIGN16 C_TraceResults			m_vResults[MaxBatchSize];	//!< Vector of results.
			I_DeferredTracerFunctorBase*	m_vFunctors[MaxBatchSize];	//!< Vector of functor pointers to call for each result.
			
#ifdef PLATFORM_PS3
			// Choose a batch size to satifsy this contraint -- this will only cause issues on PS3.
			BIZ_STATIC_ASSERT((sizeof(C_TraceResults) * MaxBatchSize) % 16 == 0 &&
						  (sizeof(C_DeferredTracerJob::Input) % 16) == 0);
#endif // #ifdef PLATFORM_PS3
		};

		Input		m_Input;											//!< Input to deferred trace job.
		Output		m_Output;											//!< Output from deferred trace job.
	};
}

extern Scheduler::jqModule DeferredTracerJobModule;

#endif // PORTAL_DEFERREDTRACERJOB_H
