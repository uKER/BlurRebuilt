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
#include "Samples/Section/SectionPostEffects.h"
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
	// open Xt
	OpenFlask();

	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionPostEffects>("PostEffects"));
	AddSection(new C_SectionLauncher<C_SectionParticles>("Particles"));
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
