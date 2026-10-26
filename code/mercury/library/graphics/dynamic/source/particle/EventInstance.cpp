#include "stdafx.h"

// header
#include "Dynamic/Particle/EventInstance.h"

// particle manager dependant
#include "Dynamic/Particle/Manager.h"

using namespace DynamicEffect;
using namespace DynamicEffect::Particle;

namespace DynamicEffect
{
	extern Debug::Logger& g_Log;
}

C_EventInstance::C_EventInstance(C_ParticleManager* pRenderer) : m_pRenderer(pRenderer), m_iSalt(0)
{
	m_pRenderer->AddEventInstance(this);
}

C_EventInstance::~C_EventInstance()
{
	m_pRenderer->RemoveEventInstance(this);
}

void C_EventInstance::Spawn(const F32 fFrameTime)
{
	/*for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		//if(m_Emitters[i])
			m_Emitters[i]->Spawn(fFrameTime);
	}*/
}

void C_EventInstance::SetIntensity(F32 fIntensity)
{
	if(fIntensity != m_fIntensity)
	{
		assert(fIntensity >= 0.0f);
		assert(fIntensity <= 10.0f);
		
		// setting intensity to zero causes emitters to be deactivated
		if(fIntensity != 0.0f)
		{
			// if the intensity was previously zero, reactivate emitter
			if(m_fIntensity == 0.0f && m_fAlpha != 0.0f)
				SetActive(true);
		}
		else
			SetActive(false);

		for(S32 i = 0; i < m_Emitters.Size(); ++i)
		{
			if(m_Emitters[i])
				m_Emitters[i]->SetIntensity(fIntensity);
		}

		m_fIntensity = fIntensity;
	}
}

void C_EventInstance::SetActive(bool bActive)
{
	if(bActive != m_bActive)
	{
		for(S32 i = 0; i < m_Emitters.Size(); ++i)
		{
			if(m_Emitters[i])
			{
				// the emitter might have been moved whilst inactive, update the previous position so we don't get crazy emissions/distance
				if(!m_bActive)
				{
					// if the effect is in world space then take into account external transform
					if(m_Emitters[i]->GetDefinition()->eSpace == E_EmitterSpace_World)
						m_Emitters[i]->SetPositionPrevious(m_Emitters[i]->GetWorldPosition());
					else
						m_Emitters[i]->SetPositionPrevious(m_Emitters[i]->GetLocalPosition());
				}					
				m_Emitters[i]->SetActive(bActive);
			}
		}

		m_bActive = bActive;
	}
}


void C_EventInstance::SetTransform(const C_Node& local, const C_Node* pParent)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetTransform(local, pParent);
	}
}

void C_EventInstance::SetLighting(const Maths::C_RGBA_F32& vLighting)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetLighting(vLighting);
	}
}

void C_EventInstance::SetLighting(const Maths::C_RGBA_F32& vLighting, const Maths::C_RGBA_F32& vTintColour)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetLighting(vLighting, vTintColour);
	}
}

void C_EventInstance::SetAlpha(F32 fAlpha)
{
	assert(fAlpha >= 0.0f && fAlpha <= 1.0f);

	if(fAlpha != m_fAlpha)
	{
		// setting alpha to zero causes emitters to be deactivated
		if(fAlpha != 0.0f)
		{
			// if the intensity was previously zero, reactivate emitter
			if(m_fAlpha == 0.0f && m_fIntensity != 0.0f)
				SetActive(true);
		}
		else
			SetActive(false);

		for(S32 i = 0; i < m_Emitters.Size(); ++i)
		{
			if(m_Emitters[i])
				m_Emitters[i]->SetAlpha(fAlpha);
		}

		m_fAlpha = fAlpha;
	}
}

void C_EventInstance::SetLifeScale(F32 fLifeScale)
{
	assert(fLifeScale >= 0.0f && fLifeScale <= 1.0f);

	if(fLifeScale != m_fLifeScale)
	{
		for(S32 i = 0; i < m_Emitters.Size(); ++i)
		{
			if(m_Emitters[i])
				m_Emitters[i]->SetLifeScale(fLifeScale);
		}

		m_fLifeScale = fLifeScale;
	}
}

void C_EventInstance::SetVelocity(const Maths::HVec3& vVelocity)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetVelocity(vVelocity);
	}
}

void C_EventInstance::SetVisibilityFlags( U32 visibilityFlags )
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetVisibilityFlags(visibilityFlags);
	}
}

void C_EventInstance::SetSpawnMesh(const C_SpawnMesh* pSpawnMesh)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetSpawnMesh(pSpawnMesh);
	}
}

void C_EventInstance::SetSpline(const Curve::C_ActorCurve3D* pSpline)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetSpline(pSpline);
	}
}

void C_EventInstance::SetSplineUsage(const F32 fBegin, const F32 fEnd)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetSplineUsage(fBegin, fEnd);
	}
}

void C_EventInstance::SetSpawner(I_Spawner* pSpawner)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetSpawner(pSpawner);
	}
}

void C_EventInstance::SetCollisionPlane(const MathsStorage::Float4A16& vCollisionPlanePosition, const MathsStorage::Float4A16& vCollisionPlaneNormal)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetCollisionPlane(vCollisionPlanePosition, vCollisionPlaneNormal);
	}
}

void C_EventInstance::SetCollisionPlane(const Maths::HVec3& vCollisionPlanePosition, const Maths::HVec3& vCollisionPlaneNormal)
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->SetCollisionPlane(vCollisionPlanePosition, vCollisionPlaneNormal);
	}
}

bool C_EventInstance::Spawning(S32 iIndex) const
{
#ifndef CFG_SPEED
	if(iIndex < 0 || iIndex >= m_Emitters.Size())
	{
		LOGERROR(g_Log, "DynamicEffect: Spawning() invalid index");
		return false;
	}
#endif
	if(m_Emitters[iIndex] && m_Emitters[iIndex]->Spawning())
		return true;
	else
		return false;
}

S32 C_EventInstance::Update(F32 fFrameTime, C_Random& random)
{
	// see if all the pointers are null, free if so
	bool bDead = true;
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
		bDead = bDead && !m_Emitters[i];

	if(bDead)
	{
		m_pRenderer->FreeEventInstance(this);
		return 0;
	}
	else if(m_fCycle > m_fDuration)
	{
		// either reset the cycle or free the event instance
		if(m_pEvent->m_Definition.bRepeating)
			Reset(random);
		else
		{
			FreeRemove();
			return 0;
		}
	}

	m_fCycle += fFrameTime;

	return 1;
}

void C_EventInstance::Reset()
{
	m_fCycle = 0.0f;
	m_fDuration = m_pEvent->m_Definition.fDuration;

	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->Reset();
	}
}


void C_EventInstance::Reset(C_Random& random)
{
	m_fCycle = 0.0f;
	m_fDuration = m_pEvent->m_Definition.fDuration + random.FRand(-m_pEvent->m_Definition.fDurationVariation, m_pEvent->m_Definition.fDurationVariation);

	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->Reset(random);
	}
}

void C_EventInstance::FreeRemove()
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			m_Emitters[i]->Free();
	}

	m_pRenderer->FreeEventInstance(this);
}

void C_EventInstance::Free()
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(C_Emitter* pEmitter = m_Emitters[i])
			pEmitter->AtomicFree(pEmitter);
	}

	m_pRenderer->AtomicFreeEventInstance(this);
}

Maths::HVec3 C_EventInstance::GetWorldPosition() const
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			return m_Emitters[i]->GetWorldPosition();
	}

	return Maths::HVec3();
}

Maths::HVec3 C_EventInstance::GetWorldNormal() const
{
	for(S32 i = 0; i < m_Emitters.Size(); ++i)
	{
		if(m_Emitters[i])
			return m_Emitters[i]->GetWorldNormal();
	}

	return Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
}











