#include "stdafx.h"

#define PARTICLE	C_Particle3D

#ifndef SPU_SPAWN

// update module
#include "Dynamic/Particle/Update.h"

namespace DynamicEffect
{
namespace Particle
{
	void Shape2DSimpleUpdate(ParticleBlock** ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, U32 numBlocks)
	{
		PERF_EVENT("Particle.UpdateShape2DSimple");
		
		ParticleBlock** ppBlock;
		const PhysicsSettings* pPhysicsSettings;
		const EffectUpdateConstants* pUpdateConstants;

		FetchInputs(ppBlockEA, pPhysicsSettingsEA, pUpdateConstantsEA, ppBlock, pPhysicsSettings, pUpdateConstants, numBlocks);

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

		// go through every block this system currently owns updating particles
		for(U32 i = 0; i < numBlocks; ++i)
		{
			Spu::MemBaseMarker marker;
			ParticleBlock* pBlock = ppBlock[i];
			ParticleBlock* pBlockEA = ppBlockEA[i];
			U8* pDataEA = pBlock->m_Element.Fetch();

			assert(pBlock->m_FrameIndex < pUpdateConstants->frameIndex);
			pBlock->m_FrameIndex = pUpdateConstants->frameIndex;

			ParticleUpdate<PARTICLE>	working;

			for(S32 a = 0; a < pBlock->m_Element.Size(); )
			{
				// prefetch data into cache lines
				DCBT(0,&pBlock->m_Element[a]);
				DCBT(128,&pBlock->m_Element[a]);

				PARTICLE* pParticle = (PARTICLE*)(&pBlock->m_Element[a]);
				
				working.SetParticle(pParticle);
				working.Load(vTime);
				working.UpdateVelocity(vResistance, vAcceleration, vForce);
				working.UpdatePosition();
				working.UpdateLife();
			
				// if particle is dead, copy particle from last block into here
				if(working.IsDead())
					pBlock->FreeParticle(a);
				else
				{
					working.Store();
					a++;
				}
			}
			if(pBlock->m_Element.Size() == 0)
			{
				S32 queuePosition = MT::AtomicIncrement(pUpdateConstants->pDeletedBlocksCount);
				if(queuePosition < pUpdateConstants->deletedBlocksCapacity)
					Spu::Set((INTPTR*)&pUpdateConstants->ppDeletedBlocks[queuePosition], (INTPTR)pBlockEA);	// [Blur2PC][x64] pointer-wide
			}
			else
			{
				working.StoreBounds(&pBlock->m_vCentre);
			#ifndef CFG_SPEED
				pBlock->m_WorkerID = Scheduler::GetWorkerID();
			#endif
				pBlock->m_VertexDataSize = pPhysicsSettings->vertexSize * pBlock->m_Element.Size();

				for(S32 a = 0; a < pBlock->m_Element.Size(); ++a)
				{
					const C_Particle3D* pParticle = (C_Particle3D*)&pBlock->m_Element[a];
					assert(pParticle->fLife >= 0.0f && pParticle->fLife <= 1.0f);
				}
			}
			
		#ifdef PLATFORM_SPU
			pBlock->m_Element.Store(pDataEA);
			Spu::StoreAsync(pBlockEA, pBlock, sizeof(ParticleBlock), 0);
		#endif
		}
	}

	// update task
	void Shape2DSimpleUpdateTask(Scheduler::Task* pTask)
	{
		EffectUpdateData* pUpdateData = (EffectUpdateData*)pTask->ParamData;
		Shape2DSimpleUpdate((ParticleBlock**)pTask->Input, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}

} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_SPU_MODULE(ParticleShape2DSimpleUpdateModule, DynamicEffect::Particle::Shape2DSimpleUpdateTask, dynamic_particle_update_shape2dsimple, "dynamic.scheduler");

#endif

// if spu and spawn module
#if (defined(PLATFORM_SPU) && defined(SPU_SPAWN)) || !defined(PLATFORM_SPU)

#include "Dynamic/Particle/Spawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape2DSimpleCreate : public DefaultCreationPolicy<Particle, Shape2DSimpleCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape2DSimpleCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
	ParticleUpdate<Particle> create;
	create.SetParticle(pParticle);
	create.CreatePositionLife(pEmitter->GetSpawnPosition(random));
	create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
	create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random));
	create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));
	pParticle->iVariant = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
	pParticle->iGameData0 = 0;
	pParticle->iUV = pEmitter->GetSpawnUV(random);
}

}
}

// spawn module
JQ_DECLARE_SPU_MODULE(ParticleShape2DSimpleSpawnModule, DynamicEffect::Particle::SpawnTask<DynamicEffect::Particle::Shape2DSimpleCreate<DynamicEffect::Particle::PARTICLE> >, dynamic_particle_spawn_shape2dsimple, "dynamic.scheduler");

#endif

#ifndef PLATFORM_SPU

#include "Dynamic/Particle/Generic/Shape2DSimple.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
Shape2DSimple::Shape2DSimple(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleEffect<PARTICLE, ShapePolicy::Shape2D, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape2DSimple)
{
	m_pUpdateModule = &ParticleShape2DSimpleUpdateModule;
	m_pUpdateFunctor = Shape2DSimpleUpdate;
	m_pSpawnModule = &ParticleShape2DSimpleSpawnModule;
	m_pSpawnFunctor = Spawn<Shape2DSimpleCreate<PARTICLE> >;
}

Shape2DSimple::~Shape2DSimple()
{
}

} // Particle
} // DynamicEffect

#endif

#undef PARTICLE
















