#include "stdafx.h"

#define PARTICLE C_Particle3D_CP

#ifndef SPU_SPAWN

#include "Dynamic/Particle/Update.h"

#ifdef PLATFORM_SPU
namespace Maths
{ 
	const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
}
#endif

namespace DynamicEffect
{
namespace Particle
{
	// update function
	void Shape2DCollisionUpdate(ParticleBlock** ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, U32 numBlocks)
	{
		PERF_EVENT("Particle.UpdateShape2DCollison");

		ParticleBlock** ppBlock;
		const PhysicsSettings* pPhysicsSettings;
		const EffectUpdateConstants* pUpdateConstants;

		FetchInputs(ppBlockEA, pPhysicsSettingsEA, pUpdateConstantsEA, ppBlock, pPhysicsSettings, pUpdateConstants, numBlocks);

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

		for(U32 i = 0; i < numBlocks; ++i)
		{
			Spu::MemBaseMarker marker;
			ParticleBlock* pBlock = ppBlock[i];
			ParticleBlock* pBlockEA = ppBlockEA[i];
			U8* pDataEA = pBlock->m_Element.Fetch();

			assert(pBlock->m_FrameIndex < pUpdateConstants->frameIndex);
			pBlock->m_FrameIndex = pUpdateConstants->frameIndex;

			assert(pBlock->m_pEffect != NULL);

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
			
		#ifdef PLATFORM_SPU
			pBlock->m_Element.Store(pDataEA);
			Spu::StoreAsync(pBlockEA, pBlock, sizeof(ParticleBlock), 0);
		#endif
		}
	}

	// update task
	void Shape2DCollisionUpdateTask(Scheduler::Task* pTask)
	{
		EffectUpdateData* pUpdateData = (EffectUpdateData*)pTask->ParamData;
		Shape2DCollisionUpdate((ParticleBlock**)pTask->Input, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}
}
}

// update module
JQ_DECLARE_SPU_MODULE(ParticleShape2DCollisionUpdateModule, DynamicEffect::Particle::Shape2DCollisionUpdateTask, dynamic_particle_update_shape2dcollision, "dynamic.scheduler");

#endif

// if spu and spawn module
#if (defined(PLATFORM_SPU) && defined(SPU_SPAWN)) || !defined(PLATFORM_SPU)

#include "Dynamic/Particle/Spawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape2DCollisionCreate : public DefaultCreationPolicy<Particle, Shape2DCollisionCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape2DCollisionCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
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

// spawn module
JQ_DECLARE_SPU_MODULE(ParticleShape2DCollisionSpawnModule, DynamicEffect::Particle::SpawnTask<DynamicEffect::Particle::Shape2DCollisionCreate<DynamicEffect::Particle::PARTICLE> >, dynamic_particle_spawn_shape2dcollision, "dynamic.scheduler");

#endif

#ifndef PLATFORM_SPU

#include "Dynamic/Particle/Generic/Shape2DCollision.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"


namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
Shape2DCollision::Shape2DCollision(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleEffect<PARTICLE, ShapePolicy::Shape2D, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape2DCollision)
{
	m_pUpdateModule = &ParticleShape2DCollisionUpdateModule;
	m_pUpdateFunctor = Shape2DCollisionUpdate;
	m_pSpawnModule = &ParticleShape2DCollisionSpawnModule;
	m_pSpawnFunctor = Spawn<Shape2DCollisionCreate<PARTICLE> >;
}

Shape2DCollision::~Shape2DCollision()
{
}


} // Particle
} // DynamicEffect

#endif

#undef PARTICLE




















