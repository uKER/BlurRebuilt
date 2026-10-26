#include "stdafx.h"

#include "Dynamic/Particle/SuperSpawn.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/BlockManager.h"
#include "Dynamic/Particle/EffectInterface.h"	// [Blur2PC] I_ParticleEffectBase -> I_DynamicEffect conversion needs the full type

namespace DynamicEffect
{
namespace Particle
{

void* NewParticle(ParticleBlock* pBlock, TrailPtrs& trailPtrs, SuperEffects* pSuperEffects)
{
	void* pParticle = (void*)pBlock->m_Element.PushBack();

	// push back particles onto trail blocks
	for(S32 j = 0; j < trailPtrs.Size(); ++j)
	{
		assert((trailPtrs[j]->m_Element.Size() == pBlock->m_Element.Size() - 1));
		C_Particle_E* pTrailParticle = (C_Particle_E*)trailPtrs[j]->m_Element.PushBack();
		if((pTrailParticle->pEmitter = pSuperEffects->pEmitterList->AtomicAllocate()))
		{
			pTrailParticle->iSalt = pTrailParticle->pEmitter->GetSalt();
			pTrailParticle->pEmitter->SetDefinition(&pSuperEffects->trailEffects[j].definition);
			pTrailParticle->pEmitter->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}
	return pParticle;
}

void* NewParticles(ParticleBlock* pBlock, TrailPtrs& trailPtrs, S32 num, SuperEffects* pSuperEffects)
{
	void* pParticle = (void*)pBlock->m_Element.PushBack(num);

	for(S32 p = 0; p < num; ++p)
	{
		// push back particles onto trail blocks
		for(S32 t = 0; t < trailPtrs.Size(); ++t)
		{
			assert((trailPtrs[t]->m_Element.Size() == pBlock->m_Element.Size() - 1));
			C_Particle_E* pTrailParticle = (C_Particle_E*)trailPtrs[t]->m_Element.PushBack();
			if((pTrailParticle->pEmitter = pSuperEffects->pEmitterList->AtomicAllocate()))
			{
				pTrailParticle->iSalt = pTrailParticle->pEmitter->GetSalt();
				pTrailParticle->pEmitter->SetDefinition(&pSuperEffects->trailEffects[t].definition);
				pTrailParticle->pEmitter->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
			}
		}
	}
	return pParticle;
}

ParticleBlock* BeginCreateSuperExisting(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, TrailPtrs& trailPtrs, const EmitterUpdateConstants* pUpdateConstants)
{
	// check to see if we can fit creation inside any current blocks
	if(ParticleBlock* pBlock = pEmitter->GetBlock())
	{
		assert(pBlock->m_pEffect);
		assert(pBlock->m_Element.Remaining() > 0);
		// search active blocks for space to fit this creation
		Maths::HVec3 vBlock(Maths::Load<Maths::HVec3>(&pEmitter->GetPosition()));
		Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>(&pBlock->m_vCentre));
		vCentre -= vBlock;
		// only return current block if its within 5 metres of centre of block
		if(Maths::Length2(vCentre) < 25.f)
		{
			U32 remaining = (U32) pBlock->m_Element.Remaining();
			if(nTotalEmit < remaining)
			{
				// theres more than enough space to fit this emission
				nNumEmit = nTotalEmit;
				nTotalEmit = 0;
			}
			else
			{
				// emission is equal or greater than remaining space - set emitter block to NULL
				nTotalEmit -= remaining;
				nNumEmit = remaining;
				pEmitter->SetBlock(NULL);
			}

			// go through all trail effects and make sure trail iterator points to the right block
			for(S32 j = 0; j < pSuperEffects->trailEffects.Size(); ++j)
			{
				trailPtrs[j] = pSuperEffects->trailEffects[j].blocks[pBlock->m_VectorPosition];
				assert(trailPtrs[j]->m_Element.Size() == pEmitter->GetBlock()->m_Element.Size());
			}
			
			return pBlock;
		}
	}

	return NULL;
}

ParticleBlock* BeginCreateSuperNew(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, TrailPtrs& trailPtrs, const EmitterUpdateConstants* pUpdateConstants)
{
	// we either don't have any blocks or couldn't fit creation in any so request a new one	
	S32 queuePosition = MT::AtomicIncrement(pSpawnSettings->pBlocks);

	if(queuePosition == 0)
	{
		S32 activedPosition = MT::AtomicIncrement(pUpdateConstants->pActivedSuperEffectsCount);
		pUpdateConstants->ppActivedSuperEffects[activedPosition] = (I_ParticleSuperEffect*)pSpawnSettings->pEffect;
	}

	if(queuePosition < pSpawnSettings->blocks.Capacity())
	{	
		if(ParticleBlock* pBlock = pSpawnSettings->pBlockManager->NewBlock())
		{
		#ifndef CFG_SPEED
			S32 currentAllocated = MT::AtomicIncrement(pSpawnSettings->pAllocated);
		#endif
			assert(currentAllocated < pSpawnSettings->blocks.Capacity());
			assert((pBlock->m_Element.Size() == 0)); 
			
			pBlock->Clear();
			pBlock->SetNodeType(pSpawnSettings->nodeType);
			pBlock->m_fTime = pUpdateConstants->time;
			pBlock->m_fExpiryTime = pEmitter->GetDefinition()->fExpiryTime;
			pBlock->m_pEffect = pSpawnSettings->pEffect;
			pBlock->m_iFlags = 0;
			pBlock->m_vLodFade = pEmitter->GetDefinition()->lodFade.x != 0.0f ? pEmitter->GetDefinition()->lodFade : pSpawnSettings->lodFade;
			pBlock->m_VectorPosition = queuePosition;
			
			pSpawnSettings->blocks[queuePosition] = pBlock;
			
			if(pEmitter->GetDefinition()->eSpace == E_EmitterSpace_Local)
			{
				if((pBlock->m_pNode = pEmitter->GetParentTransform()))
					++(*pSpawnSettings->pBlocksLocal);
			}
			else
				pBlock->m_pNode = NULL;

			for(S32 t = 0; t < pSuperEffects->trailEffects.Size(); ++t)
			{
				if(ParticleTrailBlock* pTrailBlock = pSuperEffects->pTrailBlockManager->NewBlock())
				{
					trailPtrs[t] = pTrailBlock;
					assert(pTrailBlock->m_Element.Size() == 0);
					pTrailBlock->m_pEffect = (I_ParticleSuperEffect*)pSpawnSettings->pEffect;
					pTrailBlock->m_EffectIndex = t;
					pTrailBlock->m_VectorPosition = queuePosition;
					
					pSuperEffects->trailEffects[t].blocks[queuePosition] = pTrailBlock;
				}
				else
					pSuperEffects->trailEffects[t].blocks[queuePosition] = NULL;
			}

			// work out whether we emit a full blocks worth or not
			if(nTotalEmit < pSpawnSettings->pBlockManager->GetBlockSize())
			{
				nNumEmit = nTotalEmit;
				nTotalEmit = 0;
				pEmitter->SetBlock(pBlock);
			}
			else
			{
				nNumEmit = pSpawnSettings->pBlockManager->GetBlockSize();
				nTotalEmit -= nNumEmit;
				pEmitter->SetBlock(NULL);
			}

			return pBlock;
		}
		else
		{
			pSpawnSettings->blocks[queuePosition] = NULL;
			for(S32 t = 0; t < pSuperEffects->trailEffects.Size(); ++t)
				pSuperEffects->trailEffects[t].blocks[queuePosition] = NULL;
		}
	}

	nNumEmit = 0;
	nTotalEmit = 0;
	return NULL;
}

// create trail emitter starting positions
void CreateTrails(ParticleBlock* pBlock, TrailPtrs& trailPtrs, const MathsStorage::Float4A16& vPosition, const MathsStorage::Float4A16& vVelocity, const C_Node* pParentTransform)
{
	// set the position of the associated particle emitters
	S32 i = pBlock->m_Element.Size() - 1;
	for(S32 j = 0; j < trailPtrs.Size(); ++j)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&trailPtrs[j]->m_Element[i];
		if(pParticle->pEmitter)
		{
			pParticle->pEmitter->SetTransform(C_Node(vPosition, MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f)), pParentTransform);
			pParticle->pEmitter->SetPositionPrevious(vPosition);
			pParticle->pEmitter->SetVelocity(vVelocity);
			pParticle->pEmitter->SetCalculatePositions(false);
		}
	}
}

} // Particle
} // DynamicEffect
