#ifndef PORTAL_DEFERREDTRACER_H
#define PORTAL_DEFERREDTRACER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>
#include <Btl/Utility/pair.h>
#include <Btl/Allocators/pool.h>
#include <Portal/DeferredTracerJob.h>
#include <Portal/BSPContainer.h>
#include <System/debug/Variables.h>

namespace Portal
{
	// Call this forward:
	class C_System;

	//! Deferred tracer call-back functor base class.
	class C_DeferredTracer
	{
	public:
		//! C'tor.
		explicit C_DeferredTracer(C_System* pSystem);
		//! D'tor.
		~C_DeferredTracer();

		//! Add ray to the tracer schedule.
		//@param rkRay Ray to trace.
		//@param pFunctor Function object to call when trace is complete.
		// TODO: [Stove] BUG!! In this function I take address of something that could get destroyed on the stack.
		void						AddRay(const C_Ray& rkRay, I_DeferredTracerFunctorBase* pFunctor);
		//! Debug render the ray that were deferred traced this frame.
		//@param pDebugRender Pointer to an instance of the debug renderer.
		//@param rkCameraMat View matrix.
		void						DebugRenderTracedRays(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat);
		//! End deferred trace jobs.
		NOALIAS void				EndJobs() __restrict;
		//! Return the BSP container.
		//@return The BSP container object.
		C_BSPContainer&				GetBSPTree();
		//! Return the BSP container object.
		//@return The BSP container object.
		const C_BSPContainer&		GetBSPTree() const __restrict;
		//! Start deferred trace jobs.
		NOALIAS void				StartJobs() __restrict;
		
	private:
		C_BSPContainer ALIGN16					m_BSPContainer;					//!< BSP tree object.

#ifndef PLATFORM_SPU
		mutable bool							m_bJobsHandled;					//!< Job results have been handled.
		mutable bool							m_bJobsStarted;					//!< Set to true when jobs have been started.
		Btl::Vector<Btl::Pair<const C_Ray*, I_DeferredTracerFunctorBase*> >						
												m_vTraceQueue;					//!< Queue of rays to trace.
		Btl::DynamicPool<C_DeferredTracerJob>	m_DeferredTracerJobPool;		//!< Deferred tracer job pool.
		Btl::Vector<C_DeferredTracerJob*>		m_vLiveSpuJobs;					//!< Pointers to live Spu job data from the dynamic pool.
		Scheduler::GroupHandle					m_GroupHandle;					//!< Group handle for jobs.
		C_System*								m_pSystem;						//!< Pointer to parent portal system object.
#ifndef CFG_GOLD
		Btl::Vector<C_TraceResults>				m_vDebugTraceResults;			//!< Vector of trace results from deferred tracer.
#endif // #ifndef CFG_GOLD
		static Debug::Bool						m_bUseDeferredTracerJobQueue;	//!< Set to true when deferred trace job queue is active.
#endif // #ifndef PLATFORM_SPU

		// Disallow calls to this:
		explicit C_DeferredTracer();
	};
}

#endif // PORTAL_DEFERREDTRACER_H
