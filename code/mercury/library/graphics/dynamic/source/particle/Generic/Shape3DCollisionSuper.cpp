#include "stdafx.h"

// header
#include "Dynamic/Particle/Update.h"
#include "Dynamic/Particle/SuperEffect.h"

#define PARTICLE C_Particle3D_RV3D_CP

namespace DynamicEffect
{
namespace Particle
{
	void Shape3DCollisionSuperUpdate(ParticleBlock** ppBlockEA, TrailPtrPtrs* pTrailBlocksEA, const SuperEffects* pSuperEffects, const PhysicsSettings* pPhysicsSettings, const EffectUpdateConstants* pUpdateConstants, U32 numBlocks)
	{
		PERF_EVENT("Particle.Shape3DCollisionSuperUpdate");

		ParticleBlock** ppBlock = ppBlockEA;
		
	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		XMVECTOR vTime = XMVectorReplicate(pUpdateConstants->timeStep);
		XMVECTOR vAcceleration = XMVectorMultiply(vTime, XMLoadFloat4A16((XMFLOAT4A16*)&pPhysicsSettings->vAcceleration));
		XMVECTOR vResistance = XMVectorReplicate(AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance));	// [Blur2PC]
		XMVECTOR vFriction = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(vTime, XMVectorReplicate(pPhysicsSettings->fFriction)));
		XMVECTOR vRestitution = XMVectorReplicate(pPhysicsSettings->fRestitution);
		XMVECTOR vSleepSpeedSq = XMVectorReplicate(pUpdateConstants->sleepSpeedSq);
		XMVECTOR vForce = XMLoadFloat4A16((XMFLOAT4A16*)&pUpdateConstants->globalForce);
	#else
		Maths::HF32 vTime = pUpdateConstants->timeStep;
		Maths::HVec3 vAcceleration = vTime * Maths::Load<Maths::HVec3>(&pPhysicsSettings->vAcceleration);
		Maths::HF32 vResistance = AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance);	// [Blur2PC]
		Maths::HF32 vFriction = 1.0f - vTime * pPhysicsSettings->fFriction;
		Maths::HF32 vRestitution = pPhysicsSettings->fRestitution;
		Maths::HF32 vSleepSpeedSq = pUpdateConstants->sleepSpeedSq;
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
			
			for(S32 a = 0; a < pBlock->m_Element.Size(); )
			{
				// prefetch data into cache lines
				DCBT(0,&pBlock->m_Element[a]);
				DCBT(128,&pBlock->m_Element[a]);

				PARTICLE* pParticle = (PARTICLE*)(&pBlock->m_Element[a]);
				
				working.SetParticle(pParticle);
				working.Load(vTime);

			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR vPlanePosition = working.m_vPlanePosition - working.m_vPlaneNormal * XMVectorSplatZ(working.m_vForceSizeLife);
			#else
				Maths::HVec3 vPlanePosition = working.m_vPlanePosition - working.m_vPlaneNormal * working.m_vForceSizeLife.Z();
			#endif
				
				if(!(working.m_iFlags & PARTICLE_COLLISION_SLEEP))
				{		
					working.UpdateVelocity(vResistance, vAcceleration, vForce);
					working.UpdatePosition();
					working.UpdateRotation();
					working.UpdateCollisions(vRestitution, vFriction, pPhysicsSettings->bDieOnImpact, vSleepSpeedSq, pUpdateConstants->pMaterials);
				}
				working.UpdateLife();

				// if particle is dead, copy particle from last block into here
				if(working.IsDead())
				{
					UpdateDeaths(pSuperEffects, working.m_vPosition, working.m_qRotation, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					FreeParticle(a, pBlock, trailPtrs, pSuperEffects->pEmitterList);
				}
				else
				{
					if(working.m_iFlags & PARTICLE_COLLISION_FIRST)
						UpdateCollisionsOnce(pSuperEffects, working.m_vPosition, working.m_qRotation, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					if(working.m_iFlags & PARTICLE_COLLISION)
						UpdateCollisions(pSuperEffects, working.m_vPosition, working.m_qRotation, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, vTime, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					UpdateTrails(pSuperEffects, a, trailPtrs, working.m_vPosition, working.m_qRotation, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					working.Store();
					a++;
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
				// push onto collideable blocks
				if(pPhysicsSettings->eCollision == E_Collision_Full)
				{
					S32 queuePosition = MT::AtomicIncrement(pUpdateConstants->pCollideableBlocksCount);
					if(queuePosition < pUpdateConstants->collideableBlocksCapacity)
						Spu::Set((INTPTR*)&pUpdateConstants->ppCollideableBlocks[queuePosition], (INTPTR)pBlockEA);	// [Blur2PC][x64] pointer-wide
				}

				working.StoreBounds(&pBlock->m_vCentre);
			#ifndef CFG_SPEED
				pBlock->m_WorkerID = Scheduler::GetWorkerID();
			#endif
				pBlock->m_VertexDataSize = pPhysicsSettings->vertexSize * pBlock->m_Element.Size();
			}
		}
	}

	// update task
	void Shape3DCollisionSuperUpdateTask(Scheduler::Task* pTask)
	{
		SuperEffectUpdateData* pUpdateData = (SuperEffectUpdateData*)pTask->Input;
		Shape3DCollisionSuperUpdate(pUpdateData->ppBlockEA, &pUpdateData->trailBlocksEA, pUpdateData->pSuperEffects, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_PPU_MODULE(ParticleShape3DCollisionSuperUpdateModule, DynamicEffect::Particle::Shape3DCollisionSuperUpdateTask)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

#include "Dynamic/Particle/SuperSpawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape3DCollisionSuperCreate : public DefaultCreationPolicy<Particle, Shape3DCollisionSuperCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape3DCollisionSuperCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
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
	create.CreateCollisionPlane(pEmitter->GetSpawnCollisionPlanePosition(), pEmitter->GetSpawnCollisionPlaneNormal());

	pParticle->iFlags = 0;
	pParticle->iGameData0 = 0;
	pParticle->iUV = pEmitter->GetSpawnUV(random);
}

}
}

JQ_DECLARE_PPU_MODULE(ParticleShape3DCollisionSuperSpawnModule, DynamicEffect::Particle::SpawnSuperTask<DynamicEffect::Particle::Shape3DCollisionSuperCreate<DynamicEffect::Particle::PARTICLE> >)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

#include "Dynamic/Particle/Generic/Shape3DCollisionSuper.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
Shape3DCollisionSuper::Shape3DCollisionSuper(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleSuperEffect<PARTICLE, ShapePolicy::Shape3D, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape3DCollisionSuper)
{
	m_pUpdateModule = &ParticleShape3DCollisionSuperUpdateModule;
	m_pUpdateFunctor = Shape3DCollisionSuperUpdate;
	m_pSpawnModule = &ParticleShape3DCollisionSuperSpawnModule;
	m_pSpawnFunctor = SpawnSuper<Shape3DCollisionSuperCreate<PARTICLE> >;
}

Shape3DCollisionSuper::~Shape3DCollisionSuper()
{
}



}
}

#undef PARTICLE














