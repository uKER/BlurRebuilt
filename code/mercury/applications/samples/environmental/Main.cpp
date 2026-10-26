#include "StdAfx.h"
#include "Main.h"
#include "Samples/Section/SectionFFTOcean.h"
#include "Samples/Section/SectionFog.h"
#include "Samples/Section/SectionRain.h"
#include "Samples/Section/SectionSky.h"
#include "Samples/Section/SectionWaveEquation.h"
#include "System/Serialise.h"

#if defined(PLATFORM_PC)
//#include "system\memory\MemoryTrackerDefines.h"
//#include "system\memory\MemoryTracker.h"
//#include "system\memory\AllocatorFunctions.h"

//void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
//void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
//void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
//void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }
#endif

#if defined(PLATFORM_PS3)
void PreStaticInit();
#endif

namespace Samples
{

C_Sample::C_Sample()
{
	m_Name = "EnvironmentalTestApp";
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	// add sections
	AddSection(new C_SectionLauncher<C_SectionFFTOcean>("FFTOcean"));
	AddSection(new C_SectionLauncher<C_SectionFog>("Fog"));
	AddSection(new C_SectionLauncher<C_SectionRain>("Rain"));
	AddSection(new C_SectionLauncher<C_SectionSky>("Sky"));
	AddSection(new C_SectionLauncher<C_SectionWaveEquation>("WaveEquation"));
	
	return true;
}

void C_Sample::MyDestroy()
{
}

void C_Sample::MyUpdate()
{
}

}; // namespace Samples

#if defined(PLATFORM_XENON)
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
#endif

#if defined(PLATFORM_PS3)
int main(int argc, char *argb[])
{
	PreStaticInit();

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
#endif

#if defined(PLATFORM_PC)
Samples::C_Sample* g_pSample = NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_pSample->MsgProc(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_pSample = new Samples::C_Sample();

	g_pSample->m_DeviceParams.m_Width		= 800;
	g_pSample->m_DeviceParams.m_Height		= 600;
	g_pSample->m_DeviceParams.m_pName		= "environmental_pc";
	g_pSample->m_DeviceParams.m_hInstance	= hInstance;
	g_pSample->m_DeviceParams.m_CmdShow		= nCmdShow;
	g_pSample->m_DeviceParams.m_Multisample	= Display::E_Multisample_None;
	
	if (g_pSample->Create())
	{
		g_pSample->Run();
	}
	else
	{
		LOGFATAL(Display::g_Log, "Failed to create the sample\n");
	}

	delete g_pSample;

	return 0;
}
#endif

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
