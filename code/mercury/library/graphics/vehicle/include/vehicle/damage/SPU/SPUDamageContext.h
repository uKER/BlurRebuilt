#ifndef SPUDAMAGECONTEXT_H_INCLUDED
#define SPUDAMAGECONTEXT_H_INCLUDED

#include "Vehicle/Damage/SPU/SPUDamageLightWeightImpact.h"
#include "Vehicle/Damage/SPU/SPUDamageLightWeightLayer.h"
#include "Vehicle/Damage/SPU/SPURepairContext.h"
#ifndef PLATFORM_SPU
#include <btl/Allocators/pool.h>
#include <System/Scheduler/jobqueue.h>
#endif // #ifndef PLATFORM_SPU

namespace SceneGraph
{
	class C_ModelInstance;
}

namespace Vehicle
{
	class C_NodeLayer;
}

namespace SpuDamage
{

	//! Damage context -- managed SPU tasks for car offset stream deformation.
	struct ALIGN128 C_DamageContext
	{
		C_LightWeightImpact					lightWeightDamageEvents[16];						//!< Array of light weight damage events.
		U32									inputVertexBuffers[maxNumberOfVertexBuffers];		//!< EA of input vb (either regular or cp).
		U32									outputVertexBuffers[maxNumberOfVertexBuffers];		//!< EA of output vb.
		U32									inputBufferSize[maxNumberOfVertexBuffers];			//!< Size of the input vertex buffers in bytes.
		U32									outputBufferSize[maxNumberOfVertexBuffers];			//!< Size of the output vertex buffers in bytes.
 		U16									inputStride[maxNumberOfVertexBuffers];				//!< Stride of input vertex buffers.
 		U16									outputStride[maxNumberOfVertexBuffers];				//!< Stride of output vertex buffers.
		U32									positionOffsetOffset[maxNumberOfVertexBuffers];		//!< Offset of position offset in each vertex buffer
		U32									normalOffsetOffset[maxNumberOfVertexBuffers];		//!< Offset of normal offset in each vertex buffer
		U32									controlPointOffset[maxNumberOfVertexBuffers];		//!< Offset of control point info in each vertex buffer
		U32									colourOffset[maxNumberOfVertexBuffers];				//!< Offset of vertex colours in each vertex buffer

 		F32 __attribute__((aligned(16)))	controlPoints[maxControlPoints << 2];				//!< List of control points.
		F32	__attribute__((aligned(16)))	boundsInfo[4];										//!< Bounds info.
		F32 __attribute__((aligned(16)))	codeDamageScale[4];									//!< Damage scale.
		F32*								pControlPointData;									//!< Control point data (6*3*4 floats)
		Vehicle::C_NodeLayer*				layerPtr;											//!< Pointer to the layers array.
 		float*								heightProfilePtr;									//!< Height profile.
 		bool								fatalDamage;										//!< Bool indicating if damage is fatal.
 		bool								transitioning;										//!< Do the damage gradually.
 		bool								forceRun;											//!< Set to true if repairing.
 		bool								forceRunThisFrame;									//!< Force to run regardless of how many have run this frame
		bool								noDeformation;										//!< Set to true to disable vertex deformation.
		F32									currentDamagePercentage;							//!< Current damage percentage for repair.
		F32									duration;											//!< Duration of repair.
		F32									amount;												//!< Repair amount.
		U32									damageLayerPtr;										//!< EA of damage layer.
		U32									damageStateEA;										//!< Effective address of the damage label.
		U32									contextIndex;										//!< Context index.
		U32									contextLabelValue;									//!< Label value for the context.
		U16									numberOfDamageEvents;								//!< Number of damage events used.
 		U16									vertexBufferCount;									//!< Number of vertex buffers used.
 		U16									heightProfileSize;									//!< Height profile size.
	};
	
#ifndef PLATFORM_SPU
	//! Context manager.
	class C_ContextManager
	{
	public:
		//! C'tor.
		explicit C_ContextManager();
		//! D'tor.
		~C_ContextManager();
		
		//! Allocate a free damage context.
		//@return Pointer to a free damage context, NULL if not available.
		__forceinline NOALIAS C_DamageContext*	Allocate()
		{
			BIZ_ASSERT_MSG(m_bDoneInit, "Need to startup the context manager first!");
			C_DamageContext* pContext = NULL;
			pContext = m_damageContexts.Allocate();
			BIZ_ASSERT_MSG(pContext, "Ran out of damage contexts.");
			if(pContext)
			{
				pContext->contextIndex = m_freeContexts.Back();	// top of the stack.
				m_freeContexts.PopBack();
				pContext->damageStateEA = (U32)&m_jobLabels[pContext->contextIndex];			// Setup the context pointers/index.
				pContext->contextLabelValue = m_jobLabels[pContext->contextIndex].m_value + 1;
			}
			return pContext;
		}
		//! Finalise context -- kick the Spu job.
		//@param pContext Pointer to the context.
		//@return True if the context was successfully finalised.
		const bool								FinaliseContext(C_DamageContext* __restrict pContext);
		//! Free the context.
		//@param pContext Pointer to the context to free.
		void									Free(C_DamageContext* __restrict pContext);
		//! Shutdown the context manager.
		void									Shutdown();
		//! Spin on jobs -- only do this immediately before model is being deleted.
		void									Spin() const __restrict
		{ 	
			BIZ_ASSERT_MSG(m_bDoneInit, "Need to startup the context manager first!");
			Scheduler::Flush(m_taskGroup); 
		}
		//! Initialise the context manager.
		void									Startup();
		//! Wait on a specific context's Spu task.
		void									WaitOnContextTask(C_DamageContext* __restrict pContext);
		//! Zero run jobs count.
		static void								ZeroRunJobsCount();
		//! Set the policy for the damage context.
		//@param iPolicyHandle Handle for the policy the damage context manager should operate with.
		static void								SetSchedulerPolicy(const S32 iPolicyHandle);
		
	private:
		Scheduler::GroupHandle					m_taskGroup;			//!< Task group for damage jobs.
		Btl::StaticPool<C_DamageContext, 20>	m_damageContexts;		//!< Damage contexts.
		Btl::Vector<C_DamageContext*, Btl::VectorParameters_Static<20> >
												m_runnableContexts;		//!< Damage jobs to run.
		U32										m_frameID;				//!< ID of last frame we ran damage jobs.
		bool									m_bDoneInit;			//!< Set to true if init is complete.
		Btl::Vector<U32, Btl::VectorParameters_Static<20> >					
												m_freeContexts;			//!< Stack of free contexts.
		static U32								m_RunThisFrame;			//!< Number of jobs run in this frame.
		static const U32						m_MaxPerFrame;			//!< Maximum number of jobs to run per frame.

		//!< Manage the state of the jobs.
		struct ALIGN128 DamageLabel
		{
			U32 m_value; //!< Value of the job running.
		};

		static S32								m_PolicyHandle;			//!< Handle for the scheduler policy.
		DamageLabel								m_jobLabels[20];		//!< Managed state of damage jobs.	
	};
#endif // #ifndef PLATFORM_SPU

};

#endif // #ifndef SPUDAMAGECONTEXT_H_INCLUDED
