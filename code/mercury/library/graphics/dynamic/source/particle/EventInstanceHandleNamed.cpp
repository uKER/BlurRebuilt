#include "stdafx.h"

// header
#include "Dynamic/Particle/EventInstanceHandleNamed.h"
#include "Dynamic/Particle/Manager.h"

using namespace DynamicEffect::Particle;

void C_EventInstanceHandleNamed::SetName(const Util::C_String& name)
{	
	m_Name = name;
	if(C_Event* pEvent = m_pManager->FindEvent(name))
	{
		C_EventInstanceHandle handle  = pEvent->Create(DynamicEffect::C_Node());
		m_pObject = handle.m_pObject;
		m_iSalt = handle.m_iSalt;
	}
}

	// master builds don't do checks
#ifndef CFG_GOLD
C_EventInstance* C_EventInstanceHandleNamed::Object()
{
	if (!m_pObject || (m_pObject->GetSalt() != m_iSalt))
	{
		if(C_Event* pEvent = m_pManager->FindEvent(m_Name))
		{
			C_EventInstanceHandle handle  = pEvent->Create(DynamicEffect::C_Node());
			if(handle.Object())
			{
				m_pObject = handle.m_pObject;
				m_iSalt = handle.m_iSalt;
			
				return m_pObject;
			}
			else // handle not valid, return zero
				return 0;
		}
		else // couldn't find an event of this name, return zero
			return 0;
	}

	return m_pObject;
}
#endif










