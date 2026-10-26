#include "StdAfx.h"
#include "Main.h"
#include "Samples/Section/SectionSkyDome.h"
#include "Samples/Section/SectionParticles.h"
#include "Samples/Section/SectionPostEffects.h"
#include "Samples/Section/SectionLightEffect.h"
#include "Samples/Section/SectionTracerFire.h"
#include "Samples/Section/SectionDynamicEffect.h"

#include "System/Serialise.h"
#include "Xt/XtGame.h"

XT_APPLICATION
	XT_DEPENDS_ON(RenderDynamicEffect)
	XT_DEPENDS_ON(RenderLightEffect)
	XT_DEPENDS_ON(RenderParticles)
	XT_DEPENDS_ON(RenderTracerEffect)
XT_END_APPLICATION

XtDb*				gXtDb;
XtPack*				gXtPack;
XtCallbackContext	gXtContext('P');

void OpenFlask()
{
	XtInitialise();

	Sys::I_File* f;

#if defined(PLATFORM_XENON)
	if (f = Sys::C_FileSystem::Instance().Open("game:\\DynamicEffect.bin"))
#elif defined(PLATFORM_PC)
	if (f = Sys::C_FileSystem::Instance().Open("DynamicEffect.bin"))
#elif defined(PLATFORM_PS3)
	if ((f = Sys::C_FileSystem::Instance().Open("DynamicEffect.bin")))
#endif
	{
		U32 length = (U32)f->Length();
		U8* data = new U8[length];

		if (data)
		{
			if (f->Read(data, length))
			{
				gXtDb	= XtDbCreate(XtGetApplicationSchema(), 2000, &gXtContext);
				gXtPack	= XtPackOpen(gXtDb, data, length);

				XtNetworkStart(gXtDb);
			}
		}

		delete data;
		delete f;
	}
}



namespace Samples
{

C_Sample::C_Sample()
{
	m_Name = "DynamicEffectTestApp";

	m_DeviceParams.m_RingBufferSecondarySize = 1024 * 1024 * 16;
	//m_DeviceParams.m_BackBufferColourFormat = Display::E_Format_A8R8G8B8;
	//m_DeviceParams.m_WindowBufferColourFormat = Display::E_Format_A8R8G8B8;
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	// open Xt
	OpenFlask();
	
	// add sections
	AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionParticles>("Particles"));
	AddSection(new C_SectionLauncher<C_SectionLightEffect>("LightEffect"));
	AddSection(new C_SectionLauncher<C_SectionTracerFire>("TracerFire"));
	AddSection(new C_SectionLauncher<C_SectionDynamicEffect>("DynamicEffect"));
		
	return true;
}

void C_Sample::MyDestroy()
{
	XtPackClose(gXtDb, gXtPack);
	XtDbDestroy(gXtDb);
}

void C_Sample::MyUpdate()
{
	XtNetworkUpdate();
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
