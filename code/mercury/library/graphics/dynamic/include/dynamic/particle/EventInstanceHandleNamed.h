#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCEHANDLENAMED_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCEHANDLENAMED_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/EventInstanceHandle.h"

namespace DynamicEffect
{
namespace Particle
{

class C_ParticleManager;

// extension to instance handle will automatically try and reaquire handle based on the name if it becomes invalid

class C_EventInstanceHandleNamed : public C_EventInstanceHandle
{
public:
	C_EventInstanceHandleNamed(C_ParticleManager* pManager, const Util::C_String& name) : m_pManager(pManager), m_Name(name) {}
	C_EventInstanceHandleNamed() {}
	~C_EventInstanceHandleNamed() {}

	void SetName(const Util::C_String& name);

	// master builds don't do checks
#ifndef CFG_GOLD
	C_EventInstance* Object();	
#endif

private:
	Util::C_String		m_Name;
	C_ParticleManager*	m_pManager;
};

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCEHANDLENAMED_H
