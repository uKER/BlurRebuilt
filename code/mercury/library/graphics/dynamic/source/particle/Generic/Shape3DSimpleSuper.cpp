#include "stdafx.h"

// header
#include "Dynamic/Particle/Update.h"
#include "Dynamic/Particle/SuperEffect.h"

#define PARTICLE C_Particle3D_RV3D

namespace DynamicEffect
{
namespace Particle
{
	void Shape3DSimpleSuperUpdate(ParticleBlock** ppBlockEA, TrailPtrPtrs* pTrailBlocksEA, const SuperEffects* pSuperEffects, const PhysicsSettings* pPhysicsSettings, const EffectUpdateConstants* pUpdateConstants, U32 numBlocks)
	{
		PERF_EVENT("Particle.Shape3DSimpleSuperUpdate");

		ParticleBlock** ppBlock = ppBlockEA;
		
	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		XMVECTOR vTime = XMVectorReplicate(pUpdateConstants->timeStep);
		XMVECTOR vAcceleration = XMVectorMultiply(vTime, XMLoadFloat4A16((XMFLOAT4A16*)&pPhysicsSettings->vAcceleration));
		XMVECTOR vResistance = XMVectorReplicate(AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance));	// [Blur2PC]
		XMVECTOR vForce = XMLoadFloat4A16((XMFLOAT4A16*)&pUpdateConstants->globalForce);
	#else
		Maths::HF32 vTime = pUpdateConstants->timeStep;
		Maths::HVec3 vAcceleration = vTime * Maths::Load<Maths::HVec3>(&pPhysicsSettings->vAcceleration);
		Maths::HF32 vResistance = AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance);	// [Blur2PC]
		Maths::HVec3 vForce = Maths::Load<Maths::HVec3>(&pUpdateConstants->globalForce);
	#endif

		TrailPtrs trailPtrs;
		trailPtrs.Resize(pTrailBlocksEA->Size());

		for(U32 i = 0; i < numBlocks; ++i)
		{
			ParticleBlock* pBlock = ppBlock[i];
			ParticleBlock* pBlockEA = ppBlock[i];

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
				//working.UpdateForces(working2.forces);
				working.UpdateVelocity(vResistance, vAcceleration, vForce);
				working.UpdatePosition();
				working.UpdateLife();
				working.UpdateRotation();
		
				// if particle is dead, copy particle from last block into here
				if(working.IsDead())
				{
					UpdateDeaths(pSuperEffects, working.m_vPosition, working.m_qRotation, working.m_vVelocity, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					FreeParticle(p, pBlock, trailPtrs, pSuperEffects->pEmitterList);
				}
				else
				{
					UpdateTrails(pSuperEffects, p, trailPtrs, working.m_vPosition, working.m_qRotation, working.m_vVelocity, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					working.Store();
					p++;
				}
			}
			if(pBlock->m_Element.Size() == 0)
			{
				S32 queuePosition = MT::AtomicIncrement(pUpdateConstants->pDeletedSuperBlocksCount);

				// TODO: SPU this
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
	void Shape3DSimpleSuperUpdateTask(Scheduler::Task* pTask)
	{
		SuperEffectUpdateData* pUpdateData = (SuperEffectUpdateData*)pTask->Input;
		Shape3DSimpleSuperUpdate(pUpdateData->ppBlockEA, &pUpdateData->trailBlocksEA, pUpdateData->pSuperEffects, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_PPU_MODULE(ParticleShape3DSimpleSuperUpdateModule, DynamicEffect::Particle::Shape3DSimpleSuperUpdateTask)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

#include "Dynamic/Particle/SuperSpawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape3DSimpleSuperCreate : public DefaultCreationPolicy<Particle, Shape3DSimpleSuperCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape3DSimpleSuperCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
	ParticleUpdate<Particle> create;

	create.SetParticle(pParticle);
	create.CreatePositionLife(pEmitter->GetSpawnPosition(random));
	create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
	create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));
	create.CreateRotation(pEmitter->GetSpawnOrientation(random), pEmitter->GetSpawnAngularVelocity(random));

	if (pSpawnSettings->particleShape == E_ParticleShape_Mesh)
	{
		pParticle->iVariant = random.Rand(pSpawnSettings->meshCount);
		create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random), &pSpawnSettings->meshScale);
	}
	else
	{
		create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random));
		pParticle->iVariant = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
	}

	pParticle->iGameData0 = 0;
	pParticle->iUV = pEmitter->GetSpawnUV(random);
}

}
}

JQ_DECLARE_PPU_MODULE(ParticleShape3DSimpleSuperSpawnModule, DynamicEffect::Particle::SpawnSuperTask<DynamicEffect::Particle::Shape3DSimpleSuperCreate<DynamicEffect::Particle::PARTICLE> >)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

#include "Dynamic/Particle/Generic/Shape3DSimpleSuper.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
Shape3DSimpleSuper::Shape3DSimpleSuper(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleSuperEffect<C_Particle3D_RV3D, ShapePolicy::Shape3D, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape3DSimpleSuper)
{
	m_pUpdateModule = &ParticleShape3DSimpleSuperUpdateModule;
	m_pUpdateFunctor = Shape3DSimpleSuperUpdate;
	m_pSpawnModule = &ParticleShape3DSimpleSuperSpawnModule;
	m_pSpawnFunctor = SpawnSuper<Shape3DSimpleSuperCreate<PARTICLE> >;
}

Shape3DSimpleSuper::~Shape3DSimpleSuper()
{
}

}
}

#undef PARTICLE
















