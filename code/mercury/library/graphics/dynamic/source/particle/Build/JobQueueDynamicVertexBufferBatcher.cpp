#include "stdafx.h"

#include <Dynamic/View.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>
#include <Dynamic/Particle/Build/JobQueueDynamicVertexBufferBatcher.h>
#include <Dynamic/Particle/VertexBuildSettings.h>
#include <system/scheduler/jobqueue.h>

#ifdef PLATFORM_PS3
extern Scheduler::jqModule ParticleVertexBuildShape2DModule;
extern Scheduler::jqModule ParticleVertexBuildShape3DModule;
extern Scheduler::jqModule ParticleVertexBuildSpriteRotatingModule;
#else
extern Scheduler::jqModule ParticleVertexBuildModule;
#endif

namespace DynamicEffect
{
namespace Particle
{
namespace BuildPolicy
{

Btl::DynamicPool<JobQueueDynamicVertexBufferBatch>		C_JobQueueDynamicVertexBufferBatcher::m_JobQueueDynamicVertexBufferBatch;

Debug::Int g_VertexBuildBatchesPerTask("Graphics.Dynamic.Particle.VertexBuildBatchesPerTask", 32, Debug::Range(1, 32) );

void C_JobQueueDynamicVertexBufferBatcher::AddNode(const C_View& view, const RenderNodePtrs* pBlocks, const VertexBuildSettings* pSettings, void* pBuffer)
{
	const E_VertexBuild vertexBuildType = pSettings->eVertexBuild;
	JobQueueDynamicVertexBufferBatches& batches = m_Batches[vertexBuildType];

	JobQueueDynamicVertexBufferBatch* pBatch;
	
	// if no batches add allocate one from dynamic pool, otherwise get end one
	if(batches.Size() == 0)
	{
		pBatch = m_JobQueueDynamicVertexBufferBatch.Allocate();
		pBatch->vertexBuildData.Clear();
		batches.PushBack(pBatch);
	}
	else
		pBatch = batches[batches.Size() - 1];
	
	if(pBatch->vertexBuildData.Size() != pBatch->vertexBuildData.Capacity())
	{
		pBatch->vertexBuildData.PushBack();

		VertexBuildData& data = pBatch->vertexBuildData.Back();
		data.pBlocks = pBlocks;
		data.pSettings = pSettings;
		data.pBuffer = pBuffer;

		// see if we have enough things batched up to run a task
		if(pBatch->vertexBuildData.Size() == g_VertexBuildBatchesPerTask)
		{
			VertexBuildParams params;
			params.pVertexBuildDataArray = &pBatch->vertexBuildData[0];
			params.vertexBuildDataCount = pBatch->vertexBuildData.Size();
			params.pConstants = &view.GetVertexBuildConstants();

			const C_Renderer* pRenderer = (const C_Renderer*)view.GetRenderer();

		#ifdef PLATFORM_PS3
			MT::AtomicIncrement(params.pConstants->pVertexBuildingJobs);
			switch(vertexBuildType)
			{
			case E_VertexBuild_Shape2D:
				Scheduler::AddTask(ParticleVertexBuildShape2DModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
				break;
			case E_VertexBuild_Shape3D:
				Scheduler::AddTask(ParticleVertexBuildShape3DModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
				break;
			case E_VertexBuild_SpriteRotating:
				Scheduler::AddTask(ParticleVertexBuildSpriteRotatingModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
				break;
			}
		#else
			Scheduler::AddTask(ParticleVertexBuildModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
		#endif

			// add a new bit of data for subsequent tasks
			if(JobQueueDynamicVertexBufferBatch* pNewBatch = m_JobQueueDynamicVertexBufferBatch.Allocate())
			{
				pNewBatch->vertexBuildData.Clear();
				batches.PushBack(pNewBatch);
			}
		}
	}
}

void C_JobQueueDynamicVertexBufferBatcher::EndBuild(const C_View& view)
{
	for(S32 vertexBuildType = 0; vertexBuildType != E_VertexBuild_Count; ++vertexBuildType)
	{
		JobQueueDynamicVertexBufferBatches& batches = m_Batches[vertexBuildType];

		if(batches.Size() != 0)
		{
			JobQueueDynamicVertexBufferBatch* pBatch = batches[batches.Size() - 1];

			// see if the last batch needs to be sent off
			if(pBatch->vertexBuildData.Size() != 0 && pBatch->vertexBuildData.Size() != g_VertexBuildBatchesPerTask)
			{
				VertexBuildParams params;
				params.pVertexBuildDataArray = &pBatch->vertexBuildData[0];
				params.vertexBuildDataCount = pBatch->vertexBuildData.Size();
				params.pConstants = &view.GetVertexBuildConstants();

				const C_Renderer* pRenderer = (const C_Renderer*)view.GetRenderer();

			#ifdef PLATFORM_PS3
				MT::AtomicIncrement(params.pConstants->pVertexBuildingJobs);
				switch(vertexBuildType)
				{
				case E_VertexBuild_Shape2D:
					Scheduler::AddTask(ParticleVertexBuildShape2DModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
					break;
				case E_VertexBuild_Shape3D:
					Scheduler::AddTask(ParticleVertexBuildShape3DModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
					break;
				case E_VertexBuild_SpriteRotating:
					Scheduler::AddTask(ParticleVertexBuildSpriteRotatingModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
					break;
				}
			#else
				Scheduler::AddTask(ParticleVertexBuildModule, NULL, NULL, view.GetJobQueueGroup(), pRenderer->GetJobQueuePolicyHandle(), &params, sizeof(VertexBuildParams));
			#endif
			}
		}
	}
}

void C_JobQueueDynamicVertexBufferBatcher::EndFrame()
{
	// give back all the bits of data to the dynamic pool
	for(S32 vertexBuildType = 0; vertexBuildType != E_VertexBuild_Count; ++vertexBuildType)
	{
		JobQueueDynamicVertexBufferBatches& batches = m_Batches[vertexBuildType];

		for(S32 i = 0; i != batches.Size(); ++i)
			m_JobQueueDynamicVertexBufferBatch.Free(batches[i]);

		batches.Clear();
	}
}


} // BuildPolicy
} // Particles
} // DynamicEffect











