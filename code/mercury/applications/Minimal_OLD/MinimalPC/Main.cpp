#include "Stdafx.h"
#include "Main.h"
#include "Samples/Section/SectionEmpty.h"
#include "Samples/Section/SectionSkyDome.h"

#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
#include "system\memory\AllocatorFunctions.h"

void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }

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
	AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	AddSection(new C_SectionLauncher<C_SectionEmpty>("Empty"));

	//m_pSection->EnterSection();
	
	return true;
}

C_Sample* g_pSample = NULL;

}; // namespace Samples

using namespace Samples;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_pSample->MsgProc(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_pSample = new C_Sample();

	g_pSample->m_DeviceParams.m_pName		= "MinimalPC";
	g_pSample->m_DeviceParams.m_hInstance	= hInstance;
	g_pSample->m_DeviceParams.m_CmdShow		= nCmdShow;

	if(g_pSample->Create())
	{
		g_pSample->Run();
	}
	else
	{
		LOGERROR(Samples::g_Log, "Failed to create the sample\n");

		g_pSample->Destroy();
	}

	delete g_pSample;

	return 0;
}
