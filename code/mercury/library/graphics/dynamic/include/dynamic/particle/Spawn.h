#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SPAWN_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SPAWN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Defines.h"
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
#include "Dynamic/Particle/UpdateXM.h"
#else
#include "Dynamic/Particle/UpdateHV.h"
#endif

#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/Particle/SpawnSettings.h"
#include "Dynamic/Particle/UpdateData.h"
#include "Dynamic/BlockManager.h"

namespace DynamicEffect
{
namespace Particle
{

class I_ParticleEffectBase;
struct SpawnSettings;

template<class Particle, template <class P> class Policy>
class DefaultCreationPolicy
{
public:
	typedef DefaultCreationPolicy		Base;		
	typedef Particle					ParticleType;

	static void CreateParticles(U8* pParticleData, S32 num, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
	{
		Particle* pParticle = (Particle*)pParticleData;
		for(S32 i = 0; i < num; ++i)
			Policy<Particle>::CreateParticle(&pParticle[i], random, pEmitter, pSpawnSettings);
	}
	static void CreateParticle(U8* pParticleData, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
	{
		Particle* pParticle = (Particle*)pParticleData;
		Policy<Particle>::CreateParticle(pParticle, random, pEmitter, pSpawnSettings);
	}
	static void CreateBlock(ParticleBlock* pBlock, C_Emitter* pEmitter) {}
};

ParticleBlock*		BeginCreateExisting(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const EmitterUpdateConstants* pUpdateConstants);
ParticleBlock*		BeginCreateNew(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const EmitterUpdateConstants* pUpdateConstants);
void				SpawnCustom(C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, C_Random& random, const EmitterUpdateConstants* pUpdateConstants);

template<class CreationPolicy>
ParticleBlock* BeginCreateT(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const EmitterUpdateConstants* pUpdateConstants)
{
	if(ParticleBlock* pParticleBlock = BeginCreateExisting(nNumEmit, nTotalEmit, pEmitter, pSpawnSettings, pUpdateConstants))
		return pParticleBlock;

	if(ParticleBlock* pParticleBlock = BeginCreateNew(nNumEmit, nTotalEmit, pEmitter, pSpawnSettings, pUpdateConstants))
	{
		CreationPolicy::CreateBlock((ParticleBlock*)pParticleBlock, pEmitter);
		return pParticleBlock;
	}

	return NULL;
}

template<class CreationPolicy>
void SpawnT(C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, C_Random& random, const EmitterUpdateConstants* pUpdateConstants)
{
	assert(pEmitter);
	// check to see if our block is still alive, if not invalidate pointer
	if(ParticleBlock* pBlockLS = pEmitter->GetBlock())
	{
		// check to see if block is still owned by us, or if block has more than max particles
		if(pBlockLS->m_pEffect != (I_DynamicEffect*)pSpawnSettings->pEffect || pBlockLS->m_iSalt != pEmitter->GetBlockSalt())
			pEmitter->SetBlock(NULL);
	}

	F32 cullDistance = pEmitter->GetDefinition()->cullDistance != 0.0f ? pEmitter->GetDefinition()->cullDistance : pSpawnSettings->cullDistance;
	U32 nNumEmit = 0;
	U32 nTotalEmit = pEmitter->GetSpawnAmount();

	// spawn blocks which take up the block size
	while(nTotalEmit > 0)
	{
		if(ParticleBlock* pBlockLS = BeginCreateT<CreationPolicy>(nNumEmit, nTotalEmit, pEmitter, pSpawnSettings, pUpdateConstants))
		{
		#ifdef PLATFORM_SPU
			Spu::MemBaseMarker marker;
			U8* pParticleDataEA = pBlockLS->m_Element.Fetch();
		#endif
			// emit the remaining particles for this update
			U8* pParticle = (U8*)pBlockLS->NewParticles(nNumEmit);
			CreationPolicy::Base::CreateParticles(pParticle, nNumEmit, random, pEmitter, pSpawnSettings);
			pBlockLS->CalculateExtents(cullDistance, pSpawnSettings->vertexSize);

		#ifdef PLATFORM_SPU
			ParticleBlock* pBlockEA = pEmitter->GetBlockEA();
			pBlockLS->m_Element.Store(pParticleDataEA);
			// we've finished with the block so store it back to memory
			Spu::Store(pBlockEA, pBlockLS, sizeof(ParticleBlock));
		#endif

			// if theres no particles remaining then NULL out block
			if(pBlockLS->m_Element.Remaining() == 0)
				pEmitter->SetBlock(NULL);
		}
	}
	// update previous position
	pEmitter->SetPositionPreviousEmit();
}

template<class CreatePolicy>
void Spawn(SpawningEmitter* pSpawningEmitter, SpawnSettings* pSpawnSettings, S32 count, C_Random& random, const EmitterUpdateConstants* pUpdateConstants)
{
#ifndef PLATFORM_SPU
	PERF_EVENT("Particle.Spawn");
#endif

	for(S32 i = 0; i < count; ++i)
	{
		C_Emitter* pEmitter = pSpawningEmitter[i].pEmitter;

	#ifdef PLATFORM_SPU
		// fetch the emitter, emitter def, and allocate a block
		Spu::MemBaseMarker marker;
		C_Emitter* pEmitterEA = pEmitter;
		pEmitter = (C_Emitter*)Spu::FetchAlloc(pEmitterEA, sizeof(C_Emitter));

		// fetch the parents transformation if it exists
		if(pEmitter->m_pParentTransformEA)
			pEmitter->m_pParentTransformLS = (C_Node*)Spu::FetchAllocAsync(pEmitter->m_pParentTransformEA, sizeof(C_Node), 0);

		const C_SpawnMesh* pSpawnMeshEA = pEmitter->m_pSpawnMesh;
		if(pEmitter->m_pSpawnMesh)
		{
			pEmitter->m_pSpawnMesh = (C_SpawnMesh*)Spu::FetchAlloc(pEmitter->m_pSpawnMesh, sizeof(C_SpawnMesh));
			pEmitter->m_pSpawnMesh->SpuFetchAsync(0);
		}

		const EmitterDefinition* pDefinitionEA = pEmitter->m_pDefinition;
		pEmitter->m_pDefinition = (EmitterDefinition*)Spu::FetchAllocAsync(pDefinitionEA, sizeof(EmitterDefinition), 0);

		// alloc the LS block and fetch current if it exists
		pEmitter->m_pBlockLS = (ParticleBlock*)Spu::Alloc(sizeof(ParticleBlock));
		if(pEmitter->m_pBlockEA)
			 Spu::FetchAsync(pEmitter->m_pBlockLS, pEmitter->m_pBlockEA, sizeof(ParticleBlock), 0);

		Spu::Wait(0);
	#endif

		assert(pEmitter);
		SpawnT<CreatePolicy>(pEmitter, pSpawnSettings, random, pUpdateConstants);

	#ifdef PLATFORM_SPU
		// store block and emitter back to memory
		if(pEmitter->m_pBlockEA)
			Spu::Store(pEmitter->m_pBlockEA, pEmitter->m_pBlockLS, sizeof(ParticleBlock));
		if(pEmitter->m_pSpawnMesh)
			pEmitter->m_pSpawnMesh = pSpawnMeshEA;
		// dma back updated data
		pEmitter->m_pParentTransformLS = pEmitter->m_pParentTransformEA;
		pEmitter->m_pDefinition = pDefinitionEA;
		Spu::Store(pEmitterEA, pEmitter, sizeof(C_Emitter));
	#endif
	}
}

// spawn task
template<class CreatePolicy>
void SpawnTask(Scheduler::Task* pTask)
{
	EmitterUpdateConstants* pUpdateConstants = (EmitterUpdateConstants*)pTask->Input;
	EffectSpawnData* pEffectSpawnData = (EffectSpawnData*)pTask->ParamData;

#ifdef PLATFORM_SPU
	// fetch update constants and random
	pEffectSpawnData->pSpawningEmitter = (SpawningEmitter*)Spu::FetchAllocUnaligned(pEffectSpawnData->pSpawningEmitter, sizeof(SpawningEmitter) * pEffectSpawnData->emitterCount);
	pEffectSpawnData->pSpawnSettings = (SpawnSettings*)Spu::FetchAlloc(pEffectSpawnData->pSpawnSettings, sizeof(SpawnSettings));
	pEffectSpawnData->pSpawnSettings->pBlockManager = (VectorBlockManager<ParticleBlock>*)Spu::FetchAlloc(pEffectSpawnData->pSpawnSettings->pBlockManager, sizeof(VectorBlockManager<ParticleBlock>));
	pUpdateConstants = (EmitterUpdateConstants*)Spu::FetchAlloc(pUpdateConstants, sizeof(EmitterUpdateConstants));
	C_Random* pRandomEA = pUpdateConstants->pRandom[Scheduler::GetWorkerID()];
	C_Random* pRandom = (C_Random*)Spu::FetchAlloc(pRandomEA, sizeof(C_Random));
#else
	C_Random* pRandom = pUpdateConstants->pRandom[Scheduler::GetWorkerID()];
#endif

	Spawn<CreatePolicy>(pEffectSpawnData->pSpawningEmitter, pEffectSpawnData->pSpawnSettings, pEffectSpawnData->emitterCount, *pRandom, pUpdateConstants);

#ifdef PLATFORM_SPU
	// store random back to EA
	Spu::Store(pRandomEA, pRandom, sizeof(C_Random));
#endif
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_SPAWN_H
