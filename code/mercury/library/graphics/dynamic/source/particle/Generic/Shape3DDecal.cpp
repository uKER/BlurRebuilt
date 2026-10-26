#include "stdafx.h"

#include "Dynamic/Particle/Update.h"

#define PARTICLE	C_Particle3D_RV3D

namespace DynamicEffect
{
namespace Particle
{
	void Shape3DDecalUpdate(ParticleBlock** ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, U32 numBlocks)
	{
		PERF_EVENT("Particle.UpdateShape3DSimple");
		
		ParticleBlock** ppBlock;
		const PhysicsSettings* pPhysicsSettings;
		const EffectUpdateConstants* pUpdateConstants;

		FetchInputs(ppBlockEA, pPhysicsSettingsEA, pUpdateConstantsEA, ppBlock, pPhysicsSettings, pUpdateConstants, numBlocks);
		
	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		XMVECTOR vTime = XMVectorReplicate(pUpdateConstants->timeStep);
	#else
		Maths::HF32 vTime = pUpdateConstants->timeStep;
	#endif
		
		for(U32 i = 0; i < numBlocks; ++i)
		{
			ParticleBlock* pBlock = ppBlock[i];
			ParticleBlock* pBlockEA = ppBlockEA[i];

			U8* pDataEA = pBlock->m_Element.Fetch();

			ParticleUpdate<PARTICLE>	working;

			for(S32 a = 0; a < pBlock->m_Element.Size(); )
			{
				// prefetch data into cache lines
				DCBT(0, &pBlock->m_Element[a]);
				DCBT(128, &pBlock->m_Element[a]);

				PARTICLE* pParticle = (PARTICLE*)(&pBlock->m_Element[a]);
				
				working.SetParticle(pParticle);
				working.Load(vTime);
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
			}
		}
	}

	// update task
	void Shape3DDecalUpdateTask(Scheduler::Task* pTask)
	{
		EffectUpdateData* pUpdateData = (EffectUpdateData*)pTask->ParamData;
		Shape3DDecalUpdate((ParticleBlock**)pTask->Input, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
}
}

// update module
JQ_DECLARE_PPU_MODULE(ParticleShape3DDecalUpdateModule, DynamicEffect::Particle::Shape3DDecalUpdateTask);

#include "Dynamic/Particle/Spawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape3DDecalCreate : public DefaultCreationPolicy<Particle, Shape3DDecalCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape3DDecalCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
	ParticleUpdate<Particle> create;

	create.SetParticle(pParticle);
	create.CreatePositionLife(pEmitter->GetSpawnPosition(random));
	create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
	create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));
	create.CreateRotation(pEmitter->GetSpawnOrientation(random), pEmitter->GetSpawnAngularVelocity(random));
	create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random));

	pParticle->iVariant = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
	pParticle->iGameData0 = 0;
	pParticle->iUV = pEmitter->GetSpawnUV(random);
}

}
}

// spawn module
JQ_DECLARE_PPU_MODULE(ParticleShape3DDecalSpawnModule, DynamicEffect::Particle::SpawnTask<DynamicEffect::Particle::Shape3DDecalCreate<DynamicEffect::Particle::PARTICLE> >);

// header
#include "Dynamic/Particle/Generic/Shape3DDecal.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{
// initialise system with correct values, then define custom creation and updating functions
Shape3DDecal::Shape3DDecal(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleEffect<PARTICLE, ShapePolicy::Decal, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape3DDecal)
{
	m_pUpdateModule = &ParticleShape3DDecalUpdateModule;
	m_pUpdateFunctor = Shape3DDecalUpdate;
	m_pSpawnModule = &ParticleShape3DDecalSpawnModule;
	m_pSpawnFunctor = Spawn<Shape3DDecalCreate<PARTICLE> >;	
}

Shape3DDecal::~Shape3DDecal()
{
}

}
}

#undef PARTICLE

















