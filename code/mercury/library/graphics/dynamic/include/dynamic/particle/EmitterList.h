#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EMITTERLIST_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EMITTERLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Atomic.h"
#include "Dynamic/Particle/Emitter.h"

namespace DynamicEffect
{
namespace Particle
{

class ALIGN16 C_EmitterList : public AtomicActiveCache<C_Emitter>
{
public:
	C_EmitterList() {}
	~C_EmitterList() {}

	C_Emitter*	Allocate();
	void		Free(C_Emitter* pEmitter);
	void		Free();
};

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EMITTERLIST_H
