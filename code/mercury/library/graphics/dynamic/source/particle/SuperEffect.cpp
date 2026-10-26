#include "stdafx.h"

#include "Dynamic/Particle/SuperEffect.h"
#include "Dynamic/Particle/SuperSpawn.h"

namespace DynamicEffect
{
namespace Particle
{

I_ParticleSuperEffect::I_ParticleSuperEffect(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex) :
	I_ParticleEffectBase(pRenderer, pManager, iIndex, iTypeIndex)
{
	m_SpawnSettings.pSuperEffects = &m_SuperEffects;
	m_SuperEffects.pTrailBlockManager = pManager->GetParticleTrailBlockManager();
	m_SuperEffects.pEmitterList = pManager->GetSuperEffectEmitterList();

	m_pManager->AddSuperEffect(this);
}

I_ParticleSuperEffect::~I_ParticleSuperEffect()
{
	m_pManager->RemoveSuperEffect(this);
}

void I_ParticleSuperEffect::AddChild(E_EffectChild type, const EmitterDefinition& definition, const MathsStorage::Float4A16& settings)
{
	switch(type)
	{
	case E_EffectChild_Trail:
		assert(definition.pEffect != this);
		m_SuperEffects.trailEffects.PushBack();
		m_SuperEffects.trailEffects.Back().definition = definition;
		// trail effects are inheritantly repeating because the particles manage the emitter
		m_SuperEffects.trailEffects.Back().definition.bRepeating = true;
		m_SuperEffects.trailEffects.Back().definition.fDuration = 0x7F800000;
		m_SuperEffects.trailEffects.Back().blocks.Resize(16);
		m_SuperEffects.trailEffects.Back().blocks.Clear();
		break;
	case E_EffectChild_Death:
		assert(definition.pEffect != this);
		m_SuperEffects.deathEffects.PushBack();
		m_SuperEffects.deathEffects.Back().definition = definition;
		break;
	case E_EffectChild_Collision:
		assert(definition.pEffect != this);
		m_SuperEffects.collisionEffects.PushBack();
		m_SuperEffects.collisionEffects.Back().definition = definition;
		m_SuperEffects.collisionEffects.Back().settings = settings;
		break;
	case E_EffectChild_CollisionOnce:
		assert(definition.pEffect != this);
		m_SuperEffects.collisionOnceEffects.PushBack();
		m_SuperEffects.collisionOnceEffects.Back().definition = definition;
		break;
	default:
		assert(0);
	}
}

void I_ParticleSuperEffect::Free()
{
	// only delete the trail blocks since base class will destroy normal blocks
	for(S32 i = 0; i < m_SuperEffects.trailEffects.Size(); ++i)
	{
		for(S32 j = 0; j < m_SuperEffects.trailEffects[i].blocks.Size(); ++j)
		{
			if(TrailBlock* pTrailBlock = m_SuperEffects.trailEffects[i].blocks[j])
			{
				for(int a = 0; a < pTrailBlock->m_Element.Size(); ++a)
				{
					C_Particle_E* pParticle = (C_Particle_E*)&pTrailBlock->m_Element[a];
					if(pParticle->pEmitter)
					{
						if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
							pParticle->pEmitter = NULL;
						else
							Base::m_pManager->GetSuperEffectEmitterList()->Free(pParticle->pEmitter);
					}
				}

				pTrailBlock->m_Element.Clear();
				m_SuperEffects.pTrailBlockManager->FreeBlock(pTrailBlock);
			}
		}
		m_SuperEffects.trailEffects[i].blocks.Clear();
	}

	for(S32 i = 0; i < m_SuperEffects.deathEffects.Size(); ++i)
	{
		Base::m_pManager->CleanEmitterDefinition(&m_SuperEffects.deathEffects[i].definition);
	}

	for(S32 i = 0; i < m_SuperEffects.collisionEffects.Size(); ++i)
	{
		Base::m_pManager->CleanEmitterDefinition(&m_SuperEffects.collisionEffects[i].definition);
	}

	for(S32 i = 0; i < m_SuperEffects.collisionOnceEffects.Size(); ++i)
	{
		Base::m_pManager->CleanEmitterDefinition(&m_SuperEffects.collisionOnceEffects[i].definition);
	}
}

void I_ParticleSuperEffect::UpdateMemory()
{
	// check all blocks are equal size
	S32 currentBlocksSize = m_SpawnSettings.blocks.Size();
	for(S32 i = 0; i < m_SuperEffects.trailEffects.Size(); ++i)
	{
		assert(currentBlocksSize == m_SuperEffects.trailEffects[i].blocks.Size());
	}

	// check whether blocks is about to be resized
#ifndef CFG_SPEED
	assert(m_Allocated <= m_SpawnSettings.blocks.Capacity());
	assert(m_Allocated <= m_nBlocks);
#endif

	S32 requestedBlocks = m_nBlocks;
	m_nBlocks = m_nBlocks > m_SpawnSettings.blocks.Capacity() ? m_SpawnSettings.blocks.Capacity() : m_nBlocks;
	
	// tell blocks how big it should potentially be
	m_SpawnSettings.blocks.Size(requestedBlocks);
	m_SpawnSettings.blocks.Update();

	for(S32 i = 0; i < m_SuperEffects.trailEffects.Size(); ++i)
	{
		m_SuperEffects.trailEffects[i].blocks.Size(requestedBlocks);
		m_SuperEffects.trailEffects[i].blocks.Update();
	}

	while(m_nBlocks > 0 && m_SpawnSettings.blocks[m_nBlocks - 1] == NULL)
		--m_nBlocks;

	for(S32 b = 0; b < m_nBlocks; ++b)
	{
		if(m_SpawnSettings.blocks[b] == NULL)
		{
			S32 end = --m_nBlocks;
			ParticleBlock* pBlock = m_SpawnSettings.blocks[end];
			assert(pBlock);
			pBlock->m_VectorPosition = b;
			m_SpawnSettings.blocks[b] = pBlock;

			for(S32 t = 0; t < m_SuperEffects.trailEffects.Size(); ++t)
			{
				if(ParticleTrailBlock* pTrailBlock = m_SuperEffects.trailEffects[t].blocks[end])
				{
					pTrailBlock->m_VectorPosition = b;
					m_SuperEffects.trailEffects[t].blocks[b] = pTrailBlock;
				}
			}

			while(m_nBlocks > 0 && m_SpawnSettings.blocks[m_nBlocks - 1] == NULL)
				--m_nBlocks;
		}
	}

	m_SpawnSettings.blocks.Size(m_nBlocks);
	for(S32 i = 0; i < m_SuperEffects.trailEffects.Size(); ++i)
		m_SuperEffects.trailEffects[i].blocks.Size(m_nBlocks);

	if(m_nBlocks == 0 && m_ActivePosition != -1)
		m_pManager->RemoveActiveSuperEffect(this);

#ifndef CFG_SPEED
	assert(m_nBlocks == m_Allocated);
#endif
}

void I_ParticleSuperEffect::DeleteBlock(ParticleBlock* pDelete)
{
	I_ParticleEffectBase::DeleteBlock(pDelete);

	if(m_nBlocks == 0 && m_ActivePosition != -1)
		m_pManager->RemoveActiveSuperEffect(this);
}

void I_ParticleSuperEffect::DeleteTrailBlock(TrailBlock* pTrailDelete)
{
	TrailEffect& trailEffect = m_SuperEffects.trailEffects[pTrailDelete->m_EffectIndex];

	// free all the emitters in the trail blocks
	for(S32 j = 0; j < pTrailDelete->m_Element.Size(); ++j)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&pTrailDelete->m_Element[j];
		if(pParticle->pEmitter)
		{
			if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
				pParticle->pEmitter = NULL;
			else
				Base::m_pManager->GetSuperEffectEmitterList()->Free(pParticle->pEmitter);
		}
	}

	// erase from vector
	S32 end = trailEffect.blocks.Size() - 1;
	ParticleTrailBlock* pEnd = trailEffect.blocks[end];
	assert(pEnd);
	trailEffect.blocks[pTrailDelete->m_VectorPosition] = pEnd;
	trailEffect.blocks.Size(end);
	//trailEffect.blocks.PopBack();
	pEnd->m_VectorPosition = pTrailDelete->m_VectorPosition;

	m_SuperEffects.pTrailBlockManager->FreeBlock(pTrailDelete);
}

void I_ParticleSuperEffect::SpawnCustom(C_Emitter* pEmitter, C_Random& random, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, const EmitterUpdateConstants* pUpdateConstants)
{
	SpawnSuperT<CustomSpawnerCreationPolicy>(pEmitter, pSpawnSettings, pSuperEffects, random, pUpdateConstants);
}

void FreeParticle(U32 i, ParticleBlock* pBlock, TrailPtrs& trailPtrs, C_EmitterList* pEmitterList)
{
	// get the last particle of the last block and copy it here
	void* pSrc = (void*)&pBlock->m_Element[pBlock->m_Element.Size()-1];
	void* pDest = (void*)&pBlock->m_Element[i];
	memcpy(pDest, pSrc, pBlock->m_Element.ElementSize());
	
	for(S32 t = 0; t < trailPtrs.Size(); ++t)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&trailPtrs[t]->m_Element[i];
		if(pParticle->pEmitter)
		{
			if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
				pParticle->pEmitter = NULL;
			else
				pEmitterList->AtomicFree(pParticle->pEmitter);
		}
		
		// copy trail effect
		{
			C_Particle_E* pSrc = (C_Particle_E*)&trailPtrs[t]->m_Element[trailPtrs[t]->m_Element.Size()-1];
			C_Particle_E* pDest = (C_Particle_E*)&trailPtrs[t]->m_Element[i];
			*pDest = *pSrc;
		}
		
		trailPtrs[t]->m_Element.PopBack();
	}

	pBlock->m_Element.PopBack();
}

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
// update trail emitter positions
void UpdateTrails(const SuperEffects* pSuperEffects, S32 iIndex, TrailPtrs& trailPtrs, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < pSuperEffects->trailEffects.Size(); ++i)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&trailPtrs[i]->m_Element[iIndex];
		if(pParticle->pEmitter)
		{
			if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
				pParticle->pEmitter = NULL;
			else
			{
				pParticle->pEmitter->SetLocalTransform(vPosition, qOrientation);
				pParticle->pEmitter->SetParentTransform(pParentTransform);
				pParticle->pEmitter->SetPosition(vPosition);
				pParticle->pEmitter->SetVelocity(vVelocity);
				pParticle->pEmitter->SetCalculatePositions(false);
				pParticle->pEmitter->SetVisibilityFlags(visibilityFlags);
			}
		}
	}
};

// update trail emitter positions
void UpdateTrails(const SuperEffects* pSuperEffects, S32 iIndex, TrailPtrs& trailPtrs, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < pSuperEffects->trailEffects.Size(); ++i)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&trailPtrs[i]->m_Element[iIndex];
		if(pParticle->pEmitter)
		{
			if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
				pParticle->pEmitter = NULL;
			else
			{
				pParticle->pEmitter->SetLocalTransform(vPosition, qOrientation);
				pParticle->pEmitter->SetParentTransform(pParentTransform);
				pParticle->pEmitter->SetPosition(vPosition);
				pParticle->pEmitter->SetVelocity(vVelocity);
				pParticle->pEmitter->SetCalculatePositions(false);
				pParticle->pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
				pParticle->pEmitter->SetVisibilityFlags(visibilityFlags);
			}
		}
	}
};

// deaths
void UpdateDeaths(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < pSuperEffects->deathEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			pEmitter->SetDefinition(&pSuperEffects->deathEffects[i].definition);
			pEmitter->SetVisible(true);
			pEmitter->SetLocalTransform(vPosition, qOrientation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetCalculateVelocity(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

void UpdateDeaths(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < pSuperEffects->deathEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			pEmitter->SetDefinition(&pSuperEffects->deathEffects[i].definition);
			pEmitter->SetVisible(true);

			pEmitter->SetLocalTransform(vPosition, qOrientation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

// collisions
void UpdateCollisions(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, XMVECTOR vTimeStep, const C_Node* pParentTransform, U32 visibilityFlags)
{
	MathsStorage::Float4A16 orientation;
	XMVECTOR qRotation;
	if(pSuperEffects->collisionEffects.Size() > 0)
	{
		XMVECTOR vNormalisedPlaneNormal = XMVector3NormalizeEst(vPlaneNormal);
		qRotation = GetQuaternionTo<XMVECTOR>(vNormalisedPlaneNormal);
		XMStoreFloat4A16((XMFLOAT4A16*)&orientation, qRotation);
	}

	for(S32 i = 0; i < pSuperEffects->collisionEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			MathsStorage::Float4A16 intensity;
			XMVECTOR vSettings = XMLoadFloat4A16((XMFLOAT4A16*)&pSuperEffects->collisionEffects[i].settings);
			XMVECTOR vSpeed = XMVector3LengthEst(vVelocity);
			XMVECTOR vIntensity = XMVectorSubtract(vSpeed, XMVectorSplatX(vSettings));
			vIntensity = XMVectorMultiply(vIntensity, XMVectorReciprocalEst(XMVectorSplatY(vSettings)));
			vIntensity = XMVectorMultiply(vIntensity, vTimeStep);
			vIntensity = XMVectorMax(vIntensity, XMVectorZero());
			
			XMStoreFloat4A16((XMFLOAT4A16*)&intensity, vIntensity);

			pEmitter->SetDefinition(&pSuperEffects->collisionEffects[i].definition);
			pEmitter->SetVisible(true);
			pEmitter->SetIntensity(intensity.x);

			pEmitter->SetLocalTransform(vPlanePosition, qRotation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetCalculateVelocity(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

// collisions
void UpdateCollisionsOnce(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags)
{
	XMVECTOR qRotation;
	if(pSuperEffects->collisionOnceEffects.Size() > 0)
	{
		XMVECTOR vNormalisedPlaneNormal = XMVector3NormalizeEst(vPlaneNormal);
		qRotation = GetQuaternionTo<XMVECTOR>(vNormalisedPlaneNormal);
	}

	for(S32 i = 0; i < pSuperEffects->collisionOnceEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			pEmitter->SetDefinition(&pSuperEffects->collisionOnceEffects[i].definition);
			pEmitter->SetVisible(true);

			pEmitter->SetLocalTransform(vPosition, qRotation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetCalculateVelocity(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

#else

// update trail emitter positions
void UpdateTrails(const SuperEffects* pSuperEffects, S32 iIndex, TrailPtrs& trailPtrs, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < trailPtrs.Size(); ++i)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&trailPtrs[i]->m_Element[iIndex];
		if(pParticle->pEmitter)
		{
			if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
				pParticle->pEmitter = NULL;
			else
			{
				pParticle->pEmitter->SetLocalTransform(vPosition, qOrientation);
				pParticle->pEmitter->SetParentTransform(pParentTransform);
				pParticle->pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
				pParticle->pEmitter->SetVisibilityFlags(visibilityFlags);
			}
		}
	}
};

// update trail emitter positions
void UpdateTrails(const SuperEffects* pSuperEffects, S32 iIndex, TrailPtrs& trailPtrs, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < trailPtrs.Size(); ++i)
	{
		C_Particle_E* pParticle = (C_Particle_E*)&trailPtrs[i]->m_Element[iIndex];
		if(pParticle->pEmitter)
		{
			if(pParticle->iSalt != pParticle->pEmitter->GetSalt())
				pParticle->pEmitter = NULL;
			else
			{
				pParticle->pEmitter->SetLocalTransform(vPosition, qOrientation);
				pParticle->pEmitter->SetParentTransform(pParentTransform);
				pParticle->pEmitter->SetVisibilityFlags(visibilityFlags);
			}
		}
	}
};

void UpdateDeaths(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < pSuperEffects->deathEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			pEmitter->SetDefinition(&pSuperEffects->deathEffects[i].definition);
			pEmitter->SetVisible(true);
			
			pEmitter->SetLocalTransform(vPosition, qOrientation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

void UpdateDeaths(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags)
{
	for(S32 i = 0; i < pSuperEffects->deathEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			pEmitter->SetDefinition(&pSuperEffects->deathEffects[i].definition);
			pEmitter->SetVisible(true);

			pEmitter->SetLocalTransform(vPosition, qOrientation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetCalculateVelocity(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

void UpdateCollisions(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const Maths::HF32& vTimeStep, const C_Node* pParentTransform, U32 visibilityFlags)
{
	MathsStorage::Float4A16 orientation;
	if(pSuperEffects->collisionEffects.Size() > 0)
	{
		Maths::HVec3 vNormalisedPlaneNormal = Maths::Normalise(vPlaneNormal);
		orientation = GetQuaternionTo<MathsStorage::Float4A16>(vNormalisedPlaneNormal);
	}
	Maths::C_Quaternion qFloorOrientation(Maths::Load<Maths::HVec4>(&orientation));

	for(S32 i = 0; i < pSuperEffects->collisionEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			Maths::HVec4 vSettings = Maths::Load<Maths::HVec4>(&pSuperEffects->collisionEffects[i].settings);
			Maths::HF32 fSpeed = Maths::Length(vVelocity);
			Maths::HF32 fIntensity = fSpeed - vSettings.X();
			fIntensity *= vSettings.Y().Reciprocal();
			fIntensity *= vTimeStep;
			fIntensity = Maths::Max(fIntensity, Maths::Constants<Maths::HF32>::Zero());

			pEmitter->SetDefinition(&pSuperEffects->collisionEffects[i].definition);
			pEmitter->SetVisible(true);
			pEmitter->SetIntensity(fIntensity);
			
			pEmitter->SetLocalTransform(vPosition, qFloorOrientation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetCalculateVelocity(false);
			pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

void UpdateCollisionsOnce(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags)
{
	MathsStorage::Float4A16 orientation;
	if(pSuperEffects->collisionOnceEffects.Size() > 0)
	{
		Maths::HVec3 vNormalisedPlaneNormal = Maths::Normalise(vPlaneNormal);
		orientation = GetQuaternionTo<MathsStorage::Float4A16>(vNormalisedPlaneNormal);
	}
	Maths::C_Quaternion qFloorOrientation(Maths::Load<Maths::HVec4>(&orientation));

	for(S32 i = 0; i < pSuperEffects->collisionOnceEffects.Size(); ++i)
	{
		if(C_Emitter* pEmitter = pSuperEffects->pEmitterList->AtomicAllocate())
		{
			pEmitter->SetDefinition(&pSuperEffects->collisionOnceEffects[i].definition);
			pEmitter->SetVisible(true);
			
			pEmitter->C_EmitterBase<ParticleBlock>::SetLocalTransform(vPosition, qFloorOrientation);
			pEmitter->SetParentTransform(pParentTransform);
			pEmitter->SetPositionPrevious(vPlanePosition);
			pEmitter->SetVelocity(vVelocity);
			pEmitter->SetCollisionPlane(vPlanePosition, vPlaneNormal);
			pEmitter->SetCalculatePositions(false);
			pEmitter->SetCalculateVelocity(false);
			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	}
};

#endif

} // Particle
} // DynamicEffect
