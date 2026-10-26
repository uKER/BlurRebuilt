#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_VERTEXBUILD_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_VERTEXBUILD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler/jobqueue.h>

namespace DynamicEffect
{
namespace Particle
{
namespace BuildPolicy
{

template<class Policy>
void VertexBuild(const RenderNodePtrs* pBlocks, const VertexBuildSettings* pSettings, volatile void* __restrict pBuffer, const VertexBuildConstants* pConstants)
{
	Spu::MemBaseMarker marker;

	// fetch the settings and constants
	pSettings = (const VertexBuildSettings*)Spu::FetchAllocAsync(pSettings, sizeof(VertexBuildSettings), 0);
		
	U32 sizeRenderNodePtrs = sizeof(RenderNodePtrs);
	U32 sizeRenderNodePtrsMod = (sizeRenderNodePtrs + 15) & ~15;

#ifdef PLATFORM_SPU
	// fetch blocks
	RenderNodePtrs* pBlocksLS = (DynamicEffect::RenderNodePtrs*)Spu::FetchAlloc(pBlocks, sizeRenderNodePtrsMod);
	pBlocks = pBlocksLS;
	RenderNodePtrs& blocks = *pBlocksLS;
	for(S32 b = 0; b < pBlocksLS->Size(); ++b)
		blocks[b] = (ParticleBlock*)Spu::FetchAllocAsync((ParticleBlock*)blocks[b], sizeof(ParticleBlock), 0);
	Spu::Wait(0);
	// fetch particles and transforms
	for(S32 b = 0; b < pBlocksLS->Size(); ++b)
	{
		ParticleBlock* pBlock = (ParticleBlock*)blocks[b];
		if(pBlock->m_pNode)
			pBlock->m_pNode = (C_Node*)Spu::FetchAllocAsync(pBlock->m_pNode, sizeof(C_Node), 0);
		pBlock->m_Element.FetchAsync(1);
	}
	// wait for transforms and particle data to come in
	Spu::Wait(0);
	Spu::Wait(1);
#endif

	if(pSettings->eParticleSort == E_ParticleSort_PerParticle)
	{
		ParticleArray particleSortList;
		//particleSortList.Reserve(1024);
		SortParticles(pConstants, *pBlocks, particleSortList);
		Policy::Build(pSettings, pConstants, particleSortList, pBuffer);
	}
	else
	{
		Policy::Build(pSettings, pConstants, *pBlocks, pBuffer);
	}
}

template<class Policy>
void VertexBuildTask(Scheduler::Task* pTask)
{
#ifndef PLATFORM_SPU
	PERF_EVENT("Particle.VertexBuild");
#endif
	VertexBuildParams* pParams = (VertexBuildParams*)pTask->ParamData;
	const VertexBuildConstants* pConstants = (const VertexBuildConstants*)Spu::FetchAllocAsync(pParams->pConstants, sizeof(VertexBuildConstants), 0);

	// fetch data array
	U32 sizeVertexBuildData = (pParams->vertexBuildDataCount * sizeof(VertexBuildData) + 15) & ~15;
	VertexBuildData* pDataArray = (VertexBuildData*)Spu::FetchAllocAsync(pParams->pVertexBuildDataArray, sizeVertexBuildData, 0);
	Spu::Wait(0);
	pConstants->pTexCoords = (C_TextureCoords*)Spu::FetchAllocAsync(pConstants->pTexCoords, sizeof(C_TextureCoords), 0);

	for(S32 i = 0; i < pParams->vertexBuildDataCount; ++i)
	{
		VertexBuild<Policy>(pDataArray[i].pBlocks, pDataArray[i].pSettings, pDataArray[i].pBuffer, pConstants);
	}

#ifdef PLATFORM_SPU
	MT::AtomicDecrement(pConstants->pVertexBuildingJobs);
#endif
}

} // BuildPolicy
} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_VERTEXBUILD_H
