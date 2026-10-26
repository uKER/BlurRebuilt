#include "stdafx.h"

// header
#include "Dynamic/Particle/Event.h"
#include "Dynamic/Particle/EventInstanceHandle.h"

// particle system dependant
#include "Dynamic/Particle/Manager.h"

// effect base
#include "Dynamic/Particle/EffectInterface.h"

namespace DynamicEffect
{
namespace Particle
{

namespace DynamicEffect
{
	extern Debug::Logger& g_Log;
}

C_Event::C_Event(C_ParticleManager* pManager, const EventDefinition& definition) : m_pManager(pManager), m_Definition(definition)
{
	m_pManager->AddEvent(this);
}

C_Event::~C_Event()
{
	m_pManager->RemoveEvent(this);
}

void C_Event::AddDefinition(const EmitterDefinition& settings)
{
	m_EmitterDefinitions.PushBack(settings);
}

EmitterDefinition* C_Event::GetEmitterDefinition(S32 i)
{
	if(i >= 0 && i < m_EmitterDefinitions.Size())
		return &m_EmitterDefinitions[i];
	else
		return NULL;	
}

C_EventInstanceHandle C_Event::Create(const C_Node& local, const C_Node* pParent, U32 typeFilter)
{
#ifndef CFG_SPEED
	if ((_finite(local.GetPosition().x) == 0) ||
		(_finite(local.GetPosition().y) == 0) ||
		(_finite(local.GetPosition().z) == 0))
	{
		LOGWARN(g_Log, "ParticleManager: Created event has invalid position\n");
		return C_EventInstanceHandle();
	}
	if ((_finite(local.GetRotation().x) == 0) ||
		(_finite(local.GetRotation().w) == 0) ||
		(_finite(local.GetRotation().z) == 0) ||
		(_finite(local.GetRotation().w) == 0))
	{
		LOGWARN(g_Log, "ParticleManager: Created event has invalid rotation\n");
		return C_EventInstanceHandle();
	}
#endif

#ifndef CFG_GOLD
	// TODO: remove, trying to catch 29790
	if(m_pManager->m_bUpdatingEmitters)
		DebugBreak();
	// TODO: remove, trying to catch 29790
#endif

	if(C_EventInstance* pEventInstance = m_pManager->AtomicNewEventInstance())
	{
		// events default to active
		pEventInstance->m_fIntensity = 1.0f;
		pEventInstance->m_fAlpha = 1.0f;
		pEventInstance->m_bActive = true;
		pEventInstance->m_bRepeating = m_Definition.bRepeating;
		pEventInstance->m_pEvent = this;
		pEventInstance->Reset(m_pManager->GetRandom(0));
		C_EventInstanceHandle handle(pEventInstance);
		pEventInstance->m_Emitters.Resize(m_EmitterDefinitions.Size());

		for(S32 i = 0; i < m_EmitterDefinitions.Size(); ++i)
		{
			C_Emitter* pEmitter = NULL;

			// check the particle type hasn't been filtered
			//if(!(typeFilter & (1<<m_EmitterDefinitions[i].pEffect->GetRenderSettings().eParticleType)))
			{
				if((pEmitter = m_pManager->GetEmitterList()->AtomicAllocate()))
				{
					pEmitter->SetDefinition(&m_EmitterDefinitions[i], pEventInstance);
					
					pEmitter->SetTransform(local, pParent);
					pEmitter->SetEventIndex(i);
					
					// if the effect is in world space then take into account external transform
					if(m_EmitterDefinitions[i].eSpace == E_EmitterSpace_World)
						pEmitter->SetPositionPrevious(pEmitter->GetWorldPosition());
					else
						pEmitter->SetPositionPrevious(local.GetPosition());
				}

				
			}
			pEventInstance->m_Emitters[i] = pEmitter;
		}
		
		return handle;
	}
	else
		return C_EventInstanceHandle();
}

} // Particle
} // DynamicEffect
