#include "stdafx.h"

#define PARTICLE	C_Particle3D_RV3D_CP

#ifndef SPU_SPAWN

#include "Dynamic/Particle/Update.h"

#ifdef PLATFORM_SPU
namespace Maths
{
	const vector float Const< vector float >::m_UnitW = (vector float)( 0.0f, 0.0f, 0.0f, 1.0f );
	const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
}
#endif
	
namespace DynamicEffect
{
namespace Particle
{
	void Shape3DCollisionUpdate(ParticleBlock** ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, U32 numBlocks)
	{
		PERF_EVENT("Particle.UpdateShape3DCollison");
		
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
	void Shape3DCollisionUpdateTask(Scheduler::Task* pTask)
	{
		EffectUpdateData* pUpdateData = (EffectUpdateData*)pTask->ParamData;
		Shape3DCollisionUpdate((ParticleBlock**)pTask->Input, pUpdateData->pPhysicsSettings, pUpdateData->pUpdateConstants, pUpdateData->numBlocks);
	}

} // Particle
} // DynamicEffect

// update module
JQ_DECLARE_SPU_MODULE(ParticleShape3DCollisionUpdateModule, DynamicEffect::Particle::Shape3DCollisionUpdateTask, dynamic_particle_update_shape3dcollision, "dynamic.scheduler");

#endif

// if spu and spawn module
#if (defined(PLATFORM_SPU) && defined(SPU_SPAWN)) || !defined(PLATFORM_SPU)

#include "Dynamic/Particle/Spawn.h"

namespace DynamicEffect
{
namespace Particle
{

template<class Particle>
class Shape3DCollisionCreate : public DefaultCreationPolicy<Particle, Shape3DCollisionCreate>
{
public:
	static void CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
};

//lint -e*
template<class Particle>
void Shape3DCollisionCreate<Particle>::CreateParticle(Particle* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
{
	ParticleUpdate<Particle> create;

	create.SetParticle(pParticle);
	create.CreatePositionLife(pEmitter->GetSpawnPosition(random));
	create.CreateVelocity(pEmitter->GetSpawnVelocity(random));
	create.CreateRotation(pEmitter->GetSpawnOrientation(random), pEmitter->GetSpawnAngularVelocity(random));
	create.CreateColour(pEmitter->GetSpawnLighting(), pEmitter->GetSpawnColour(random));
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

// spawn module
JQ_DECLARE_SPU_MODULE(ParticleShape3DCollisionSpawnModule, DynamicEffect::Particle::SpawnTask<DynamicEffect::Particle::Shape3DCollisionCreate<DynamicEffect::Particle::PARTICLE> >, dynamic_particle_spawn_shape3dcollision, "dynamic.scheduler");

#endif

#ifndef PLATFORM_SPU

#include "Dynamic/Particle/Generic/Shape3DCollision.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Particle
{

// initialise system with correct values, then define custom creation and updating functions
Shape3DCollision::Shape3DCollision(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex)
	: T_ParticleEffect<PARTICLE, ShapePolicy::Shape3D, BuildPolicy::T_All>(pRenderer, pManager, iIndex, E_EffectTypeIndex_Shape3DCollision)
{
	m_pUpdateModule = &ParticleShape3DCollisionUpdateModule;
	m_pUpdateFunctor = Shape3DCollisionUpdate;
	m_pSpawnModule = &ParticleShape3DCollisionSpawnModule;
	m_pSpawnFunctor = Spawn<Shape3DCollisionCreate<PARTICLE> >;	
}

Shape3DCollision::~Shape3DCollision()
{
}

} // Particle
} // DynamicEffect

#endif // PLATFORM_SPU

#undef PARTICLE



















