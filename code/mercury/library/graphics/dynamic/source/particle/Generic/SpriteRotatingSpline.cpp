#include "stdafx.h"

#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>
#include "Dynamic/Particle/Update.h"

#define PARTICLE C_Particle3D_R1D_P

namespace DynamicEffect
{
namespace Particle
{
	void SpriteRotatingSplineUpdate(ParticleBlock** ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, U32 numBlocks)
	{
		PERF_EVENT("Particle.UpdateSpriteRotatingSpline");

		ParticleBlock** ppBlock;
		const PhysicsSettings* pPhysicsSettings;
		const EffectUpdateConstants* pUpdateConstants;

		FetchInputs(ppBlockEA, pPhysicsSettingsEA, pUpdateConstantsEA, ppBlock, pPhysicsSettings, pUpdateConstants, numBlocks);

	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		XMVECTOR vTime = XMVectorReplicate(pUpdateConstants->timeStep);
		XMVECTOR vAcceleration = XMVectorMultiply(vTime, XMLoadFloat4A16((XMFLOAT4A16*)&pPhysicsSettings->vAcceleration));
		XMVECTOR vResistance = XMVectorReplicate(AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance));	// [Blur2PC]
		XMVECTOR vForce = XMLoadFloat4A16((XMFLOAT4A16*)&pUpdateConstants->globalForce);
		XMVECTOR vZero = XMVectorZero();
	#else
		Maths::HF32 vTime = pUpdateConstants->timeStep;
		Maths::HVec3 vAcceleration = vTime * Maths::Load<Maths::HVec3>(&pPhysicsSettings->vAcceleration);
		Maths::HF32 vResistance = AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance);	// [Blur2PC]
		Maths::HVec3 vZero(Maths::Constants<Maths::HF32>::Zero());
		Maths::HVec3 vForce = Maths::Load<Maths::HVec3>(&pUpdateConstants->globalForce);
	#endif

		// go through every block this system currently owns updating particles
		for(U32 i = 0; i < numBlocks; ++i)
		{
			ParticleBlock* pBlock = ppBlock[i];
			ParticleBlock* pBlockEA = ppBlockEA[i];
			U8* pDataEA = pBlock->m_Element.Fetch();
			Curve::C_ActorCurve3D* pCurve = (Curve::C_ActorCurve3D*)pBlock->m_pInfo;

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
				if(pCurve)
					working.UpdatePosition(pCurve->Evaluate(1.0f - pParticle->fLife));
				else
					working.UpdatePosition(vZero);
				working.UpdateLife();
				working.UpdateRotation();
		
				// if particle is dead, copy particle from last block into here
				if(working.IsDead())
					pBlock->FreeParticle(a);
				else
				{
					working.Store();
					++a;
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
	void SpriteRotatingSplineUpdateTask(Scheduler::Task* pTask)
	{
		EffectUpdateData* pUpdateData = (EffectUpdateData*)pTask->ParamData;
		SpriteRotatingSplineUpdate((ParticleBlock**)pTask->Input, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_PPU_MODULE(ParticleSpriteRotatingSplineUpdateModule, DynamicEffect::Particle::SpriteRotatingSplineUpdateTask);

// header
#include "Dynamic/Particle/Generic/SpriteRotatingSpline.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class SpriteRotatingSplineCreate : public DefaultCreationPolicy<Particle, SpriteRotatingSplineCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
	static void CreateBlock(ParticleBlock* pBlock, const C_Emitter* pEmitter);
};

//lint -e*
template<class Particle>
void SpriteRotatingSplineCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
	ParticleUpdate<Particle> create;

	create.SetParticle(pParticle);
	if(pEmitter->GetSpline())
		create.CreatePositionLife(pEmitter->GetSpawnPosition(random), pEmitter->GetSpline()->GetCurve()->Evaluate(0.0f));
	else
	{
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		create.CreatePositionLife(pEmitter->GetSpawnPosition(random), XMVectorZero());
#else
		create.CreatePositionLife(pEmitter->GetSpawnPosition(random), Maths::Constants<Maths::HVec3>::Zero());
#endif
	}
	create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
	create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random));
	create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));

	pParticle->vRotation.x = pEmitter->GetSpawnRotation(random);
	pParticle->vRotation.y = pEmitter->GetSpawnRotationRate(random);
	pParticle->iVariant = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
	pParticle->iGameData0 = 0;
	pParticle->iUV = pEmitter->GetSpawnUV(random);
}

template<class Particle>
void SpriteRotatingSplineCreate<Particle>::CreateBlock(ParticleBlock* pBlock, const C_Emitter* pEmitter)
{
	pBlock->m_pInfo = (void*)pEmitter->GetSpline();
}

}
}

// spawn module
JQ_DECLARE_PPU_MODULE(ParticleSpriteRotatingSplineSpawnModule, DynamicEffect::Particle::SpawnTask<DynamicEffect::Particle::SpriteRotatingSplineCreate<DynamicEffect::Particle::PARTICLE> >);//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
SpriteRotatingSpline::SpriteRotatingSpline(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleEffect<PARTICLE, ShapePolicy::SpriteRotating, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_SpriteRotatingSpline)
{
	m_pUpdateModule = &ParticleSpriteRotatingSplineUpdateModule;
	m_pUpdateFunctor = SpriteRotatingSplineUpdate;
	m_pSpawnModule = &ParticleSpriteRotatingSplineSpawnModule;
	m_pSpawnFunctor = Spawn<SpriteRotatingSplineCreate<PARTICLE> >;
}

SpriteRotatingSpline::~SpriteRotatingSpline()
{
}





}
}

#undef PARTICLE



















