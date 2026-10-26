#include "stdafx.h"

// header
#include "Dynamic/Particle/Update.h"
#include "Dynamic/Particle/SuperEffect.h"

#define PARTICLE C_Particle3D_CP

namespace DynamicEffect
{
namespace Particle
{
	void Shape2DCollisionSuperUpdate(ParticleBlock** ppBlockEA, TrailPtrPtrs* pTrailBlocksEA, const SuperEffects* pSuperEffects, const PhysicsSettings* pPhysicsSettings, const EffectUpdateConstants* pUpdateConstants, U32 numBlocks)
	{
		PERF_EVENT("Particle.Shape2DCollisionSuperUpdate");

		ParticleBlock** ppBlock = ppBlockEA;
		
	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
		XMVECTOR vTime = XMVectorReplicate(pUpdateConstants->timeStep);
		XMVECTOR vAcceleration = XMVectorMultiply(vTime, XMLoadFloat4A16((XMFLOAT4A16*)&pPhysicsSettings->vAcceleration));
		XMVECTOR vResistance = XMVectorReplicate(AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance));	// [Blur2PC]
		XMVECTOR vFriction = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(vTime, XMVectorReplicate(pPhysicsSettings->fFriction)));
		XMVECTOR vRestitution = XMVectorReplicate(pPhysicsSettings->fRestitution);
		XMVECTOR qIdentity = XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorSelectControl(0, 0, 0, 1));
		XMVECTOR vSleepSpeedSq = XMVectorReplicate(pUpdateConstants->sleepSpeedSq);
		XMVECTOR vForce = XMLoadFloat4A16((XMFLOAT4A16*)&pUpdateConstants->globalForce);
	#else
		Maths::HF32 vTime = pUpdateConstants->timeStep;
		Maths::HVec3 vAcceleration = vTime * Maths::Load<Maths::HVec3>(&pPhysicsSettings->vAcceleration);
		Maths::HF32 vResistance = AirResistanceFactor(pUpdateConstants->timeStep, pPhysicsSettings->fAirResistance);	// [Blur2PC]
		Maths::HF32 vFriction = 1.0f - vTime * pPhysicsSettings->fFriction;
		Maths::HF32 vRestitution = pPhysicsSettings->fRestitution;
		Maths::C_Quaternion qIdentity;
		Maths::HF32 vSleepSpeedSq = pUpdateConstants->sleepSpeedSq;
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
			
			for(S32 a = 0; a < pBlock->m_Element.Size(); )
			{
				// prefetch data into cache lines
				DCBT(0,&pBlock->m_Element[a]);
				DCBT(128,&pBlock->m_Element[a]);

				PARTICLE* pParticle = (PARTICLE*)(&pBlock->m_Element[a]);
				
				working.SetParticle(pParticle);
				working.Load(vTime);
				
				if(!(working.m_iFlags & PARTICLE_COLLISION_SLEEP))
				{	
					working.UpdateVelocity(vResistance, vAcceleration, vForce);
					working.UpdatePosition();
					working.UpdateCollisions(vRestitution, vFriction, pPhysicsSettings->bDieOnImpact, vSleepSpeedSq, pUpdateConstants->pMaterials);
				}
				working.UpdateLife();

			#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
				XMVECTOR vPlanePosition = working.m_vPlanePosition - working.m_vPlaneNormal * XMVectorSplatZ(working.m_vForceSizeLife);
			#else
				Maths::HVec3 vPlanePosition = working.m_vPlanePosition - working.m_vPlaneNormal * working.m_vForceSizeLife.Z();
			#endif
				
				// if particle is dead, copy particle from last block into here
				if(working.IsDead())
				{
					UpdateDeaths(pSuperEffects, working.m_vPosition, qIdentity, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, pBlock->m_pNode, pBlock->m_VisibilityFlags);
					FreeParticle(a, pBlock, trailPtrs, pSuperEffects->pEmitterList);
				}
				else
				{
					if(working.m_iFlags & PARTICLE_COLLISION_ONCE && pSuperEffects->collisionOnceEffects.Size() > 0)
					{
						UpdateCollisionsOnce(pSuperEffects, working.m_vPosition, qIdentity, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, pBlock->m_pNode, pBlock->m_VisibilityFlags);
						UpdateDeaths(pSuperEffects, working.m_vPosition, qIdentity, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, pBlock->m_pNode, pBlock->m_VisibilityFlags);
						FreeParticle(a, pBlock, trailPtrs, pSuperEffects->pEmitterList);
					}
					else
					{
						if(working.m_iFlags & PARTICLE_COLLISION)
							UpdateCollisions(pSuperEffects, working.m_vPosition, qIdentity, working.m_vVelocity, vPlanePosition, working.m_vPlaneNormal, vTime, pBlock->m_pNode, pBlock->m_VisibilityFlags);
						UpdateTrails(pSuperEffects, a, trailPtrs, working.m_vPosition, qIdentity, working.m_vVelocity, pBlock->m_pNode, pBlock->m_VisibilityFlags);
						working.Store();
						a++;
					}
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
	void Shape2DCollisionSuperUpdateTask(Scheduler::Task* pTask)
	{
		SuperEffectUpdateData* pUpdateData = (SuperEffectUpdateData*)pTask->Input;
		Shape2DCollisionSuperUpdate(pUpdateData->ppBlockEA, &pUpdateData->trailBlocksEA, pUpdateData->pSuperEffects, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_PPU_MODULE(ParticleShape2DCollisionSuperUpdateModule, DynamicEffect::Particle::Shape2DCollisionSuperUpdateTask)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

#include "Dynamic/Particle/SuperSpawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape2DCollisionSuperCreate : public DefaultCreationPolicy<Particle, Shape2DCollisionSuperCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape2DCollisionSuperCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
	ParticleUpdate<Particle> create;

	create.SetParticle(pParticle);
	create.CreatePositionLife(pEmitter->GetSpawnPosition(random));
	create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
	create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));
	create.CreateForceSizeLife(pEmitter->GetSpawnForceSizeLife(random));
	create.CreateCollisionPlane(pEmitter->GetSpawnCollisionPlanePosition(), pEmitter->GetSpawnCollisionPlaneNormal());

	pParticle->iFlags = 0;
	pParticle->iVariant = random.Rand(pSpawnSettings->atlasSize) + pSpawnSettings->atlasOffset;
	pParticle->iGameData0 = 0;
	pParticle->iUV = pEmitter->GetSpawnUV(random);
}

}
}

// header
#include "Dynamic/Particle/Generic/Shape2DCollisionSuper.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Particle/Update.h"

JQ_DECLARE_PPU_MODULE(ParticleShape2DCollisionSuperSpawnModule, DynamicEffect::Particle::SpawnSuperTask<DynamicEffect::Particle::Shape2DCollisionSuperCreate<DynamicEffect::Particle::PARTICLE> >)//, dynamic_shape2dcollisionupdate, "dynamic.scheduler");

namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
Shape2DCollisionSuper::Shape2DCollisionSuper(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleSuperEffect<PARTICLE, ShapePolicy::Shape2D, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape2DCollisionSuper)
{
	m_pUpdateModule = &ParticleShape2DCollisionSuperUpdateModule;
	m_pUpdateFunctor = Shape2DCollisionSuperUpdate;
	m_pSpawnModule = &ParticleShape2DCollisionSuperSpawnModule;
	m_pSpawnFunctor = SpawnSuper<Shape2DCollisionSuperCreate<PARTICLE> >;
}

Shape2DCollisionSuper::~Shape2DCollisionSuper()
{
}

} // Particle
} // DynamicEffect

#undef PARTICLE













