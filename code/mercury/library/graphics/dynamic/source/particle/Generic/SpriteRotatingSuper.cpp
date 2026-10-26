#include "stdafx.h"

// header
#include "Dynamic/Particle/Update.h"
#include "Dynamic/Particle/SuperEffect.h"

#define PARTICLE C_Particle3D_R1D

namespace DynamicEffect
{
namespace Particle
{
	void SpriteRotatingSuperUpdate(ParticleBlock** ppBlockEA, TrailPtrPtrs* pTrailBlocksEA, const SuperEffects* pSuperEffects, const PhysicsSettings* pPhysicsSettings, const EffectUpdateConstants* pUpdateConstants, U32 numBlocks)
	{
		PERF_EVENT("Particle.SpriteRotatingSuperUpdate");

		ParticleBlock** ppBlock = ppBlockEA;
		
	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		XMVECTOR vTime = XMVectorReplicate(pUpdateConstants->timeStep);
		XMVECTOR vAcceleration = XMVectorMultiply(vTime, XMLoadFloat4A16((XMFLOAT4A16*)&pPhysicsSettings->vAcceleration));
		XMVECTOR vResistance = XMVectorReplicate(AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance));	// [Blur2PC]
		XMVECTOR qIdentity = XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorSelectControl(0, 0, 0, 1));
		XMVECTOR vForce = XMLoadFloat4A16((XMFLOAT4A16*)&pUpdateConstants->globalForce);
	#else
		Maths::HF32 vTime = pUpdateConstants->timeStep;
		Maths::HVec3 vAcceleration = vTime * Maths::Load<Maths::HVec3>(&pPhysicsSettings->vAcceleration);
		Maths::HF32 vResistance = AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance);	// [Blur2PC]
		Maths::C_Quaternion qIdentity;
		Maths::HVec3 vForce = Maths::Load<Maths::HVec3>(&pUpdateConstants->globalForce);
	#endif

		TrailPtrs trailPtrs;
		trailPtrs.Resize(pTrailBlocksEA->Size());

		for(U32 i = 0; i < numBlocks; ++i)
		{
			ParticleBlock* pBlock = ppBlock[i];
			ParticleBlock* pBlockEA = ppBlock[i];

			assert(pBlock->m_FrameIndex < pUpdateConstants->frameIndex);
			pBlock->m_FrameIndex = pUpdateConstants->frameIndex;

			for(S32 j = 0; j < trailPtrs.Size(); ++j)
				trailPtrs[j] = (*pTrailBlocksEA)[j][i];

			ParticleUpdate<PARTICLE>	working;
			
			for(S32 p = 0; p < pBlock->m_Element.Size(); )
			{
				// prefetch data into cache lines
				DCBT(0,&pBlock->m_Element[p]);
				DCBT(128,&pBlock->m_Element[p]);

				PARTICLE* pParticle = (PARTICLE*)(&pBlock->m_Element[p]);
				
				working.SetParticle(pParticle);
				working.Load(vTime);
				working.UpdateVelocity(vResistance, vAcceleration, vForce);
				working.UpdatePosition();
				working.UpdateLife();
				working.UpdateRotation();
			
				// if particle is dead, copy particle from last block into here
				if(working.IsDead())
				{
					UpdateDeaths(pSuperEffects, working.m_vPosition, qIdentity, working.m_vVelocity, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					FreeParticle(p, pBlock, trailPtrs, pSuperEffects->pEmitterList);
				}
				else
				{
					UpdateTrails(pSuperEffects, p, trailPtrs, working.m_vPosition, qIdentity, working.m_vVelocity, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					working.Store();
					p++;
				}
			}
			if(pBlock->m_Element.Size() == 0)
			{
				S32 queuePosition = MT::AtomicIncrement(pUpdateConstants->pDeletedSuperBlocksCount);

				if(queuePosition < pUpdateConstants->deletedSuperBlocksCapacity)
				{
					DeletedSuperBlock* pDeletedSuperBlock = &pUpdateConstants->pDeletedSuperBlocks[queuePosition];

					pDeletedSuperBlock->pBlock = pBlockEA;
					pDeletedSuperBlock->trailBlocks.Resize(trailPtrs.Size());
					for(S32 i = 0; i < trailPtrs.Size(); ++i)
					{
						pDeletedSuperBlock->trailBlocks[i] = trailPtrs[i];
					}
				}
			}
			else
			{
				working.StoreBounds(&pBlock->m_vCentre);
			#ifndef CFG_SPEED
				pBlock->m_WorkerID = Scheduler::GetWorkerID();
			#endif
				pBlock->m_VertexDataSize = pPhysicsSettings->vertexSize * pBlock->m_Element.Size();
			}
		}
	}
	
	// update task
	void SpriteRotatingSuperUpdateTask(Scheduler::Task* pTask)
	{
		SuperEffectUpdateData* pUpdateData = (SuperEffectUpdateData*)pTask->Input;
		SpriteRotatingSuperUpdate(pUpdateData->ppBlockEA, &pUpdateData->trailBlocksEA, pUpdateData->pSuperEffects, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_PPU_MODULE(ParticleSpriteRotatingSuperUpdateModule, DynamicEffect::Particle::SpriteRotatingSuperUpdateTask)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

#include "Dynamic/Particle/SuperSpawn.h"

namespace DynamicEffect
{
namespace Particle
{
	template<class Particle>
	class SpriteRotatingSuperCreate : public DefaultCreationPolicy<Particle, SpriteRotatingSuperCreate>
	{
	public:
		static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
	};

	//lint -e*
	template<class Particle>
	void SpriteRotatingSuperCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
	{
		ParticleUpdate<Particle> create;
		create.SetParticle(pParticle);
		create.CreatePositionLife(pEmitter->GetSpawnPosition(random));
		create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
		create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random));
		create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));

		pParticle->vRotation.x = pEmitter->GetSpawnRotation(random);
		pParticle->vRotation.y = pEmitter->GetSpawnRotationRate(random);
		pParticle->iVariant = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
		pParticle->iGameData0 = 0;
		pParticle->iUV = pEmitter->GetSpawnUV(random);
	}
}
}

JQ_DECLARE_PPU_MODULE(ParticleSpriteRotatingSuperSpawnModule, DynamicEffect::Particle::SpawnSuperTask<DynamicEffect::Particle::SpriteRotatingSuperCreate<DynamicEffect::Particle::PARTICLE> >)

#include "Dynamic/Particle/Generic/SpriteRotatingSuper.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{
// initialise system with correct values, then define custom creation and updating functions
SpriteRotatingSuper::SpriteRotatingSuper(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleSuperEffect<PARTICLE, ShapePolicy::SpriteRotating, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_SpriteRotatingSuper)
{
	m_pUpdateModule = &ParticleSpriteRotatingSuperUpdateModule;
	m_pUpdateFunctor = SpriteRotatingSuperUpdate;
	m_pSpawnModule = &ParticleSpriteRotatingSuperSpawnModule;
	m_pSpawnFunctor = SpawnSuper<SpriteRotatingSuperCreate<PARTICLE> >;
}

SpriteRotatingSuper::~SpriteRotatingSuper()
{
}


} // Particle
} // DynamicEffect

#undef PARTICLE











