#include "StdAfx.h"
#include "Main.h"
#include "Samples/Section/SectionSkyDome.h"
#include "Samples/Section/SectionParticles.h"
#include "Samples/Section/SectionRain.h"
#include "Samples/Section/SectionPostEffects.h"
#include "Samples/Section/SectionDynamicEffect.h"

namespace Samples
{

C_Sample::C_Sample()
{
	m_Name = "ParticlesTestApp";
	//m_DeviceParams.m_BackBufferColourFormat = Display::E_Format_A8R8G8B8;
	//m_DeviceParams.m_WindowBufferColourFormat = Display::E_Format_A8R8G8B8;

	m_DeviceParams.m_RingBufferSecondarySize = 1024*1024*12;
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionRain>("Rain"));
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionParticles>("Particles"));
	AddSection(new C_SectionLauncher<C_SectionDynamicEffect>("DynamicEffect2"));
	
	return true;
}

}; // namespace Samples

void __cdecl main()
{
	Samples::C_Sample sample;
	
	if (sample.Create())
	{
		sample.Run();
	}
	else
	{
		LOGFATAL(Samples::g_Log, "Failed to create the sample\n");
	}
}

/*
#include <system\memory.h>
#include <system\memory\memorytrackerdefines.h>
 
void * operator new (size_t size)
{
    size += sizeof(Memory::AllocationNode);
 
    void* p = malloc(size);
	S32 iOrg = (S32)size;
    S32 iActual = (S32)_msize(p);
 
	Memory::Tracker::Instance().TrackAllocation(p,           // allocation address
                                     iOrg,        // original size
                                     iActual,     // actual size (the difference is used to calculate memory wastage)
									 Memory::kMemType_System,
                                     (Memory::AllocationNode*)(((U8*)p) + iActual - sizeof(Memory::AllocationNode)), // address of tracking struct
                                     MEMORY_TRACKER_FLAGS);

	return p;
}
 
void operator delete(void *mem)
{
	if(mem)
	{
	    S32 iActual = (int)_msize(mem);
		Memory::Tracker::Instance().TrackDeallocation(	iActual, // actual allocation size (used to track memory wastage)
														Memory::kMemType_System,
														(Memory::AllocationNode*)(((U8*)mem) + iActual - sizeof(Memory::AllocationNode)) // address of tracking struct
                                       );
		free(mem);
	}
}*/
