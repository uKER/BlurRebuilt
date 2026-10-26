#ifndef GRAVITY_FLUID_EVENTS_H
#define GRAVITY_FLUID_EVENTS_H

namespace SharedPhysics
{
	class C_GraphNode;
}

namespace Gravity
{

	class C_Body;

	struct ALIGN16 C_FluidEvent
	{
		MathsStorage::Float3 m_SubmergedCentroid;
		MathsStorage::Float3 m_SurfacePoint;
		F32 m_SubmergedVolume;
		SharedPhysics::C_GraphNode* m_pBody;
	};

	const U32 MaxFluidEvents = 512;

	class C_FluidEventBuffer
	{
		friend class C_Scene;
		C_FluidEvent* m_pFluidEvents;
		U32 m_NumFluidEvents;
		U32 m_MaxFluidEvents;
		C_FluidEventBuffer* m_pPpuPtr;

	public:
		C_FluidEventBuffer(U32 maxFluidEvents = MaxFluidEvents) : m_NumFluidEvents(0), m_MaxFluidEvents(maxFluidEvents)
		{
			m_pFluidEvents = (C_FluidEvent*)BizAlignedMalloc(sizeof(C_FluidEvent) * maxFluidEvents, 16);
			m_pPpuPtr = this;
		}

		~C_FluidEventBuffer()
		{
			if(m_pFluidEvents)
			{
				BizAlignedFree(m_pFluidEvents);
			}
			m_pFluidEvents = NULL;
		}

		void Reset()
		{
			m_NumFluidEvents = 0;
		}

		void AddFluidEvent(const C_FluidEvent& fluidEvent);
//		{
//			//Lock the memory address...
//			U32 index = MT::AtomicAdd((U32*)(&m_NumFluidEvents), 1);
//			if(index < Spu::Get(&m_MaxFluidEvents))
//			{
//				DebugBreak();
//#ifdef PLATFORM_SPU
//				Spu::Store(&m_pFluidEvents[index], &fluidEvent, sizeof(C_FluidEvent));
//#else
//				m_pFluidEvents[index] = fluidEvent;
//#endif
//			}
//		}
	};

}

#endif