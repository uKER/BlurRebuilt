#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EVENT_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EVENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Emitter.h"

namespace DynamicEffect
{
namespace Particle
{

class C_ParticleManager;
class C_EventInstanceHandle;

struct EventDefinition
{
	Util::C_String	name;
	F32			fDuration;
	F32			fDurationVariation;
	F32			fCullDistance;
	F32			fLocalParticleLife;
	bool		bRepeating;
	bool		bCollisionPlane;		// whether any of the events require collision planes
};

class C_Event
{
public:
	C_Event(C_ParticleManager* pRenderer, const EventDefinition& definition);
	~C_Event();
	void						AddDefinition(const EmitterDefinition& settings);
	C_EventInstanceHandle		Create(const C_Node& local, const C_Node* pParent = NULL, U32 typeFilter = 0);
	const EventDefinition&		GetDefinition() const { return m_Definition; }

	// get pointer to definition so client can modify definitions
	EmitterDefinition*			GetEmitterDefinition(S32 i);
	friend class C_ParticleManager;
	friend class C_Emitter;
	friend class C_EventInstance;

private:
	EmitterDefinitions		m_EmitterDefinitions;
	EventDefinition			m_Definition;
	C_ParticleManager*		m_pManager;
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EVENT_H
