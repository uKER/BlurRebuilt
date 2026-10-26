#include "Stdafx.h"
#include "Main.h"

#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
#include "system\memory\AllocatorFunctions.h"

void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }


#include "Samples/Section/SectionSkyDome.h"
#include "Samples/Section/SectionParticles.h"
#include "Samples/Section/SectionRain.h"
#include "Samples/Section/SectionPostEffects.h"

using namespace Samples;

namespace Samples
{

C_Sample::C_Sample()
{
}

C_Sample::~C_Sample()
{
}

bool C_Sample::MyCreate()
{
	AddSection(new C_SectionLauncher<C_SectionRain>("Rain"));
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionParticles>("Particles"));
	
	return true;
}

C_Sample* g_pSample = NULL;

}; // namespace Samples

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return Samples::g_pSample->MsgProc(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_pSample = new Samples::C_Sample();

	g_pSample->m_DeviceParams.m_Width		= 800;
	g_pSample->m_DeviceParams.m_Height		= 600;
	g_pSample->m_DeviceParams.m_pName		= "test_display_pc";
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
