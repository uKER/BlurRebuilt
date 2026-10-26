#include "stdAfx.h"
#include "gravity/sdk/FluidEvents.h"

namespace Gravity
{

	void C_FluidEventBuffer::AddFluidEvent(const C_FluidEvent& fluidEvent)
	{
		//Lock the memory address...
		U32 index = MT::AtomicAdd((U32*)(&m_pPpuPtr->m_NumFluidEvents), 1);
		if(index < m_MaxFluidEvents)
		{
#ifdef PLATFORM_SPU
			Spu::Store(&m_pFluidEvents[index], &fluidEvent, sizeof(C_FluidEvent));
#else
			m_pFluidEvents[index] = fluidEvent;
#endif
		}
	}

}