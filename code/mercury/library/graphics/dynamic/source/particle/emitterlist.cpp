#include "stdafx.h"

#include "Dynamic/Particle/EmitterList.h"
#include "Dynamic/Particle/Emitter.h"

namespace DynamicEffect
{
namespace Particle
{

C_Emitter*	C_EmitterList::Allocate()
{
	return AtomicActiveCache<C_Emitter>::Allocate();
}

void C_EmitterList::Free(C_Emitter* pEmitter)
{
#ifndef CFG_GOLD
	// TODO: remove, trying to catch 29790
	if(pEmitter->m_pEventInstance != NULL)
		DebugBreak();
	// TODO: remove, trying to catch 29790
#endif
	pEmitter->m_iSalt += 1;
	AtomicActiveCache<C_Emitter>::Free(pEmitter);
}

void C_EmitterList::Free()
{
	AtomicActiveCache<C_Emitter>::Free();
}

}
}










		


	


















