#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SUPERSPAWN_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SUPERSPAWN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Defines.h"
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
#include "Dynamic/Particle/UpdateXM.h"
#else
#include "Dynamic/Particle/UpdateHV.h"
#endif

#include "Dynamic/Particle/Super.h"
#include "Dynamic/Particle/SpawnSettings.h"
#include "Dynamic/Particle/UpdateData.h"
#include "Dynamic/BlockManager.h"

namespace DynamicEffect
{
namespace Particle
{

void*				NewParticle(ParticleBlock* pBlock, TrailPtrs& trailPtrs, SuperEffects* pSuperEffects);
void*				NewParticles(ParticleBlock* pBlock, TrailPtrs& trailPtrs, S32 num, SuperEffects* pSuperEffects);
void				CreateTrails(ParticleBlock* pBlock, TrailPtrs& trailPtrs, const MathsStorage::Float4A16& vPosition, const MathsStorage::Float4A16& vVelocity, const C_Node* pParentTransform);
ParticleBlock*		BeginCreateSuperExisting(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, TrailPtrs& trailPtrs, const EmitterUpdateConstants* pUpdateConstants);
ParticleBlock*		BeginCreateSuperNew(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, TrailPtrs& trailPtrs, const EmitterUpdateConstants* pUpdateConstants);

template<class CreationPolicy>
ParticleBlock* BeginCreateSuperT(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, TrailPtrs& trailPtrs, const EmitterUpdateConstants* pUpdateConstants)
{
	if(ParticleBlock* pParticleBlock = BeginCreateSuperExisting(nNumEmit, nTotalEmit, pEmitter, pSpawnSettings, pSuperEffects, trailPtrs, pUpdateConstants))
		return pParticleBlock;

	if(ParticleBlock* pParticleBlock = BeginCreateSuperNew(nNumEmit, nTotalEmit, pEmitter, pSpawnSettings, pSuperEffects, trailPtrs, pUpdateConstants))
	{
		CreationPolicy::CreateBlock((ParticleBlock*)pParticleBlock, pEmitter);
		return pParticleBlock;
	}

	return NULL;
}

template<class CreationPolicy>
void SpawnSuperT(C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, C_Random& random, const EmitterUpdateConstants* pUpdateConstants)
{
	// check to see if our block is still alive, if not invalidate pointer
	if(ParticleBlock* pBlock = pEmitter->GetBlock())
	{
		// check to see if block is still owned by us, or if block has more than max particles
		if(pBlock->m_pEffect != pSpawnSettings->pEffect || pBlock->m_iSalt != pEmitter->GetBlockSalt())
			pEmitter->SetBlock(NULL);
	}

	F32 cullDistance = pEmitter->GetDefinition()->cullDistance != 0.0f ? pEmitter->GetDefinition()->cullDistance : pSpawnSettings->cullDistance;
	U32 nNumEmit = 0;
	U32 nTotalEmit = pEmitter->GetSpawnAmount();
	ParticleBlock* pBlock;
	TrailPtrs trailPtrs;
	trailPtrs.Resize(pSuperEffects->trailEffects.Size());

	const C_Node* pParentTransform;
	if(pEmitter->GetDefinition()->eSpace == E_EmitterSpace_Local)
		pParentTransform = pEmitter->GetParentTransform();
	else
		pParentTransform = NULL;

	// spawn blocks
	while(nTotalEmit > 0)
	{
		if((pBlock = BeginCreateSuperT<CreationPolicy>(nNumEmit, nTotalEmit, pEmitter, pSpawnSettings, pSuperEffects, trailPtrs, pUpdateConstants)))
		{
			// emit the remaining particles for this update
			for(U32 i = 0; i < nNumEmit; ++i)
			{
				// grab the item on end of current block list
				if(U8* pParticleData = (U8*)NewParticle(pBlock, trailPtrs, pSuperEffects))
				{
					CreationPolicy::Base::CreateParticle(pParticleData, random, pEmitter, pSpawnSettings);
					C_Particle3D* pParticle = (C_Particle3D*)pParticleData;
					CreateTrails(pBlock, trailPtrs, (MathsStorage::Float4A16&)pParticle->vPosition, (MathsStorage::Float4A16&)pParticle->vVelocity, pParentTransform);
				}
			}
			pBlock->CalculateExtents(cullDistance, pSpawnSettings->vertexSize);
		}
	}
}

template<class CreatePolicy>
void SpawnSuper(SpawningEmitter* pSpawningEmitter, SpawnSettings* pSpawnSettings, S32 count, C_Random& random, const EmitterUpdateConstants* pUpdateConstants)
{
	PERF_EVENT("Particle.SpawnSuper");

	for(S32 i = 0; i < count; ++i)
	{
		C_Emitter* pEmitter = pSpawningEmitter[i].pEmitter;
		SpawnSuperT< CreatePolicy >(pEmitter, pSpawnSettings, pSpawnSettings->pSuperEffects, random, pUpdateConstants);
	}
}

template<class CreatePolicy>
void SpawnSuperTask(Scheduler::Task* pTask)
{
	EmitterUpdateConstants* pUpdateConstants = (EmitterUpdateConstants*)pTask->Input;
	EffectSpawnData* pEffectSpawnData = (EffectSpawnData*)pTask->ParamData;

	SpawnSuper<CreatePolicy>(pEffectSpawnData->pSpawningEmitter, pEffectSpawnData->pSpawnSettings, pEffectSpawnData->emitterCount, *pUpdateConstants->pRandom[Scheduler::GetWorkerID()], pUpdateConstants);
}



} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_SUPEREFFECT_H
